---
title: sni-svc
section: 8
date: July 2025
---

# NAME

**sni-svc** – inspect or control services supervised by `sni-supervise`

# SYNOPSIS

**sni-svc** [**-d** *svdir*] [**-w**] [**-r**] [*service* [*command*]]

# DESCRIPTION

**sni-svc** prints the status of services or sends control commands to a single supervised service.

If no *service* is given, it prints the status of all valid service directories under *svdir* (default is `.`).
If a *command* is provided, it sends the corresponding one-byte command to the service’s `supervise/control` FIFO.

Status is read from `supervise/status` and printed in human-readable form.

# OPTIONS

**-d** *svdir*
 Use *svdir* as the base directory (default is `.`).

**-w**
 Wait for the status to change after sending a command (up to 5s timeout).

**-r**
 Before sending the command, stop the service and wait for it to go `down`.

**-h**
 Show usage.

# COMMANDS

Following commands are recognized:

| Char       | Action                           |
| ---------- | -------------------------------- |
| *u*p       | Start service and enable restart |
| *d*own     | Stop service and disable restart |
| *o*nce     | Start once, do not restart       |
| *t*erm     | Send SIGTERM to service          |
| *p*ause    | Send SIGSTOP to service          |
| *c*ontinue | Send SIGCONT to service          |
| *a*larm    | Send SIGALRM to service          |
| *h*up      | Send SIGHUP to service           |
| *i*nt      | Send SIGINT to service           |
| *q*uit     | Send SIGQUIT to service          |
| *1*        | Send SIGUSR1 to service          |
| *2*        | Send SIGUSR2 to service          |
| *x*        | Terminate supervise instance     |

But actually only the first character of the argument is used.

# EXAMPLES

Print status of all services in current directory:

```
% sni-svc
```

Stop a service:

```
% sni-svc -w myservice down
```

Restart a service, wait for status change:

```
% sni-svc -r myservice up
```

Send SIGINT to service:

```
% sni-svc myservice int
```

# SEE ALSO

**sni-supervise(8)**, **sni-waitsignal(1)**

# LICENSE

Zlib License
