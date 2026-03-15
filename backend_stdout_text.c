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

#include "backend.h"

static void *stdout_text_backend_init(void)
{
	return stdout;
}

static int stdout_text_backend_write_status(void *backend_data,
					    const char *status)
{
	FILE *file = backend_data;

	if (fputs(status, file) == EOF)
		return -1;
	if (fputc('\n', file) == EOF)
		return -1;
	if (fflush(file) == EOF)
		return -1;

	return 0;
}

static void stdout_text_backend_shutdown(void *backend_data)
{
	FILE *file = backend_data;

	if (file)
		fflush(file);
}

const struct backend stdout_text_backend = {
	.name = "stdout-text",
	.render_icons = false,
	.init = stdout_text_backend_init,
	.write_status = stdout_text_backend_write_status,
	.shutdown = stdout_text_backend_shutdown,
};
