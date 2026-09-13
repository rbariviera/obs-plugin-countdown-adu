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
 * Minimal stand-in for OBS's <obs-module.h>, used ONLY by the standalone
 * harness so the shared widget sources can compile without libobs. It declares
 * just enough of the OBS module API that CountdownDock references. The actual
 * obs_module_text() is implemented in obs-module-stub.cpp.
 */

#pragma once

/* Log levels mirror libobs/util/base.h so shared sources can use LOG_*. */
#ifndef LOG_ERROR
#define LOG_ERROR 100
#define LOG_WARNING 200
#define LOG_INFO 300
#define LOG_DEBUG 400
#endif

/* Opaque OBS types referenced by the shared sources. */
typedef struct obs_source obs_source_t;
typedef struct obs_scene obs_scene_t;
typedef struct obs_sceneitem obs_sceneitem_t;
typedef struct obs_data obs_data_t;

#ifdef __cplusplus
extern "C" {
#endif

const char *obs_module_text(const char *lookup_string);

/*
 * In real OBS, obs_module_file() is a macro returning a heap-allocated absolute
 * path (freed with bfree). Here we provide plain functions with the same
 * contract so the shared sources can locate bundled data files (e.g. fonts).
 */
char *obs_module_file(const char *file);
void bfree(void *ptr);

/* Source / scene API used by the settings dialog (stubbed with fake data). */
const char *obs_source_get_name(const obs_source_t *source);
const char *obs_source_get_unversioned_id(const obs_source_t *source);
obs_source_t *obs_get_source_by_name(const char *name);
void obs_source_release(obs_source_t *source);
obs_scene_t *obs_scene_from_source(const obs_source_t *source);
obs_source_t *obs_sceneitem_get_source(const obs_sceneitem_t *item);
void obs_scene_enum_items(obs_scene_t *scene, bool (*callback)(obs_scene_t *, obs_sceneitem_t *, void *),
			  void *param);

/* obs_data + source update, used to push the countdown text into a source. */
obs_data_t *obs_data_create(void);
void obs_data_set_string(obs_data_t *data, const char *name, const char *val);
void obs_data_release(obs_data_t *data);
void obs_source_update(obs_source_t *source, obs_data_t *settings);

#ifdef __cplusplus
}
#endif
