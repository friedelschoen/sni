CC      = cc
CFLAGS  = -O2
LDFLAGS =
TARGET  = supervise
SRC     = supervise.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)
