// Pseudo-terminal: spawn the user's shell with a controlling tty.
// POSIX only (forkpty); on Windows Spawn() returns false.
#pragma once
#include <cstddef>

struct Pty {
    int fd = -1;
    long pid = -1;

    // Fork the user's $SHELL (fallback /bin/zsh, /bin/sh) on a cols x rows pty.
    bool Spawn(int cols, int rows);
    // >0 bytes read, -1 = no data right now, 0 = EOF (shell exited).
    int Read(char* buf, size_t n);
    void Write(const char* s, size_t n);
    // SIGHUP (then SIGKILL) the child and reap it.
    void Kill();

    Pty() = default;
    Pty(const Pty&) = delete;
    Pty& operator=(const Pty&) = delete;
    ~Pty() { Kill(); }
};
