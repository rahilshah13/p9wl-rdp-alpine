### Build: make
### Clean: make clean

CC = gcc
CFLAGS = -O3 -Wall -Wextra -DWLR_USE_UNSTABLE -I. -Isrc -include sys/types.h -include sys/select.h -include unistd.h
CFLAGS += $(shell pkg-config --cflags wlroots-0.19 wayland-server xkbcommon pixman-1 freerdp3 winpr3)
CFLAGS += -g -O0

LDFLAGS = $(shell pkg-config --libs wlroots-0.19 wayland-server xkbcommon pixman-1 freerdp3 winpr3)
LDFLAGS += -lpthread -lm -lssl -lcrypto -lfftw3f

### Source files (Plan 9 / 9P2000 dependencies removed)
SRCS = main.c \
       input/input.c \
       input/keymap.c \
       draw/draw.c \
       draw/draw_cmd.c \
       draw/compress.c \
       draw/scroll.c \
       draw/send.c \
       input/clipboard.c \
       wayland/focus_manager.c \
       wayland/popup.c \
       wayland/toplevel.c \
       wayland/wl_input.c \
       wayland/output.c \
       wayland/client.c \
       draw/phase_correlate.c \
       draw/parallel.c

OBJS = $(SRCS:.c=.o)

### Headers (Plan 9 / 9P2000 dependencies removed)
HDRS = types.h \
       input/input.h \
       input/keymap.h \
       draw/draw.h \
       draw/draw_cmd.h \
       draw/compress.h \
       draw/scroll.h \
       draw/send.h \
       input/clipboard.h \
       wayland/focus_manager.h \
       wayland/popup.h \
       wayland/toplevel.h \
       wayland/wl_input.h \
       wayland/output.h \
       wayland/client.h \
       wayland/wayland.h \
       draw/phase_correlate.h \
       draw/parallel.h

TARGET = p9wl-rdp-alpine

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
