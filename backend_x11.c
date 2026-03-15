/*
 * Copyright (C) 2015-2016 Omar Sandoval
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>

#include "backend.h"

struct x11_backend_data {
	Display *dpy;
	Window root;
};

static void *x11_backend_init(void)
{
	struct x11_backend_data *data;

	data = malloc(sizeof(*data));
	if (!data) {
		perror("malloc");
		return NULL;
	}

	data->dpy = XOpenDisplay(NULL);
	if (!data->dpy) {
		fprintf(stderr, "unable to open display '%s'\n",
			XDisplayName(NULL));
		free(data);
		return NULL;
	}

	data->root = DefaultRootWindow(data->dpy);
	return data;
}

static int x11_backend_write_status(void *backend_data, const char *status)
{
	struct x11_backend_data *data = backend_data;

	XStoreName(data->dpy, data->root, status);
	XFlush(data->dpy);
	return 0;
}

static void x11_backend_shutdown(void *backend_data)
{
	struct x11_backend_data *data = backend_data;

	if (!data)
		return;

	if (data->dpy) {
		XStoreName(data->dpy, data->root, "");
		XFlush(data->dpy);
		XCloseDisplay(data->dpy);
	}

	free(data);
}

const struct backend x11_backend = {
	.name = "x11",
	.init = x11_backend_init,
	.write_status = x11_backend_write_status,
	.shutdown = x11_backend_shutdown,
};
