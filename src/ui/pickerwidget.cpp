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

#include "../pickerwidget.h"
#include "./ui_pickerwidget.h"

#include <lcf/dbstring.h>
#include <lcf/ldb/reader.h>
#include <lcf/lmt/reader.h>
#include <lcf/lmu/reader.h>
#include <QCryptographicHash>
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

bool PickerWidget::is_oneway(int from_id, int to_id, QString to_map, std::string encoding) {
    // map_outgoing is used as a cache
    if (map_outgoing.contains(to_id)) {
        return !map_outgoing[to_id].contains(from_id);
    } else {
        // find and cache the destination id of every transfer in the map
        std::unique_ptr<lcf::rpg::Map> map = lcf::LMU_Reader::Load(to_map.toUtf8().data(), encoding);
        if (not map) {
            return true;
        }
        map_outgoing[to_id] = QList<int>();
        for (const lcf::rpg::Event &i: map->events) {
            for (const lcf::rpg::EventPage &j: i.pages) {
                for (lcf::rpg::EventCommand k: j.event_commands) {
                    if (k.code == int(lcf::rpg::EventCommand::Code::Teleport)) {
                        map_outgoing[to_id].append(k.parameters[0]);
                    }
                }
            }
        }
        return is_oneway(from_id, to_id, to_map, encoding);
    }
}

void PickerWidget::addModelItem(const QString &folder, QString name, QString type, int id) {
    // the model consists of three columns:
    // - a checkbox next to the name (of either a folder or a file),
    // - a diff type (files only),
    // - and whether it is file-based or id-based (maps are an outlier and are handled separately).
    // an empty diff type signifies a folder.
    for (auto &i: ui->treeWidget->findItems(folder, Qt::MatchExactly)) {
        auto bogus = i->text(1);
        if (i->text(1) == "") {
            QTreeWidgetItem *model_name = new QTreeWidgetItem(i);
            model_name->setText(0, name);
            model_name->setText(1, type);
            model_name->setText(2, QString::number(id));
            model_name->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            model_name->setCheckState(0, Qt::CheckState::Checked);
            if (type == "+") {
                model_name->setBackground(0, addition_brush);
            } else if (type == "-") {
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
    addModelItem(folder, std::move(name), type, id);
}

void PickerWidget::genmapmeta(QStringList &bgm, QStringList &connections, QString path, int id,
                              const std::string &encoding) {
    QList<lcfops::connection_info> connections_raw;
    std::unique_ptr<lcf::rpg::Map> current_map = lcf::LMU_Reader::Load(
        QString(path + QString("/Map%1.lmu").arg(lcfops::paddedint(id, 4))).toUtf8().data(), encoding);
    for (lcf::rpg::Event i: current_map->events) {
        for (lcf::rpg::EventPage j: i.pages) {
            int last_teleport = 0;
            for (lcf::rpg::EventCommand k: j.event_commands) {
                if (k.code == int(lcf::rpg::EventCommand::Code::PlayBGM) && k.string != "(OFF)") {
                    // add bgm
                    lcf::rpg::Music located;
                    located.name = lcf::ToString(k.string);
                    located.fadein = k.parameters[0];
                    located.volume = k.parameters[1];
                    located.tempo = k.parameters[2];
                    located.balance = k.parameters[3];
                    QString bgmstring = lcfops::bgmstring(located, &i);
                    // strip duplicates
                    bool dirty = false;
                    for (QString l: bgm) {
                        if (l.first(l.lastIndexOf("-")) == bgmstring.first(bgmstring.lastIndexOf("-"))) {
                            dirty = true;
                        }
                    }
                    if (!dirty) {
                        // there are rare edge cases where a bgm is played after a teleport into another map
                        // since we have no way of guaranteeing the destination map has been modified, we instead note it on the teleport source
                        QString after_teleport;
                        if (last_teleport) {
                            after_teleport = QString(" (played after transfer to MAP[%1])").arg(
                                lcfops::paddedint(last_teleport, 4));
                        }
                        bgm.append(bgmstring + after_teleport);
                    }
                } else if (k.code == int(lcf::rpg::EventCommand::Code::Teleport) && k.parameters[0] != id) {
                    // add location
                    //QString mapstring = lcfops::mapstring(k.parameters[0], i.x, i.y, is_oneway(id, k.parameters[0], QString(path + QString("/Map%1.lmu").arg(lcfops::paddedint(k.parameters[0], 4)))));
                    connections_raw.append(
                        lcfops::connection_info(k.parameters[0], k.parameters[1], k.parameters[2], i.x, i.y, id, path));
                    last_teleport = k.parameters[0];
                }
            }
        }
    }
    // merge adjacent warps (max 3 tiles distance) with shared destinations
    // we can represent every connection as a qrect of width/height 5, where the center is the connecting event
    // by getting the bounding rectangle of every intersecting rectangle and shortening all sides by 2 we get the connection cluster bounds
    QList<lcfops::connection_info> clusters;
    QMargins m(2, 2, 2, 2);
    for (lcfops::connection_info i: connections_raw) {
        QRect a = i.xy.marginsAdded(m);
        QRect b = i.dest_xy.marginsAdded(m);
        bool found_cluster = false;
        for (lcfops::connection_info &j: clusters) {
            if ((j.xy & a).isValid() && (j.dest_xy & b).isValid() && (j.dest == i.dest)) {
                j.xy = j.xy.united(a);
                j.dest_xy = j.dest_xy.united(b);
                found_cluster = true;
                break;
            }
        }
        if (!found_cluster) {
            clusters.append(lcfops::connection_info(i.dest, b, a, i.id, i.path));
        }
    }
    for (const lcfops::connection_info &i: clusters) {
        connections.append(lcfops::mapstring(i.dest, i.dest_xy.marginsRemoved(m), i.xy.marginsRemoved(m),
                                             is_oneway(i.id, i.dest,
                                                       QString(i.path + QString("/Map%1.lmu").arg(
                                                                   lcfops::paddedint(i.dest, 4))), encoding)));
    }
}

void PickerWidget::gendiff() {
    changelog = chgen::ChangelogGenerator::scan();

    // Populate the model
    for (const auto &map: changelog->maps) {
        addModelItem("Map", QString::fromStdString("[" + data::id_string(map.data.ID) + "] - " + ToString(map.data.name)), QString::fromStdString(data::status_string(map.status)));
    }

    for (const auto &menu_theme: changelog->menu_themes) {
        addModelItem("System", QString::fromStdString(menu_theme.filename), QString::fromStdString(data::status_string(menu_theme.status)));
    }

    for (const auto &charset: changelog->charsets) {
        addModelItem("CharSet", QString::fromStdString(charset.filename), QString::fromStdString(data::status_string(charset.status)));
    }

    for (const auto &chipset: changelog->chipsets) {
        addModelItem("ChipSet", QString::fromStdString(chipset.filename), QString::fromStdString(data::status_string(chipset.status)));
    }

    for (const auto &music: changelog->musics) {
        addModelItem("Music", QString::fromStdString(music.filename), QString::fromStdString(data::status_string(music.status)));
    }

    for (const auto &sound: changelog->sounds) {
        addModelItem("Sound", QString::fromStdString(sound.filename), QString::fromStdString(data::status_string(sound.status)));
    }

    for (const auto &panorama: changelog->panoramas) {
        addModelItem("Panorama", QString::fromStdString(panorama.filename), QString::fromStdString(data::status_string(panorama.status)));
    }

    for (const auto &picture: changelog->pictures) {
        addModelItem("Picture", QString::fromStdString(picture.filename), QString::fromStdString(data::status_string(picture.status)));
    }

    for (const auto &animation_file : changelog->animation_files) {
        addModelItem("Battle", QString::fromStdString(animation_file.filename), QString::fromStdString(data::status_string(animation_file.status)));
    }

    ui->treeWidget->sortItems(0, Qt::SortOrder::AscendingOrder);

    // Populate the model with database info
    for (const auto &ce: changelog->common_events) {
        addModelItem("CE", QString::fromStdString("[" + data::id_string(ce.data.ID) + "] - " + lcf::ToString(ce.data.name)), QString::fromStdString(data::status_string(ce.status)));
    }

    for (const auto &tileset: changelog->tilesets) {
        addModelItem("Tileset", QString::fromStdString("[" + data::id_string(tileset.data.ID) + "] - " + lcf::ToString(tileset.data.name)), QString::fromStdString(data::status_string(tileset.status)));
    }

    for (const auto &sw: changelog->switches) {
        addModelItem("S", QString::fromStdString("[" + data::id_string(sw.data.ID) + "] - " + lcf::ToString(sw.data.name)), QString::fromStdString(data::status_string(sw.status)));
    }

    for (const auto &variable: changelog->variables) {
        addModelItem("V", QString::fromStdString("[" + data::id_string(variable.data.ID) + "] - " + lcf::ToString(variable.data.name)), QString::fromStdString(data::status_string(variable.status)));
    }

    for (const auto &animation: changelog->animations) {
        addModelItem("Animation", QString::fromStdString("[" + data::id_string(animation.data.ID) + "] - " + lcf::ToString(animation.data.name)), QString::fromStdString(data::status_string(animation.status)));
    }

    for (const auto &battler_animation: changelog->battler_animations) {
        addModelItem("BattlerAnim", QString::fromStdString("[" + data::id_string(battler_animation.data.ID) + "] - " + lcf::ToString(battler_animation.data.name)), QString::fromStdString(data::status_string(battler_animation.status)));
    }

    for (const auto &troop: changelog->troops) {
        addModelItem("Troop", QString::fromStdString("[" + data::id_string(troop.data.ID) + "] - " + lcf::ToString(troop.data.name)), QString::fromStdString(data::status_string(troop.status)));
    }

    for (const auto &state: changelog->states) {
        addModelItem("State", QString::fromStdString("[" + data::id_string(state.data.ID) + "] - " + lcf::ToString(state.data.name)), QString::fromStdString(data::status_string(state.status)));
    }

    for (const auto &skill: changelog->skills) {
        addModelItem("Skill", QString::fromStdString("[" + data::id_string(skill.data.ID) + "] - " + lcf::ToString(skill.data.name)), QString::fromStdString(data::status_string(skill.status)));
    }

    for (const auto &item: changelog->items) {
        addModelItem("Item", QString::fromStdString("[" + data::id_string(item.data.ID) + "] - " + lcf::ToString(item.data.name)), QString::fromStdString(data::status_string(item.status)));
    }

    for (const auto &enemy: changelog->enemies) {
        addModelItem("Enemy", QString::fromStdString("[" + data::id_string(enemy.data.ID) + "] - " + lcf::ToString(enemy.data.name)), QString::fromStdString(data::status_string(enemy.status)));
    }

    for (const auto &element: changelog->elements) {
        addModelItem("Element", QString::fromStdString("[" + data::id_string(element.data.ID) + "] - " + lcf::ToString(element.data.name)), QString::fromStdString(data::status_string(element.status)));
    }

    for (const auto &c: changelog->classes) {
        addModelItem("Class", QString::fromStdString("[" + data::id_string(c.data.ID) + "] - " + lcf::ToString(c.data.name)), QString::fromStdString(data::status_string(c.status)));
    }

    for (const auto &actor: changelog->actors) {
        addModelItem("Actor", QString::fromStdString("[" + data::id_string(actor.data.ID) + "] - " + lcf::ToString(actor.data.name)), QString::fromStdString(data::status_string(actor.status)));
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
