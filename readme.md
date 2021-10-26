after
=====

run a command *after* a process terminates.

I often remember too late to chain commands to long running jobs and this small
tool allows me to avoid to stop and re-run the job to chain something to it.

for example, I can get a pop-up when this slow download is finished:

    $ after $(pgrep -f youtube-dl) gxmessage "video download finished!"

or I can turn off my computer after the backup is done:

    $ after $(pgrep backup) systemctl poweroff

here `backup` could have been started by `cron` because `after` is not limited
to processes executed from the current shell/session/terminal/user but it can
"wait" (not in the UNIX sense, see below) for *any* process.

the general usage is:

    $ after -h
    after: usage: after pid [cmd] [cmdargs...]

`cmd` is not required, if missing, `after` will still block until the process
terminates and you can chain commands using shell syntax (this is also useful
for using shell completion feature while typing the command) e.g.:

    $ after $(pgrep sleep) && echo hello && printf "world\n"
    hello
    world
    $

but please remember that the exit status of `after` is **not** the exit status
of the "waited" process: `after` does not `wait()` for a process in the UNIX
sense, it merely gets notified when the process exits and then calls
`execvp(cmd)` (if `cmd` is present).

install
-------

clone the repo and build and install with your usual `make install`. you can
change `PREFIX` if you want, default is `/usr/local` which puts the binary in
`/usr/local/bin`.

BUGS (features?)
----------------

- **NOT PORTABLE!** `after` only runs on (recent) Linux (`>= 5.3`). it's
  implemented using the new [`pidfd_open()`][1] syscall/api which:

  > creates a file descriptor that refers to the process whose PID is specified
  > in pid.

  `pidfd`s are cool because they are stable, private handles to processes (i.e.
  no pid recycling problems) and are `poll`alble!

  > A PID file descriptor can be monitored using `poll(2)`, `select(2)`, and
  > `epoll(7)`.  When the process that it refers to terminates, these interfaces
  > indicate the file descriptor as readable. Note, however, that in the current
  > implementation, nothing can be read from the file descriptor (`read(2)` on
  > the file descriptor fails with the error `EINVAL`).

  `poll()` support was added in kernel `5.3`, which is good considering that
  current Debian stable is on `5.10`.

  to be honest, the core functionality of `after` was already there in the man
  page of `pidfd_open()` shown as example, I "stole" it from there eheheh :)

- `after` **can't** get the exit status of the terminated process and use it to
  *conditionally* chain other commands. this would be really nice to have, but
  at the moment it is not possible to `wait()` on non-child processes, not even
  with `pidfd`s. maybe it's possible to get the exit value by other means e.g.
  `ptrace` but I'd like to stick with `pidfd` api and avoid additional overhead.

- *but but all this can already be done using [insert-better-tool/shell-feature]!*
  yes yes, I know, this is yet another tool that does roughly the same thing
  using a new Linux api :)

[1]: https://man7.org/linux/man-pages/man2/pidfd_open.2.html
