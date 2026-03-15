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

#ifndef BACKEND_H
#define BACKEND_H

struct backend {
	const char *name;
	void *(*init)(void);
	int (*write_status)(void *data, const char *status);
	void (*shutdown)(void *data);
};

#if WITH_X11
extern const struct backend x11_backend;
#endif
extern const struct backend stdout_text_backend;

#endif /* BACKEND_H */
