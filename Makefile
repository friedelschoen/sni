CC      = cc
CFLAGS  = -std=gnu99 -Wall -Wextra -O2
LDFLAGS =
TARGETS = sni-supervise sni-svc sni-waitsignal
MAN1    = sni-waitsignal.1
MAN5    = sni.5
MAN8    = sni-supervise.8 sni-svc.8
MANUALS = $(MAN1) $(MAN5) $(MAN8)
PREFIX  = /usr/local/share

all: $(TARGETS) $(MANUALS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%: %.md
	lowdown -stman -o $@ $^

compile_flags.txt:
	echo $(CFLAGS) | tr ' ' '\n' > $@

clean:
	rm -f $(TARGETS) $(MANUALS) compile_flags.txt

install: $(TARGETS) $(MANUALS)
	install -d $(PREFIX)/bin \
		$(PREFIX)/share/man/man1 $(PREFIX)/share/man/man5 $(PREFIX)/share/man/man8
	install -m 0755 $(TARGETS) $(PREFIX)/bin
	install $(MAN1) $(PREFIX)/share/man/man1
	install $(MAN5) $(PREFIX)/share/man/man5
	install $(MAN8) $(PREFIX)/share/man/man8
