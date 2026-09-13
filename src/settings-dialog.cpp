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

#include "settings-dialog.hpp"

#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/config-file.h>

#include <cstring>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace {

constexpr const char *kConfigSection = "CountdownAdu";

/* Per-row config keys: scene name and source name for rows 1 and 2. */
const char *sceneKey(int row)
{
	return row == 0 ? "Font1Scene" : "Font2Scene";
}
const char *sourceKey(int row)
{
	return row == 0 ? "Font1Source" : "Font2Source";
}

bool isTextSourceId(const char *id)
{
	if (!id) {
		return false;
	}
	return strcmp(id, "text_ft2_source") == 0 || strcmp(id, "text_ft2_source_v2") == 0 ||
	       strcmp(id, "text_gdiplus") == 0 || strcmp(id, "text_gdiplus_v2") == 0 ||
	       strcmp(id, "text_gdiplus_v3") == 0;
}

/* obs_scene_enum_items callback: collects text-source names into a QStringList. */
bool collectTextSources(obs_scene_t *, obs_sceneitem_t *item, void *param)
{
	auto *names = static_cast<QStringList *>(param);
	obs_source_t *source = obs_sceneitem_get_source(item);
	if (source && isTextSourceId(obs_source_get_unversioned_id(source))) {
		names->append(QString::fromUtf8(obs_source_get_name(source)));
	}
	return true; /* continue enumeration */
}

} // namespace

CountdownSettingsDialog::CountdownSettingsDialog(QWidget *parent) : QDialog(parent)
{
	setWindowTitle(obs_module_text("CountdownAdu.Settings.Title"));
	setModal(true);
	buildUi();
	loadSettings();
}

void CountdownSettingsDialog::buildUi()
{
	auto *root = new QVBoxLayout(this);

	auto *grid = new QGridLayout();
	grid->setHorizontalSpacing(8);
	grid->setVerticalSpacing(8);

	/* Header row */
	grid->addWidget(new QLabel(obs_module_text("CountdownAdu.Settings.Scene"), this), 0, 1);
	grid->addWidget(new QLabel(obs_module_text("CountdownAdu.Settings.Source"), this), 0, 2);

	const char *rowLabels[2] = {"CountdownAdu.Settings.Font1", "CountdownAdu.Settings.Font2"};

	for (int i = 0; i < 2; i++) {
		auto *label = new QLabel(obs_module_text(rowLabels[i]), this);

		rows[i].scene = new QComboBox(this);
		rows[i].source = new QComboBox(this);
		rows[i].scene->setMinimumWidth(160);
		rows[i].source->setMinimumWidth(160);

		populateScenes(rows[i].scene);
		/* First entry blank -> no source until a scene is chosen. */
		rows[i].source->addItem("");

		grid->addWidget(label, i + 1, 0);
		grid->addWidget(rows[i].scene, i + 1, 1);
		grid->addWidget(rows[i].source, i + 1, 2);

		/* When the scene changes, repopulate this row's source combo. */
		connect(rows[i].scene, &QComboBox::currentTextChanged, this,
			[this, i](const QString &sceneName) { populateTextSources(rows[i].source, sceneName); });
	}

	root->addLayout(grid);

	auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
		saveSettings();
		accept();
	});
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	root->addWidget(buttons);
}

void CountdownSettingsDialog::populateScenes(QComboBox *sceneCombo)
{
	sceneCombo->clear();
	sceneCombo->addItem(""); /* blank = none selected */

	struct obs_frontend_source_list scenes = {};
	obs_frontend_get_scenes(&scenes);
	for (size_t i = 0; i < scenes.sources.num; i++) {
		obs_source_t *scene = scenes.sources.array[i];
		sceneCombo->addItem(QString::fromUtf8(obs_source_get_name(scene)));
	}
	obs_frontend_source_list_free(&scenes);
}

void CountdownSettingsDialog::populateTextSources(QComboBox *sourceCombo, const QString &sceneName)
{
	sourceCombo->clear();
	sourceCombo->addItem(""); /* blank = none selected */

	if (sceneName.isEmpty()) {
		return;
	}

	obs_source_t *sceneSource = obs_get_source_by_name(sceneName.toUtf8().constData());
	if (!sceneSource) {
		return;
	}

	obs_scene_t *scene = obs_scene_from_source(sceneSource);
	if (scene) {
		QStringList names;
		obs_scene_enum_items(scene, collectTextSources, &names);
		names.sort(Qt::CaseInsensitive);
		sourceCombo->addItems(names);
	}

	obs_source_release(sceneSource);
}

void CountdownSettingsDialog::loadSettings()
{
	config_t *cfg = obs_frontend_get_user_config();
	if (!cfg) {
		return;
	}

	for (int i = 0; i < 2; i++) {
		const char *sceneName = config_get_string(cfg, kConfigSection, sceneKey(i));
		const char *sourceName = config_get_string(cfg, kConfigSection, sourceKey(i));

		if (sceneName && *sceneName) {
			int idx = rows[i].scene->findText(QString::fromUtf8(sceneName));
			if (idx >= 0) {
				rows[i].scene->setCurrentIndex(idx); /* triggers source populate */
			}
		}
		if (sourceName && *sourceName) {
			int idx = rows[i].source->findText(QString::fromUtf8(sourceName));
			if (idx >= 0) {
				rows[i].source->setCurrentIndex(idx);
			}
		}
	}
}

void CountdownSettingsDialog::updateConfiguredSources(const QString &text)
{
	config_t *cfg = obs_frontend_get_user_config();
	if (!cfg) {
		return;
	}

	for (int i = 0; i < 2; i++) {
		const char *sourceName = config_get_string(cfg, kConfigSection, sourceKey(i));
		if (!sourceName || !*sourceName) {
			continue;
		}

		obs_source_t *source = obs_get_source_by_name(sourceName);
		if (!source) {
			continue;
		}

		/* Only touch actual text sources. */
		if (isTextSourceId(obs_source_get_unversioned_id(source))) {
			obs_data_t *settings = obs_data_create();
			obs_data_set_string(settings, "text", text.toUtf8().constData());
			obs_source_update(source, settings);
			obs_data_release(settings);
		}

		obs_source_release(source);
	}
}

void CountdownSettingsDialog::saveSettings()
{
	config_t *cfg = obs_frontend_get_user_config();
	if (!cfg) {
		return;
	}

	for (int i = 0; i < 2; i++) {
		config_set_string(cfg, kConfigSection, sceneKey(i), rows[i].scene->currentText().toUtf8().constData());
		config_set_string(cfg, kConfigSection, sourceKey(i),
				  rows[i].source->currentText().toUtf8().constData());
	}

	config_save(cfg);
}
