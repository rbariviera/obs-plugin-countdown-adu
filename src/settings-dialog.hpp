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

#pragma once

#include <QDialog>

class QComboBox;

/*
 * Settings window for the countdown dock.
 *
 * Two rows ("Fonte 1" / "Fonte 2"), each with a scene combo and a text-source
 * combo. Picking a scene populates its text sources (text_ft2_source /
 * text_gdiplus). The first entry of every combo is blank (no selection).
 * Selections are persisted in the OBS user (global) config and restored on
 * restart.
 */
class CountdownSettingsDialog : public QDialog {
	Q_OBJECT

public:
	explicit CountdownSettingsDialog(QWidget *parent = nullptr);

	/*
	 * Writes `text` to the text sources currently configured for "Fonte 1"
	 * and "Fonte 2" (as saved in the OBS user config). Missing/blank/not-found
	 * selections are skipped. Safe to call frequently (e.g. once per second).
	 */
	static void updateConfiguredSources(const QString &text);

private:
	struct FontRow {
		QComboBox *scene = nullptr;
		QComboBox *source = nullptr;
	};

	void buildUi();
	void populateScenes(QComboBox *sceneCombo);
	void populateTextSources(QComboBox *sourceCombo, const QString &sceneName);
	void loadSettings();
	void saveSettings();

	FontRow rows[2];
};
