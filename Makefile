CC      = cc
CFLAGS  = -std=gnu99 -Wall -Wextra -O2
LDFLAGS =
TARGETS = supervise svc waitsignal
PREFIX  = /usr/local/share

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

compile_flags.txt:
	echo $(CFLAGS) | tr ' ' '\n' > $@

clean:
	rm -f $(TARGETS)

install: $(TARGETS)
	install -d $(PREFIX)/bin
	install -m 0755 $(TARGETS) $(PREFIX)/bin
