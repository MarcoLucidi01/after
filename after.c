/*
 * MIT License
 *
 * Copyright (c) 2021 Marco Lucidi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#define VERSION "0.2.0"

static void     die(const char *, ...);
static pid_t    parsepid(const char *);

static const char *progname = "";

int main(int argc, char **argv)
{
        progname = argv[0];

        if (argc < 2)
                die("missing pid");

        if (strcmp(argv[1], "-h") == 0)
                die("usage: %s pid [cmd] [cmdargs...]", progname);
        else if (strcmp(argv[1], "-v") == 0)
                die(VERSION);

        pid_t pid = parsepid(argv[1]);
        if (pid < 0)
                die("invalid pid %s", argv[1]);

        /* currently there is no glibc wrapper for this system call */
        int pidfd = syscall(SYS_pidfd_open, pid, 0);
        if (pidfd < 0)
                die("pidfd_open(%d): %s", pid, strerror(errno));

        struct pollfd pollfd = { .fd = pidfd, .events = POLLIN, .revents = 0 };
        int ready = poll(&pollfd, 1, -1);
        close(pidfd);
        if (ready < 0)
                die("poll({%d, POLLIN}): %s", pidfd, strerror(errno));
        if (ready != 1 || ! (pollfd.revents & POLLIN))
                die("poll({%d, POLLIN}): pidfd not ready", pidfd);

        if (argc > 2) {
                execvp(argv[2], argv + 2);
                die("execvp(%s): %s", argv[2], strerror(errno));
        }
}

static void die(const char *reason, ...)
{
        fprintf(stderr, "%s: ", progname);
        va_list ap;
        va_start(ap, reason);
        vfprintf(stderr, reason, ap);
        va_end(ap);
        fprintf(stderr, "\n");

        exit(EXIT_FAILURE);
}

static pid_t parsepid(const char *s)
{
        if (s[0] == '\0')
                return -1;

        char *end = NULL;
        long pid = strtol(s, &end, 10);
        if (pid < 0 || pid == LONG_MAX || s == end || end[0] != '\0' || pid != (pid_t)pid)
                return -1;

        return pid;
}
