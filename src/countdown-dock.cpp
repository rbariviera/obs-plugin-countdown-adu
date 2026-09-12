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

#include "countdown-dock.hpp"
#include "repeat-button.hpp"

#include <obs-module.h>

#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTime>
#include <QVBoxLayout>

namespace {

/* Shared dark-theme styling, roughly matching layout_main_window.svg. */
const char *kDockStyle = R"qss(
QWidget#countdownAduDockWidget {
	background-color: #1b1c20;
}
QLabel#titleLabel {
	color: #e1e4e8;
	font-size: 14px;
	font-weight: 700;
}
QLabel#columnLabel {
	color: #a0a6b2;
	font-size: 12px;
	font-weight: 600;
}
QLabel#displayLcd {
	background-color: #0d1117;
	border: 2px solid #252930;
	border-radius: 8px;
	color: #00e5ff;
	font-family: "Courier New", monospace;
	font-size: 48px;
	font-weight: 900;
	letter-spacing: 0px;
	padding: 6px 6px;
}
QPushButton {
	background-color: #343841;
	border: 1px solid #4d525c;
	border-radius: 6px;
	color: #f0f2f5;
	font-weight: 700;
}
QPushButton:hover {
	background-color: #3c4046;
}
QPushButton:pressed {
	background-color: #2c3036;
}
QPushButton#stepButton {
	font-size: 18px;
	font-weight: bold;
	color: #ffffff;
}
QPushButton#actionButton {
	font-size: 12px;
}
)qss";

QPushButton *makeStepButton(const QString &glyph, int minHeight, QWidget *parent)
{
	auto *btn = new RepeatButton(glyph, parent);
	btn->setObjectName("stepButton");
	btn->setMinimumSize(28, minHeight);
	btn->setFocusPolicy(Qt::NoFocus);
	return btn;
}

} // namespace

CountdownDock::CountdownDock(QWidget *parent) : QWidget(parent)
{
	setObjectName("countdownAduDockWidget");
	setStyleSheet(kDockStyle);
	buildUi();
	updateDisplay();
}

void CountdownDock::buildUi()
{
	auto *root = new QVBoxLayout(this);
	root->setContentsMargins(12, 12, 12, 12);
	root->setSpacing(8);

	/* Title */
	//auto *title = new QLabel(obs_module_text("CountdownAdu.Main.Title"), this);
	//title->setObjectName("titleLabel");
	//title->setAlignment(Qt::AlignCenter);
	//root->addWidget(title);

	/* Middle row: [Hora +/-] [display] [Min +/-] */
	auto *middle = new QHBoxLayout();
	middle->setSpacing(12);

	/* Left column: Hora */
	auto *hourCol = new QVBoxLayout();
	hourCol->setSpacing(6);
	auto *hourLabel = new QLabel(obs_module_text("CountdownAdu.Main.Hour"), this);
	hourLabel->setObjectName("columnLabel");
	hourLabel->setAlignment(Qt::AlignCenter);
	auto *hourPlus = makeStepButton("+", buttonHeight, this);
	auto *hourMinus = makeStepButton(QString::fromUtf8("\u2212"), buttonHeight, this); /* minus sign */
	hourCol->addWidget(hourLabel);
	hourCol->addWidget(hourPlus);
	hourCol->addWidget(hourMinus);
	hourCol->addStretch(1);
	middle->addLayout(hourCol);

	/* Center: LCD display */
	auto *centerCol = new QVBoxLayout();
	centerCol->setSpacing(8);

	auto *title = new QLabel(obs_module_text("CountdownAdu.Main.Title"), this);
	title->setObjectName("titleLabel");
	title->setAlignment(Qt::AlignCenter);

	display = new QLabel("20:00", this);
	display->setObjectName("displayLcd");
	display->setAlignment(Qt::AlignCenter);
	display->setMinimumHeight(60);
	display->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	auto *glow = new QGraphicsDropShadowEffect(display);
	glow->setBlurRadius(18);
	glow->setColor(QColor(0, 229, 255, 200));
	glow->setOffset(0, 0);
	display->setGraphicsEffect(glow);

	centerCol->addWidget(title);
	centerCol->addWidget(display);
	centerCol->addStretch(1);
	middle->addLayout(centerCol, 1);

	/* Right column: Min */
	auto *minCol = new QVBoxLayout();
	minCol->setSpacing(6);
	auto *minLabel = new QLabel(obs_module_text("CountdownAdu.Main.Minute"), this);
	minLabel->setObjectName("columnLabel");
	minLabel->setAlignment(Qt::AlignCenter);
	auto *minPlus = makeStepButton("+", buttonHeight, this);
	auto *minMinus = makeStepButton(QString::fromUtf8("\u2212"), buttonHeight, this);
	minCol->addWidget(minLabel);
	minCol->addWidget(minPlus);
	minCol->addWidget(minMinus);
	minCol->addStretch(1);
	middle->addLayout(minCol);

	root->addLayout(middle);

	/* Bottom row: Agora / 5 min / 10 min / settings */
	auto *bottom = new QHBoxLayout();
	bottom->setSpacing(12);

	auto *nowBtn = new QPushButton(obs_module_text("CountdownAdu.Main.Now"), this);
	nowBtn->setObjectName("actionButton");
	auto *plus5Btn = new QPushButton(obs_module_text("CountdownAdu.Main.Plus5"), this);
	plus5Btn->setObjectName("actionButton");
	auto *plus10Btn = new QPushButton(obs_module_text("CountdownAdu.Main.Plus10"), this);
	plus10Btn->setObjectName("actionButton");
	auto *settingsBtn = new QPushButton(this);
	settingsBtn->setObjectName("actionButton");
	settingsBtn->setText(QString::fromUtf8("\u2699")); /* gear glyph fallback */

	for (QPushButton *b : {nowBtn, plus5Btn, plus10Btn, settingsBtn}) {
		b->setMinimumHeight(buttonHeight);
		b->setFocusPolicy(Qt::NoFocus);
	}

	bottom->addWidget(nowBtn, 1);
	bottom->addWidget(plus5Btn, 1);
	bottom->addWidget(plus10Btn, 1);
	bottom->addWidget(settingsBtn, 1);

	root->addLayout(bottom);

	/* Connections */
	connect(static_cast<RepeatButton *>(hourPlus), &RepeatButton::triggered, this, [this]() { addHour(+1); });
	connect(static_cast<RepeatButton *>(hourMinus), &RepeatButton::triggered, this, [this]() { addHour(-1); });
	connect(static_cast<RepeatButton *>(minPlus), &RepeatButton::triggered, this, [this]() { addMinute(+1); });
	connect(static_cast<RepeatButton *>(minMinus), &RepeatButton::triggered, this, [this]() { addMinute(-1); });

	connect(nowBtn, &QPushButton::clicked, this, [this]() { setToNow(0); });
	connect(plus5Btn, &QPushButton::clicked, this, [this]() { setToNow(5); });
	connect(plus10Btn, &QPushButton::clicked, this, [this]() { setToNow(10); });
	connect(settingsBtn, &QPushButton::clicked, this, &CountdownDock::openSettings);
}

void CountdownDock::addHour(int delta)
{
	hour = (hour + delta + 24) % 24;
	updateDisplay();
}

void CountdownDock::addMinute(int delta)
{
	int total = hour * 60 + minute + delta;
	total = (total % (24 * 60) + (24 * 60)) % (24 * 60);
	hour = total / 60;
	minute = total % 60;
	updateDisplay();
}

void CountdownDock::setToNow(int plusMinutes)
{
	QTime now = QTime::currentTime();
	QTime target = now.addSecs(plusMinutes * 60);
	hour = target.hour();
	minute = target.minute();
	updateDisplay();
}

void CountdownDock::openSettings()
{
	/* Settings window will be implemented later. */
}

void CountdownDock::updateDisplay()
{
	display->setText(QString::asprintf("%02d:%02d", hour, minute));
}
