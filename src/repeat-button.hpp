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

#include <QPushButton>

class QTimer;

/*
 * A push button that fires `triggered()` once when pressed and then keeps
 * firing while held down, gradually accelerating (shorter interval) the
 * longer it stays pressed. Releasing the button (or the mouse leaving it)
 * stops the repetition.
 */
class RepeatButton : public QPushButton {
	Q_OBJECT

public:
	explicit RepeatButton(const QString &text, QWidget *parent = nullptr);

signals:
	void triggered();

protected:
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
	void onTimeout();

private:
	void stopRepeat();

	QTimer *timer = nullptr;

	/* Repeat pacing (milliseconds). */
	static constexpr int kInitialInterval = 400;
	static constexpr int kMinInterval = 45;
	static constexpr int kAcceleration = 35;

	int currentInterval = kInitialInterval;
};
