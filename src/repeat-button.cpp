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

#include "repeat-button.hpp"

#include <QMouseEvent>
#include <QTimer>

RepeatButton::RepeatButton(const QString &text, QWidget *parent) : QPushButton(text, parent)
{
	timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &RepeatButton::onTimeout);
}

void RepeatButton::mousePressEvent(QMouseEvent *event)
{
	QPushButton::mousePressEvent(event);

	if (event->button() != Qt::LeftButton) {
		return;
	}

	/* First, immediate action on press. */
	emit triggered();

	/* Then start the accelerating auto-repeat. */
	currentInterval = kInitialInterval;
	timer->start(currentInterval);
}

void RepeatButton::mouseReleaseEvent(QMouseEvent *event)
{
	stopRepeat();
	QPushButton::mouseReleaseEvent(event);
}

void RepeatButton::onTimeout()
{
	/* Only keep repeating while the button is actually held down. */
	if (!isDown()) {
		stopRepeat();
		return;
	}

	emit triggered();

	/* Accelerate: shorten the interval down to the minimum. */
	if (currentInterval > kMinInterval) {
		currentInterval = qMax(kMinInterval, currentInterval - kAcceleration);
		timer->start(currentInterval);
	}
}

void RepeatButton::stopRepeat()
{
	if (timer->isActive()) {
		timer->stop();
	}
	currentInterval = kInitialInterval;
}
