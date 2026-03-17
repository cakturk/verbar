PREFIX ?= /usr/local
PKG_CONFIG ?= pkg-config
WITH_X11 ?= 1
WITH_DROPBOX ?= 1
WITH_NET ?= 1
WITH_VOLUME ?= 1

BASE_CFLAGS = -std=gnu99 -pedantic -Wall -Werror -D_GNU_SOURCE -O2
BASE_LDFLAGS =

CFLAGS := $(BASE_CFLAGS) \
	-DWITH_X11=$(WITH_X11) \
	-DWITH_DROPBOX=$(WITH_DROPBOX) \
	-DWITH_NET=$(WITH_NET) \
	-DWITH_VOLUME=$(WITH_VOLUME)
LDFLAGS := $(BASE_LDFLAGS)

ifeq ($(WITH_X11),1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags x11)
LDFLAGS += $(shell $(PKG_CONFIG) --libs x11)
endif

ifeq ($(WITH_NET),1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libmnl)
LDFLAGS += $(shell $(PKG_CONFIG) --libs libmnl)
endif

ifeq ($(WITH_VOLUME),1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags libpulse)
LDFLAGS += $(shell $(PKG_CONFIG) --libs libpulse)
endif
