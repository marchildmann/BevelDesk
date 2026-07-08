#include "platform/pty.h"

#ifndef _WIN32

#include <cerrno>
#include <cstdlib>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#if defined(__APPLE__)
  #include <util.h>
#else
  #include <pty.h>
#endif

bool Pty::Spawn(int cols, int rows) {
    struct winsize ws;
    std::memset(&ws, 0, sizeof(ws));
    ws.ws_col = (unsigned short)cols;
    ws.ws_row = (unsigned short)rows;

    int master = -1;
    pid_t child = forkpty(&master, nullptr, nullptr, &ws);
    if (child < 0) return false;
    if (child == 0) {
        // child: exec an interactive shell with a terminal type we can parse
        setenv("TERM", "xterm-16color", 1);
        const char* shell = getenv("SHELL");
        if (!shell || !*shell) shell = "/bin/zsh";
        execl(shell, shell, "-i", (char*)nullptr);
        execl("/bin/sh", "/bin/sh", "-i", (char*)nullptr);
        _exit(127);
    }
    fcntl(master, F_SETFL, fcntl(master, F_GETFL) | O_NONBLOCK);
    fd = master;
    pid = child;
    return true;
}

int Pty::Read(char* buf, size_t n) {
    if (fd < 0) return 0;
    ssize_t r = read(fd, buf, n);
    if (r > 0) return (int)r;
    if (r < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return -1;
    return 0; // EOF or EIO: shell exited
}

void Pty::Write(const char* s, size_t n) {
    if (fd < 0) return;
    size_t off = 0;
    while (off < n) {
        ssize_t w = write(fd, s + off, n - off);
        if (w > 0) { off += (size_t)w; continue; }
        if (w < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) break; // drop rest
        break;
    }
}

void Pty::Kill() {
    if (fd >= 0) { close(fd); fd = -1; }
    if (pid > 0) {
        kill((pid_t)pid, SIGHUP);
        int st = 0;
        if (waitpid((pid_t)pid, &st, WNOHANG) == 0) {
            kill((pid_t)pid, SIGKILL);
            waitpid((pid_t)pid, &st, 0);
        }
        pid = -1;
    }
}

#else // _WIN32 — not wired up yet (ConPTY would go here)

bool Pty::Spawn(int, int) { return false; }
int Pty::Read(char*, size_t) { return 0; }
void Pty::Write(const char*, size_t) {}
void Pty::Kill() {}

#endif
