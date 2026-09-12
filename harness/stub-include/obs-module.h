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

#ifdef __cplusplus
extern "C" {
#endif

const char *obs_module_text(const char *lookup_string);

#ifdef __cplusplus
}
#endif
