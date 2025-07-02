CC      = cc
CFLAGS  = -std=gnu99 -Wall -Wextra -O2
LDFLAGS =
TARGETS  = supervise waitsignal svc

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

compile_flags.txt:
	echo $(CFLAGS) | tr ' ' '\n' > $@

clean:
	rm -f $(TARGETS)
