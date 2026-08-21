### Build: make
### Clean: make clean

CC = gcc
CFLAGS = -O3 -Wall -Wextra -DWLR_USE_UNSTABLE -I. -Isrc -include sys/types.h -include sys/select.h -include unistd.h
CFLAGS += $(shell pkg-config --cflags wlroots-0.19 wayland-server xkbcommon pixman-1 freerdp3 winpr3 xcb xcb-renderutil)
CFLAGS += -g -O0

LDFLAGS = $(shell pkg-config --libs wlroots-0.19 wayland-server xkbcommon pixman-1 freerdp3 winpr3 xcb xcb-renderutil)
LDFLAGS += -lpthread -lm -lssl -lcrypto -lfftw3f

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
       wayland/xwayland.c \
       draw/phase_correlate.c \
       draw/parallel.c

OBJS = $(SRCS:.c=.o)
HDRS = types.h
TARGET = p9wl-rdp-alpine

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
