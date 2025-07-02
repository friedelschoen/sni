# `supervise`

**`supervise`** is a small, dependency-aware process supervisor inspired by `runit` and similar minimalist systems like `minit` and `daemontools`.
It monitors a `./run` script, tracks and supervises dependencies listed in a `depends` file, and communicates via FIFOs under the `supervise/` directory.

It is designed to be simple, hackable, and robust. It uses no external libraries and is ideal for embedded systems, static builds, or small custom init setups.

## Features

* Automatically restarts `./run` if it exits
* Starts all services listed in the `depends` file
* Restarts dependencies if they are missing or stop
* Responds to runtime control messages via `supervise/control`
* Updates binary `supervise/status`, human-readable `supervise/stat`, and `supervise/pid`
* Exits gracefully, stopping `./run` and its dependencies in order

## Making a Service

A service is a directory. This directory **must contain a `./run` executable**. This script is executed directly inside the service-directory and should take **no arguments**. The environment is preserved.

```bash
#!/bin/sh

# Example run script
exec my-daemon --no-background
```

### Guidelines:

* Use `exec` in `./run` so the PID is correct.
* The daemon **must run in foreground**. If it daemonizes itself (forks to background), `supervise` will treat `./run` as exited.
* To handle daemonizing daemons, use a helper like `waitsignal`, which waits until your daemon is ready and then blocks until killed.

```sh
#!/bin/sh

my-daemon &
waitsignal
my-daemonctl stop
```

> Note: `supervise` **does not support `./finish`** scripts. You must clean up or wait inside your `./run`.

## Dependency Management

If a service contains a `depends` file, `supervise` will:

* Start every service listed there (relative paths)
* Re-check dependencies every second (`SIGALRM`)
* Terminate orphaned dependencies if they’re no longer listed
* Prevents duplicate instances via lockfiles

Example `depends` file:

```
logging
db
# optional
cache
```

Dependencies are started using the same binary (`supervise`) recursively. You must ensure all services are laid out in directories with proper `./run` scripts.

## Controlling Services

Send commands by writing a single character to `supervise/control`:

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

Example:

```sh
echo d > supervise/control
```

## Monitoring State

`supervise` writes three files:

* `supervise/status`: binary, tai64 + pid + flags
* `supervise/stat`: `"run"` or `"down"`
* `supervise/pid`: pid of the `./run` process (if running)

You can watch these with inotify or read them periodically.


## Logging

If you want logging, pipe output from `./run` into a logger yourself (e.g., `multilog`, `svlogd`, or a simple custom logger).

Example:

```sh
#!/bin/sh
exec 2>&1
exec my-daemon | ./logger
```

## License

This project is licensed under the beautiful terms of the Zlib license.
