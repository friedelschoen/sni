---
title: sni
section: 5
date: 2025-07-02
---

# NAME

**sni** – supervise-not-init files

# SYNOPSIS

<service>/depends
<service>/run
<service>/supervise/
<service>/supervise/control
<service>/supervise/lock
<service>/supervise/ok
<service>/supervise/pid
<service>/supervise/stat
<service>/supervise/status

# DESCRIPTION

## run

This is a long-running script, which is expected to run alongside the daemon.

## depends

A newline-terminated list of dependencies of a service which it depends on. This service is located in the parent directory of the service. It only starts the supervisor and will start a dependency almost simultaneously. If the service depends on the actual state of the service (booted up or builded, etc.), consider a more complex technique in the scripts.

Leading and trailing whitespaces are stripped, also characters after a `#` is considered a comment and is stripped.

## supervise/

A directory created by sni-supervise(8). It is only created when non-existing, if sni-* is running on a read-only filesystem, you can symlink `supervise/` to a temporary or writable filesystem.

## supervise/lock

This empty file is created to forbid multiple supervisors on one service. This is done by lockf(3), which releases a lock whenever the supervisor exits.

## supervise/ok

This is a FIFO (or named pipe) which is opened read-only. This file is used by runit-sv to check liveness but unused by sni-svc(8).

## supervise/status

This file hold the current state of the service. This used a 20-byte long machine-readable format and is also used in runit and daemontools.

```
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|     STATUS CHANGE     |  NANOSEC  |    PID    |PS|WU|TR|SR|
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
```

- STATUS CHANGE: unix seconds + 4611686018427387914ULL (tai, lower endian)
- NANOSEC: unix nanoseconds (unused by sni)
- PID = current pid (big endian)
- PS = is paused (int boolean)
- WU = wants up ('u' if want up, 'd' if want down)
- SR = state runit (0 is down, 1 is running, 2 if finishing in runit and daemontools)
- TR = was terminated (int boolean)

## supervise/stat

This file describes human-readable the current state, which is either 'run' or 'down'.

## supervise/pid

This file describes human-readable the current pid or 0 when no process is running.

## supervise/control

This FIFO (or named pipe) accepts one-byte commands to control the service-state. Following commands are understood:

| Command | Description                           |
| ------- | ------------------------------------- |
| `u`     | start service and do restart          |
| `d`     | terminate service and stop restarting |
| `o`     | start service and stop restarting     |
| `t`     | send SIGTERM to service               |
| `p`     | send SIGSTOP to service               |
| `c`     | send SIGCONT to service               |
| `a`     | send SIGALRM to service               |
| `h`     | send SIGHUP to service                |
| `i`     | send SIGINT to service                |
| `q`     | send SIGQUIT to service               |
| `1`     | send SIGUSR1 to service               |
| `2`     | send SIGUSR2 to service               |
| `x`     | quit supervise instance               |

Invalid characters are ignored.

# SEE ALSO

**svc(8)**, **waitsignal(1)**

# AUTHOR

Friedel Schön <derfriedmundschoen@gmail.com>

# LICENSE

Zlib License
