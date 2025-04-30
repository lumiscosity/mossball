/*
 * This file is part of Mossball.
 *
 * Mossball is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mossball is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mossball. If not, see <http://www.gnu.org/licenses/>.
 */

#include "pickerwidget.h"
#include "./ui_pickerwidget.h"

#include <lcf/dbstring.h>
#include <lcf/ldb/reader.h>
#include <QDirIterator>
#include <QTreeWidgetItem>
#include <utility>
#include <qmessagebox.h>

#include "../chgen/chgen.h"

PickerWidget::PickerWidget(QWidget *parent) : QDialog(parent), ui(new Ui::PickerWidget) {
    ui->setupUi(this);
    // checkbox + name, diff type, id
    ui->treeWidget->hideColumn(1);
    ui->treeWidget->hideColumn(2);
}

PickerWidget::~PickerWidget() {
    delete ui;
}

void PickerWidget::sendWarning(const QString &message) {
    QMessageBox::warning(this, "Warning", message);
}

void PickerWidget::addModelItem(const QString &folder, QString name, data::Status status, int id) {
    // the model consists of three columns:
    // - a checkbox next to the name (of either a folder or a file),
    // - a diff type (files only),
    // - and whether it is file-based or id-based (maps are an outlier and are handled separately).
    // an empty diff type signifies a folder.
    for (auto &i: ui->treeWidget->findItems(folder, Qt::MatchExactly)) {
        auto bogus = i->text(1);
        if (i->text(1) == "") {
            QTreeWidgetItem *model_name = new QTreeWidgetItem(i);
            // Add +/*/- sign to the text for accessibility
            model_name->setText(0, QString::fromStdString(data::status_string(status)) + " " + name);
            model_name->setText(1, QString::fromStdString(data::status_string(status)));
            model_name->setText(2, QString::number(id));
            model_name->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            model_name->setCheckState(0, Qt::CheckState::Checked);
            if (status == data::Status::ADDED) {
                model_name->setBackground(0, addition_brush);
            } else if (status == data::Status::REMOVED) {
                model_name->setBackground(0, removal_brush);
            } else {
                model_name->setBackground(0, modified_brush);
            }
            return;
        }
    }
    // no folder found, make a new one and try again
    QTreeWidgetItem *model_name = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr));
    model_name->setText(0, folder);
    model_name->setText(1, "");
    model_name->setText(2, "");
    model_name->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate | Qt::ItemIsEnabled);
    model_name->setCheckState(0, Qt::CheckState::Unchecked);
    ui->treeWidget->insertTopLevelItem(0, model_name);
    addModelItem(folder, std::move(name), status, id);
}

/**
 * @brief Checks for unchecked items in the picker widget and removes them
 * @note This implementation is messy, as it parses the content of the string in the picker widget. But this is the only way I found to do it
 */
void PickerWidget::removeUncheckedItems() {
    if (!changelog) {
        sendWarning("No changelog has been scanned");
        return;
    }

    std::vector<QString> ids_to_remove;

    // Iterate over all top-level items in the tree widget
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *top_level_item = ui->treeWidget->topLevelItem(i);
        // Iterate over all child items of the top-level item
        for (int j = 0; j < top_level_item->childCount(); ++j) {
            QTreeWidgetItem *child_item = top_level_item->child(j);
            if (child_item->checkState(0) == Qt::CheckState::Unchecked) {
                QString text = child_item->text(0);
                if (text.length() > 3 && text[2] == '[') {
                    // Extract the value between brackets
                    int end_bracket = text.indexOf(']', 3);
                    if (end_bracket != -1) {
                        QString id = text.mid(3, end_bracket - 3);
                        ids_to_remove.push_back(id);
                    }
                } else {
                    // Extract the value from the 3rd character onwards (because of the +/-/* indicator)
                    QString id = text.mid(2).trimmed();
                    ids_to_remove.push_back(id);
                }
            }
        }
    }

    // Remove items from changelog based on the collected IDs or filenames
    auto removeIfUnchecked = [&ids_to_remove](auto &container, auto idExtractor) {
        container.erase(
            std::remove_if(container.begin(), container.end(), [&ids_to_remove, idExtractor](const auto &item) {
                return std::find(ids_to_remove.begin(), ids_to_remove.end(), idExtractor(item)) != ids_to_remove.end();
            }),
            container.end()
        );
    };

    auto extractId = [](const auto &item) {
        return QString::fromStdString(data::id_string(item.data.ID));
    };

    auto extractFilename = [](const auto &item) {
        return QString::fromStdString(item.filename);
    };

    removeIfUnchecked(changelog->maps, extractId);
    removeIfUnchecked(changelog->common_events, extractId);
    removeIfUnchecked(changelog->tilesets, extractId);
    removeIfUnchecked(changelog->switches, extractId);
    removeIfUnchecked(changelog->variables, extractId);
    removeIfUnchecked(changelog->animations, extractId);
    removeIfUnchecked(changelog->battler_animations, extractId);
    removeIfUnchecked(changelog->troops, extractId);
    removeIfUnchecked(changelog->states, extractId);
    removeIfUnchecked(changelog->skills, extractId);
    removeIfUnchecked(changelog->items, extractId);
    removeIfUnchecked(changelog->enemies, extractId);
    removeIfUnchecked(changelog->elements, extractId);
    removeIfUnchecked(changelog->classes, extractId);
    removeIfUnchecked(changelog->actors, extractId);

    // For assets, we need to remove based on the filename instead of ID
    removeIfUnchecked(changelog->menu_themes, extractFilename);
    removeIfUnchecked(changelog->charsets, extractFilename);
    removeIfUnchecked(changelog->chipsets, extractFilename);
    removeIfUnchecked(changelog->musics, extractFilename);
    removeIfUnchecked(changelog->sounds, extractFilename);
    removeIfUnchecked(changelog->panoramas, extractFilename);
    removeIfUnchecked(changelog->pictures, extractFilename);
    removeIfUnchecked(changelog->animation_files, extractFilename);
}

void PickerWidget::gendiff() {
    changelog = chgen::ChangelogGenerator::scan();

    // Populate the model
    for (const auto &map: changelog->maps) {
        addModelItem(
            "Map", QString::fromStdString("[" + data::id_string(map.data.ID) + "] - " + lcf::ToString(map.data.name)),
            map.status);
    }

    for (const auto &menu_theme: changelog->menu_themes) {
        addModelItem("System", QString::fromStdString(menu_theme.filename),
                     menu_theme.status);
    }

    for (const auto &charset: changelog->charsets) {
        addModelItem("CharSet", QString::fromStdString(charset.filename),
                     charset.status);
    }

    for (const auto &chipset: changelog->chipsets) {
        addModelItem("ChipSet", QString::fromStdString(chipset.filename),
                     chipset.status);
    }

    for (const auto &music: changelog->musics) {
        addModelItem("Music", QString::fromStdString(music.filename),
                     music.status);
    }

    for (const auto &sound: changelog->sounds) {
        addModelItem("Sound", QString::fromStdString(sound.filename),
                     sound.status);
    }

    for (const auto &panorama: changelog->panoramas) {
        addModelItem("Panorama", QString::fromStdString(panorama.filename),
                     panorama.status);
    }

    for (const auto &picture: changelog->pictures) {
        addModelItem("Picture", QString::fromStdString(picture.filename),
                     picture.status);
    }

    for (const auto &animation_file: changelog->animation_files) {
        addModelItem("Battle", QString::fromStdString(animation_file.filename),
                     animation_file.status);
    }

    ui->treeWidget->sortItems(0, Qt::SortOrder::AscendingOrder);

    // Populate the model with database info
    for (const auto &ce: changelog->common_events) {
        addModelItem(
            "CE", QString::fromStdString("[" + data::id_string(ce.data.ID) + "] - " + lcf::ToString(ce.data.name)),
            ce.status);
    }

    for (const auto &tileset: changelog->tilesets) {
        addModelItem(
            "Tileset",
            QString::fromStdString("[" + data::id_string(tileset.data.ID) + "] - " + lcf::ToString(tileset.data.name)),
            tileset.status);
    }

    for (const auto &sw: changelog->switches) {
        addModelItem(
            "Switch", QString::fromStdString("[" + data::id_string(sw.data.ID) + "] - " + lcf::ToString(sw.data.name)),
            sw.status);
    }

    for (const auto &variable: changelog->variables) {
        addModelItem(
            "Variable", QString::fromStdString(
                "[" + data::id_string(variable.data.ID) + "] - " + lcf::ToString(variable.data.name)),
            variable.status);
    }

    for (const auto &animation: changelog->animations) {
        addModelItem("Animation",
                     QString::fromStdString(
                         "[" + data::id_string(animation.data.ID) + "] - " + lcf::ToString(animation.data.name)),
                     animation.status);
    }

    for (const auto &battler_animation: changelog->battler_animations) {
        addModelItem("Battler Animation",
                     QString::fromStdString(
                         "[" + data::id_string(battler_animation.data.ID) + "] - " +
                         lcf::ToString(battler_animation.data.name)),
                     battler_animation.status);
    }

    for (const auto &troop: changelog->troops) {
        addModelItem(
            "Troop",
            QString::fromStdString("[" + data::id_string(troop.data.ID) + "] - " + lcf::ToString(troop.data.name)),
            troop.status);
    }

    for (const auto &state: changelog->states) {
        addModelItem(
            "State",
            QString::fromStdString("[" + data::id_string(state.data.ID) + "] - " + lcf::ToString(state.data.name)),
            state.status);
    }

    for (const auto &skill: changelog->skills) {
        addModelItem(
            "Skill",
            QString::fromStdString("[" + data::id_string(skill.data.ID) + "] - " + lcf::ToString(skill.data.name)),
            skill.status);
    }

    for (const auto &item: changelog->items) {
        addModelItem(
            "Item",
            QString::fromStdString("[" + data::id_string(item.data.ID) + "] - " + lcf::ToString(item.data.name)),
            item.status);
    }

    for (const auto &enemy: changelog->enemies) {
        addModelItem(
            "Enemy",
            QString::fromStdString("[" + data::id_string(enemy.data.ID) + "] - " + lcf::ToString(enemy.data.name)),
            enemy.status);
    }

    for (const auto &element: changelog->elements) {
        addModelItem(
            "Element",
            QString::fromStdString("[" + data::id_string(element.data.ID) + "] - " + lcf::ToString(element.data.name)),
            element.status);
    }

    for (const auto &c: changelog->classes) {
        addModelItem(
            "Class", QString::fromStdString("[" + data::id_string(c.data.ID) + "] - " + lcf::ToString(c.data.name)),
            c.status);
    }

    for (const auto &actor: changelog->actors) {
        addModelItem(
            "Actor",
            QString::fromStdString("[" + data::id_string(actor.data.ID) + "] - " + lcf::ToString(actor.data.name)),
            actor.status);
    }

    // yoink the maps and put them up top
    for (const auto &i: ui->treeWidget->findItems("", Qt::MatchExactly, 1)) {
        if (i->text(0) == "Map") {
            ui->treeWidget->insertTopLevelItem(
                0, ui->treeWidget->takeTopLevelItem(ui->treeWidget->indexFromItem(i, 0).row()));
        }
    }
}


QString PickerWidget::genlog() {
    if (!changelog) {
        sendWarning("No changelog has been scanned");
        return "No changelog data";
    }
    return changelog->stringify();
}
