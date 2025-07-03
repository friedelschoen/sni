# `sni` - supervise-not-init

**`sni`** is a minimalist, dependency-aware process supervisor and a drop-in replacement for runit's `runsv` and `runsvdir`.

It is intentionally *not* an init system - it is designed to be embedded, portable, and easy to reason about.

## Goals

- Simplicity
- Portability
- Predictable behavior

## Use-cases

- Supervision trees in custom systems
- Supervision of user services

## Basic Usage

1. Create a service directory:
```
mkdir my-service
````

2. Add a `./run` script that launches your long-running process:
```sh
#!/bin/sh

exec my-daemon --no-background
````

3. (Optional) List dependent services in a `depends` file.

4. Start the supervisor:
```sh
sni-supervise my-service
```

5. Control the service using `sni-svc`:
```sh
sni-svc my-service up  # start
sni-svc my-service down  # stop
```

## See Also

See the manual pages (`man -k sni`) for full technical details, supported commands and status file format.

## License

Licensed under the beautiful terms of Zlib License.