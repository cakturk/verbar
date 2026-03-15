PREFIX ?= /usr/local
PKG_CONFIG ?= pkg-config
WITH_X11 ?= 0

BASE_CFLAGS = -std=gnu99 -pedantic -Wall -Werror -D_GNU_SOURCE -O2
BASE_LDFLAGS = -lmnl -lpulse

CFLAGS = $(BASE_CFLAGS) -DWITH_X11=$(WITH_X11)
LDFLAGS = $(BASE_LDFLAGS)

ifeq ($(WITH_X11),1)
CFLAGS += $(shell $(PKG_CONFIG) --cflags x11)
LDFLAGS += $(shell $(PKG_CONFIG) --libs x11)
endif
