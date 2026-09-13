/*
obs-plugin-countdown-adu - UI preview harness
Copyright (C) 2026

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

/*
 * Minimal stand-in for <util/config-file.h> for the standalone harness. Backed
 * by an in-memory map that persists to a small ini file on disk, so the
 * dialog's save/load can be exercised locally.
 */

#pragma once

typedef struct config_data config_t;

#ifdef __cplusplus
extern "C" {
#endif

const char *config_get_string(config_t *config, const char *section, const char *name);
void config_set_string(config_t *config, const char *section, const char *name, const char *value);
int config_save(config_t *config);

#ifdef __cplusplus
}
#endif
