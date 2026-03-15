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

#include "verbar_internal.h"

void set_icon_output(bool enabled);

int render_status(struct str *str, bool wordy, bool render_icons)
{
	set_icon_output(render_icons);
	str->len = 0;

	if (str_append(str, " "))
		return -1;

	if (append_sections(str, wordy))
		return -1;

	return str_null_terminate(str);
}
