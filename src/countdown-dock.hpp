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

#include <QString>
#include <QWidget>

class QLabel;
class QTimer;

class CountdownDock : public QWidget {
	Q_OBJECT

public:
	explicit CountdownDock(QWidget *parent = nullptr);

protected:
	/* Pause the countdown loop while the dock is hidden (unchecked in the
	 * Docks menu) and resume it when shown again. */
	void showEvent(QShowEvent *event) override;
	void hideEvent(QHideEvent *event) override;

private slots:
	void addHour(int delta);
	void addMinute(int delta);
	void setToNow(int plusMinutes = 0);
	void openSettings();

private:
	void buildUi();
	void updateDisplay();
	QString loadDigitalFont();

	/* Sets the initial time based on the current weekday (see .cpp). */
	void applyStartupSchedule();

	/* Countdown engine: writes the remaining MM:SS to the configured sources. */
	void startCountdown();
	void tickCountdown();
	int remainingSeconds() const;

	QLabel *display = nullptr;

	/* Ticks every second while the countdown is running. */
	QTimer *countdownTimer = nullptr;

	/* Family name of the embedded 7/14-segment font, empty if unavailable. */
	QString digitalFontFamily;

	/* Target start time, 24h. */
	int hour = 20;
	int minute = 0;

	int buttonHeight = 24;

	/* Font size (in pixels) of the LCD panel digits. */
	int lcdFontSize = 32;

	/* Inner padding (px) around the LCD digits. Smaller = shorter panel. */
	int lcdMargin = 12;
};
