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
 * Standalone preview app for the CountdownDock widget.
 *
 * Lets you iterate on the dock's look and behaviour without launching the
 * whole OBS. Uses the exact same widget sources as the plugin; only the
 * obs_module_text() provider differs (see obs-module-stub.cpp).
 */

#include "countdown-dock.hpp"
#include "settings-dialog.hpp"

#include <QApplication>
#include <QTimer>

#include <cstdlib>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setApplicationName("Countdown Dock Preview");

	auto *dock = new CountdownDock();
	dock->setWindowTitle("Countdown Dock Preview");

	if (std::getenv("COUNTDOWN_HARNESS_MIN")) {
		/* Shrink to the minimum size, matching how small the dock can get
		 * inside OBS. Useful to preview the minimum footprint. */
		dock->resize(dock->minimumSizeHint());
	} else {
		/* Roughly the size of the reference layout (layout_main_window.svg). */
		dock->resize(540, 330);
	}
	dock->show();

	/*
	 * Optional: open the settings dialog directly for previewing/testing:
	 *   COUNTDOWN_HARNESS_SETTINGS=1 ./countdown-harness
	 */
	CountdownSettingsDialog *settings = nullptr;
	if (std::getenv("COUNTDOWN_HARNESS_SETTINGS")) {
		settings = new CountdownSettingsDialog(dock);
		settings->show();
	}

	/*
	 * Optional non-interactive screenshot mode for quick visual checks:
	 *   COUNTDOWN_HARNESS_SHOT=/tmp/preview.png ./countdown-harness
	 * Renders the widget (or the settings dialog, if open) to PNG and exits.
	 */
	if (const char *shot = std::getenv("COUNTDOWN_HARNESS_SHOT")) {
		const QString path = QString::fromUtf8(shot);
		QWidget *target = settings ? static_cast<QWidget *>(settings) : static_cast<QWidget *>(dock);
		QTimer::singleShot(300, target, [target, path]() {
			target->grab().save(path);
			QApplication::quit();
		});
	}

	return app.exec();
}
