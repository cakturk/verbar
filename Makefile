include config.mk

OBJS := main.o \
	backend_stdout_text.o \
	plugins.o \
	render.o \
	util.o \
	clock.o \
	cpu.o \
	dropbox.o \
	mem.o \
	net.o \
	power.o \
	volume.o \
	pa_watcher.o

ifeq ($(WITH_X11),1)
OBJS += backend_x11.o
endif

verbar: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: install
install: verbar
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m755 verbar $(DESTDIR)$(PREFIX)/bin/

.PHONY: clean
clean:
	rm -f verbar $(OBJS)
