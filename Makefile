TARGET := fspupsmon

SRCDIR := src

CFLAGS += -O2 -Werror -Wall -Wextra -I$(SRCDIR) -D_GNU_SOURCE

LDFLAGS += -lrt

SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c, $(SRCDIR)/%.o, $(SRCS))

all: $(TARGET)

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS)

install:
	install -D --mode=0755 $(TARGET) $(DESTDIR)/usr/bin/$(TARGET)

clean:
	-rm $(OBJS) $(TARGET)
