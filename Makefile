CC      = cc
CFLAGS  = -std=gnu99 -Wall -Wextra -O2
LDFLAGS =
TARGET  = supervise
SRC     = supervise.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

compile_flags.txt:
	echo $(CFLAGS) | tr ' ' '\n' > $@

clean:
	rm -f $(TARGET)
