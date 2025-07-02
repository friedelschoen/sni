---
title: sni-supervise
section: 8
date: 2025-07-02
---

# NAME

**sni-supervise** – supervise-not-init - supervisor

# SYNOPSIS

**sni-supervise** [-h] [-C _dir_]

# DESCRIPTION

**sni-supervise** starts a small, robust process supervisor for the current directory or given service directory.

Each service is expected to contain:

- a `run` executable script (run in the foreground),
- an optional `depends` file listing other services,
- a `supervise/` directory with control/status FIFOs and state files.

# SUPERVISION

The supervisor:

- Parses `./depends` and starts each dependency recursively using the same binary.
- Starts the `./run` script in the service directory.
- Monitors for its exit and restarts it if needed.
- Watches `supervise/control` for commands.
- Writes its state to `supervise/status`, `supervise/stat` and `supervise/pid`.

# DEPENDENCIES

If a `depends` file exists, each line is interpreted as a sibling service directory.

When a dependency dies, `supervise` reloads `depends`, compares with the previous list, and:

- Starts new dependencies that are not already running.
- Terminates dependencies that are no longer listed.

Dependencies are supervised separately.

# COMMANDS

The control FIFO (`supervise/control`) accepts one-byte commands:

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

# EXIT STATUS

Returns 0 if started successfully and exits cleanly.
Returns 1 on error or setup failure.

# SEE ALSO

**sni-svc(8)**, **sni-waitsignal(1)**, **sni(5)**

# AUTHOR

Friedel Schön <derfriedmundschoen@gmail.com>

# LICENSE

Zlib License
