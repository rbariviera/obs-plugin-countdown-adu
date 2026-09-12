/*
obs-plugin-countdown-adu
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

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <plugin-support.h>

#include "countdown-dock.hpp"

#define COUNTDOWN_ADU_DOCK_ID "obs-plugin-countdown-adu-dock"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

MODULE_EXPORT const char *obs_module_description(void)
{
	return "ADU Countdown plugin for OBS Studio";
}

static void register_countdown_dock(void)
{
	/* Ownership of the widget is transferred to OBS, which wraps it in a
	 * dock and adds a show/hide toggle to the Docks menu. */
	CountdownDock *dock = new CountdownDock();

	if (!obs_frontend_add_dock_by_id(COUNTDOWN_ADU_DOCK_ID, obs_module_text("CountdownAdu.Dock.Title"), dock)) {
		obs_log(LOG_WARNING, "failed to register countdown dock");
		delete dock;
	}
}

bool obs_module_load(void)
{
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
	return true;
}

void obs_module_post_load(void)
{
	/* Register the dock here (not on FINISHED_LOADING) so that it already
	 * exists when OBS restores the saved dock layout during startup.
	 * Otherwise Qt has no state to restore for it and the dock always comes
	 * back hidden, losing its previous position/visibility. */
	register_countdown_dock();
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "plugin unloaded");
}
