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
#include "settings-dialog.hpp"

#include <obs-module.h>
#include <plugin-support.h>

#include <QDate>
#include <QFont>
#include <QFontDatabase>
#include <QFrame>
#include <QIcon>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSize>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>

namespace {

/* Shared dark-theme styling, roughly matching layout_main_window.svg. */
const char *kDockStyle = R"qss(
QWidget#countdownAduDockWidget {
	background-color: #1b1c20;
}
QLabel#titleLabel {
	color: #e1e4e8;
	font-size: 12px;
	font-weight: 700;
}
QLabel#columnLabel {
	color: #a0a6b2;
	font-size: 10px;
	font-weight: 600;
	margin-top: 6px;
}
QFrame#lcdPanel {
	background-color: #0d1117;
	border: 2px solid #252930;
	border-radius: 8px;
}
QLabel#displayLcd {
	background: transparent;
	border: none;
	color: #00e5ff;
}
QLabel#displayGhost {
	background: transparent;
	border: none;
	color: #082b36;
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
	font-size: 16px;
	font-weight: bold;
	color: #ffffff;
	/* Override the OBS theme's global QPushButton rules. The theme pins a fixed
	 * max-height, which - combined with shrinking the dock - clips the +/-
	 * glyph. Set an explicit min-height (so the glyph always fits) and release
	 * the max-height clamp. */
	padding: 2px;
	margin: 0px;
	min-height: 22px;
	max-height: 16777215px;
}
QPushButton#actionButton {
	font-size: 10px;
	padding: 0px 0px;
	margin: 0px;
}
QPushButton#actionButtonSettings {
	font-size: 22px;
	padding: 0px 0px;
	margin: 0px;
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
	digitalFontFamily = loadDigitalFont();
	setStyleSheet(kDockStyle);
	buildUi();
	applyStartupSchedule();
	updateDisplay();

	countdownTimer = new QTimer(this);
	countdownTimer->setInterval(1000);
	connect(countdownTimer, &QTimer::timeout, this, &CountdownDock::tickCountdown);
	startCountdown();
}

void CountdownDock::applyStartupSchedule()
{
	/*
	 * Default start time based on the weekday the dock is created (OBS launch):
	 *   - Tuesday / Thursday -> 20:00
	 *   - Saturday           -> 19:30
	 *   - Sunday             -> 18:30
	 *   - any other day      -> current time + 10 minutes
	 * Qt: dayOfWeek() is 1=Monday ... 7=Sunday.
	 */
	switch (QDate::currentDate().dayOfWeek()) {
	case Qt::Tuesday:
	case Qt::Thursday:
		hour = 20;
		minute = 0;
		break;
	case Qt::Saturday:
		hour = 19;
		minute = 30;
		break;
	case Qt::Sunday:
		hour = 18;
		minute = 30;
		break;
	default:
		setToNow(10);
		break;
	}
}

QString CountdownDock::loadDigitalFont()
{
	/* The font ships in the plugin's data dir (data/fonts). Resolve its path
	 * through OBS (or the harness stub) and register it with Qt. */
	QString family;

	const char *names[] = {"fonts/DSEG14Classic-Bold.ttf", "fonts/DSEG14Classic-Regular.ttf"};
	for (const char *name : names) {
		char *path = obs_module_file(name);
		if (!path) {
			obs_log(LOG_WARNING, "digital font: obs_module_file returned null for '%s'", name);
			continue;
		}

		obs_log(LOG_INFO, "digital font: resolved '%s' -> '%s'", name, path);

		int id = QFontDatabase::addApplicationFont(QString::fromUtf8(path));
		bfree(path);

		if (id < 0) {
			obs_log(LOG_WARNING, "digital font: addApplicationFont failed for '%s'", name);
			continue;
		}

		const QStringList families = QFontDatabase::applicationFontFamilies(id);
		obs_log(LOG_INFO, "digital font: registered id=%d families=[%s]", id,
			families.join(", ").toUtf8().constData());
		if (!families.isEmpty() && family.isEmpty()) {
			family = families.first();
		}
	}

	obs_log(LOG_INFO, "digital font: using family '%s'", family.toUtf8().constData());

	return family;
}

void CountdownDock::buildUi()
{
	auto *root = new QVBoxLayout(this);
	root->setContentsMargins(8, 6, 8, 6);
	root->setSpacing(6);

	/* Title */
	//auto *title = new QLabel(obs_module_text("CountdownAdu.Main.Title"), this);
	//title->setObjectName("titleLabel");
	//title->setAlignment(Qt::AlignCenter);
	//root->addWidget(title);

	/* Middle row: [Hora +/-] [display] [Min +/-] */
	auto *middle = new QHBoxLayout();
	middle->setSpacing(8);

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
	centerCol->setSpacing(6);

	auto *title = new QLabel(obs_module_text("CountdownAdu.Main.Title"), this);
	title->setObjectName("titleLabel");
	title->setAlignment(Qt::AlignCenter);

	/*
	 * LCD panel: a dark frame containing two stacked labels in the same grid
	 * cell - a dim "ghost" showing all segments lit, and the active digits on
	 * top with a cyan glow. Both use the embedded 14-segment font when found.
	 */
	auto *lcdPanel = new QFrame(this);
	lcdPanel->setObjectName("lcdPanel");
	lcdPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	auto *lcdGrid = new QGridLayout(lcdPanel);
	lcdGrid->setContentsMargins(lcdMargin, lcdMargin, lcdMargin, lcdMargin);

	const QString digitFamily = digitalFontFamily.isEmpty() ? "Courier New" : digitalFontFamily;

	QFont digitFont(digitFamily);
	digitFont.setPixelSize(lcdFontSize);

	/*
	 * Also pin the font via a per-widget stylesheet. The OBS theme applies a
	 * global QLabel stylesheet, and Qt stylesheets take precedence over
	 * setFont(); without this, the digits fall back to the theme font inside
	 * OBS (while still looking correct in the standalone harness).
	 */
	const QString digitCss = QString("font-family: \"%1\"; font-size: %2px;").arg(digitFamily).arg(lcdFontSize);

	/* Ghost layer: all segments lit ('~' = all-on in DSEG14), dimmed. */
	auto *ghost = new QLabel(digitalFontFamily.isEmpty() ? "88:88" : "~~:~~", lcdPanel);
	ghost->setObjectName("displayGhost");
	ghost->setAlignment(Qt::AlignCenter);
	ghost->setFont(digitFont);
	ghost->setStyleSheet(QString("QLabel#displayGhost { background: transparent; border: none; "
				     "color: #082b36; %1 }")
				     .arg(digitCss));

	display = new QLabel("20:00", lcdPanel);
	display->setObjectName("displayLcd");
	display->setAlignment(Qt::AlignCenter);
	display->setFont(digitFont);
	display->setStyleSheet(
		QString("QLabel#displayLcd { background: transparent; border: none; color: #00e5ff; %1 }")
			.arg(digitCss));

	auto *glow = new QGraphicsDropShadowEffect(display);
	glow->setBlurRadius(18);
	glow->setColor(QColor(0, 229, 255, 200));
	glow->setOffset(0, 0);
	display->setGraphicsEffect(glow);

	lcdGrid->addWidget(ghost, 0, 0);
	lcdGrid->addWidget(display, 0, 0);

	centerCol->addWidget(title);
	centerCol->addWidget(lcdPanel);
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
	bottom->setSpacing(8);

	auto *nowBtn = new QPushButton(obs_module_text("CountdownAdu.Main.Now"), this);
	nowBtn->setObjectName("actionButton");
	auto *plus5Btn = new QPushButton(obs_module_text("CountdownAdu.Main.Plus5"), this);
	plus5Btn->setObjectName("actionButton");
	auto *plus10Btn = new QPushButton(obs_module_text("CountdownAdu.Main.Plus10"), this);
	plus10Btn->setObjectName("actionButton");
	auto *settingsBtn = new QPushButton(this);
	settingsBtn->setObjectName("actionButtonSettings");
	/*
	 * Use a vector gear icon (perfectly centered and crisp) instead of the
	 * Unicode gear glyph, whose per-font metrics render it off-center.
	 */
	if (char *iconPath = obs_module_file("icons/gear.svg")) {
		settingsBtn->setIcon(QIcon(QString::fromUtf8(iconPath)));
		settingsBtn->setIconSize(QSize(16, 16));
		bfree(iconPath);
	} else {
		settingsBtn->setText(QString::fromUtf8("\u2699")); /* fallback glyph */
	}

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
	startCountdown();
}

void CountdownDock::addMinute(int delta)
{
	int total = hour * 60 + minute + delta;
	total = (total % (24 * 60) + (24 * 60)) % (24 * 60);
	hour = total / 60;
	minute = total % 60;
	updateDisplay();
	startCountdown();
}

void CountdownDock::setToNow(int plusMinutes)
{
	QTime now = QTime::currentTime();
	QTime target = now.addSecs(plusMinutes * 60);
	hour = target.hour();
	minute = target.minute();
	updateDisplay();
	startCountdown();
}

void CountdownDock::openSettings()
{
	CountdownSettingsDialog dialog(this);
	dialog.exec();
}

int CountdownDock::remainingSeconds() const
{
	/* Difference between the target time (HH:MM:00) and the real clock,
	 * within the same day. Negative once the target has passed. */
	QTime target(hour, minute, 0);
	QTime now = QTime::currentTime();
	return now.secsTo(target);
}

void CountdownDock::startCountdown()
{
	/* Guard: time may be set during construction before the timer exists
	 * (applyStartupSchedule). In that case the constructor starts it later. */
	if (!countdownTimer) {
		return;
	}

	/* (Re)evaluate after any time change. Tick immediately so the sources
	 * update without waiting a second, then keep ticking if still positive. */
	tickCountdown();
	if (remainingSeconds() > 0) {
		countdownTimer->start();
	}
}

void CountdownDock::tickCountdown()
{
	int remaining = remainingSeconds();
	if (remaining < 0) {
		remaining = 0;
	}

	/* Format as MM:SS where minutes are the TOTAL minutes (may exceed 59):
	 * e.g. 1h10m00s -> "70:00". */
	const int totalMinutes = remaining / 60;
	const int seconds = remaining % 60;
	const QString text = QString::asprintf("%02d:%02d", totalMinutes, seconds);

	CountdownSettingsDialog::updateConfiguredSources(text);

	if (remaining <= 0) {
		countdownTimer->stop(); /* reached 00:00 -> stop updating */
	}
}

void CountdownDock::updateDisplay()
{
	display->setText(QString::asprintf("%02d:%02d", hour, minute));
}
