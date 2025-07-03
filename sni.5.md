---
title: sni
section: 5
date: July 2025
---

# NAME

**sni** – supervise-not-init files

# SYNOPSIS

```
<service>/run
<service>/finish
<service>/depends
<service>/supervise/
<service>/supervise/control
<service>/supervise/lock
<service>/supervise/ok
<service>/supervise/pid
<service>/supervise/stat
<service>/supervise/status
```

# DESCRIPTION

## run

A long-running script expected to start and monitor the daemon. It is executed directly and should not daemonize or fork. The script should `exec` the actual process.

## finish

A short script which is expected to clean-up the service or stop the background-daemon when `exec`'ing to `sni-waitpid`.

## depends

A newline-separated list of service dependencies. Each listed service is expected to reside in the parent directory of the current service.

Dependencies are started concurrently with the current service, but **no guarantee is made** about readiness. If the service depends on the state of another (e.g., fully started or ready), more advanced coordination must be implemented in the scripts themselves.

Leading/trailing whitespace is stripped, and anything following a `#` is considered a comment and ignored.

## supervise/

A directory created by `sni-supervise(8)`. If the filesystem is read-only, this directory can be symlinked to a writable location (e.g., `/tmp`).

## supervise/status

Contains the current service state in a fixed 20-byte binary format, compatible with `runit` and `daemontools`.

```

+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|     STATUS CHANGE     |  NANOSEC  |    PID    |PS|WU|TR|SR|
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

```

- **STATUS CHANGE**: TAI64 timestamp (Unix seconds + `4611686018427387914ULL`, little endian)
- **NANOSEC**: Nanoseconds field (currently unused by `sni`)
- **PID**: Current process ID (big endian)
- **PS**: Paused status (0 or 1)
- **WU**: Desired state ('u' for up, 'd' for down)
- **TR**: Termination status (1 if last exit was due to SIGTERM)
- **SR**: Run state (0 = down, 1 = running, 2 = finish)

## supervise/stat

Contains a human-readable state string: either `run`, `finish` or `down`.

## supervise/pid

Contains the process ID of the running service, or `0` if no process is active.

## supervise/control

A FIFO (named pipe) that accepts single-byte commands to control the service. Supported commands:

| Command | Description                           |
| ------- | ------------------------------------- |
| `u`     | Start service and enable restarts     |
| `d`     | Terminate service and disable restart |
| `o`     | Start service once (no restart)       |
| `t`     | Send `SIGTERM` to the service         |
| `p`     | Send `SIGSTOP` (pause)                |
| `c`     | Send `SIGCONT` (resume)               |
| `a`     | Send `SIGALRM`                        |
| `h`     | Send `SIGHUP`                         |
| `i`     | Send `SIGINT`                         |
| `q`     | Send `SIGQUIT`                        |
| `1`     | Send `SIGUSR1`                        |
| `2`     | Send `SIGUSR2`                        |
| `x`     | Stop and exit the supervisor process  |

Unrecognized commands are ignored.

## supervise/lock

An empty file used to prevent multiple supervisors from managing the same service. A file lock is acquired via `lockf(3)`, which is automatically released if the supervisor exits.

## supervise/ok

A FIFO (named pipe) opened read-only. Used by some tools (e.g., `runit-sv`) to detect liveness. Currently unused by `sni-svc(8)`.

# SEE ALSO

**sni-svc(8)**, **waitsignal(1)**

# AUTHOR

Friedel Schön <derfriedmundschoen@gmail.com>

# LICENSE

Zlib License
