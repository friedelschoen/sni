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

The supervisor:

- Parses `./depends` and starts each dependency recursively using the same binary.
- Starts the `./run` script in the service directory.
- Monitors for its exit and restarts it if needed.
- Watches `supervise/control` for commands.
- Writes its state to `supervise/status`, `supervise/stat` and `supervise/pid`.

For more information about expected files see sni(5).

Controlling is done with either runits sv(8) or sni-svc(8).

# EXIT STATUS

Returns 0 if started successfully and exits cleanly.
Returns 1 on error or setup failure.

# SEE ALSO

**sni-svc(8)**, **sni-waitsignal(1)**, **sni(5)**

# AUTHOR

Friedel Schön <derfriedmundschoen@gmail.com>

# LICENSE

Zlib License
