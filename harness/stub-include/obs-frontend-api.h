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
 * Minimal stand-in for <obs-frontend-api.h> for the standalone harness. Only
 * declares what the settings dialog uses. Backed by fake data (see
 * obs-module-stub.cpp).
 */

#pragma once

#include <obs-module.h>
#include <stddef.h>

typedef struct config_data config_t;

/* Simplified source list: exposes the same sources.num / sources.array shape
 * the real DARRAY-based struct provides, which is all the dialog reads. */
struct obs_frontend_source_list {
	struct {
		size_t num;
		obs_source_t **array;
	} sources;
};

#ifdef __cplusplus
extern "C" {
#endif

void obs_frontend_get_scenes(struct obs_frontend_source_list *sources);
void obs_frontend_source_list_free(struct obs_frontend_source_list *source_list);
config_t *obs_frontend_get_user_config(void);

#ifdef __cplusplus
}
#endif
