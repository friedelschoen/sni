---
title: sni-waitsignal
section: 1
date: July 2025
---

# NAME

**sni-waitsignal** – wait for a termination signal and print which one occurred

# SYNOPSIS

**sni-waitsignal** [**-n**]

# DESCRIPTION

**sni-waitsignal** waits for a signal and then prints which signal it received.
It is typically used in `run` scripts to wait for the supervising process to signal termination, such as when wrapping daemons that fork themselves.

After printing the signal, it exits with status code 0.

# OPTIONS

**-n**
 Print the *name* of the signal (e.g. `"Terminated"`) instead of its numeric value.

# EXAMPLES

./run:
```sh
#!/bin/sh

my-background-daemon
exec sni-waitsignal
````

./finish:
```sh
#!/bin/sh

my-daemonctl stop
````

Or for clean-up:

./run:
```sh
#!/bin/sh

echo hello > ~/myfile
````

./finish:
```sh
#!/bin/sh

rm -f ~/myfile
````

# EXIT STATUS

Always exits with status code 0.

# SEE ALSO

**sni-supervise(8)**, **sni(5)**, **signal(7)**

# LICENSE

Zlib License
