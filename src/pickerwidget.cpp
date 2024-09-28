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
#include <lcf/lmt/reader.h>
#include <lcf/lmu/reader.h>
#include <QCryptographicHash>
#include <QDirIterator>
#include <QTreeWidgetItem>

PickerWidget::PickerWidget(QWidget *parent) : QDialog(parent), ui(new Ui::PickerWidget) {
    ui->setupUi(this);
    // checkbox + name, diff type, id
    ui->treeWidget->hideColumn(1);
    ui->treeWidget->hideColumn(2);
}

PickerWidget::~PickerWidget() {
    delete ui;
}

bool PickerWidget::is_oneway(int from_id, int to_id, QString to_map) {
    // map_outgoing is used as a cache
    if (map_outgoing.contains(to_id)) {
        return !map_outgoing[to_id].contains(from_id);
    } else {
        // find and cache the destination id of every transfer in the map
        std::unique_ptr<lcf::rpg::Map> map = lcf::LMU_Reader::Load(to_map.toStdString());
        map_outgoing[to_id] = QList<int>();
        for (lcf::rpg::Event i : map->events) {
            for (lcf::rpg::EventPage j : i.pages) {
                for (lcf::rpg::EventCommand k : j.event_commands) {
                    if (k.code == int(lcf::rpg::EventCommand::Code::Teleport)) {
                        map_outgoing[to_id].append(k.parameters[0]);
                    }
                }
            }
        }
        return is_oneway(from_id, to_id, to_map);
    }
}

void PickerWidget::addModelItem(QString folder, QString name, QString type, int id) {
    // the model consists of three columns:
    // - a checkbox next to the name (of either a folder or a file),
    // - a diff type (files only),
    // - and whether it is file-based or id-based (maps are an outlier and are handled separately).
    // an empty diff type signifies a folder.
    for (auto &i: ui->treeWidget->findItems(folder, Qt::MatchExactly)) {
        auto bogus = i->text(1);
        if (i->text(1) == ""){
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
    addModelItem(folder, name, type, id);
}

void PickerWidget::genmapmeta(QStringList &bgm, QStringList &connections, QString path, int id) {
    QList<lcfops::connection_info> connections_raw;
    std::unique_ptr<lcf::rpg::Map> current_map = lcf::LMU_Reader::Load(QString(path + QString("/Map%1.lmu").arg(lcfops::paddedint(id, 4))).toStdString());
    for (lcf::rpg::Event i : current_map->events) {
        for (lcf::rpg::EventPage j : i.pages) {
            int last_teleport = 0;
            for (lcf::rpg::EventCommand k : j.event_commands) {
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
                    for (QString l : bgm) {
                        if (l.first(l.lastIndexOf("-")) == bgmstring.first(bgmstring.lastIndexOf("-"))) {
                            dirty = true;
                        }
                    }
                    if (!dirty) {
                        // there are rare edge cases where a bgm is played after a teleport into another map
                        // since we have no way of guaranteeing the destination map has been modified, we instead note it on the teleport source
                        QString after_teleport;
                        if (last_teleport) {
                            after_teleport = QString(" (played after transfer to MAP[%1])").arg(lcfops::paddedint(last_teleport, 4));
                        }
                        bgm.append(bgmstring + after_teleport);
                    }
                } else if (k.code == int(lcf::rpg::EventCommand::Code::Teleport) && k.parameters[0] != id) {
                    // add location
                    //QString mapstring = lcfops::mapstring(k.parameters[0], i.x, i.y, is_oneway(id, k.parameters[0], QString(path + QString("/Map%1.lmu").arg(lcfops::paddedint(k.parameters[0], 4)))));
                    connections_raw.append(lcfops::connection_info(k.parameters[0], i.x, i.y, id, path));
                    last_teleport = k.parameters[0];
                }
            }
        }
    }
    // merge adjacent warps (max 3 tiles distance) with shared destinations
    // we can represent every connection as a qrect of width/height 5, where the center is the connecting event
    // by getting the bounding rectangle of every intersecting rectangle and shortening all sides by 2 we get the connection cluster bounds
    // TODO: still needs deduplication on the opposite end to ensure we only combine warps leading to adjacent spots
    QList<lcfops::connection_info> clusters;
    QMargins m(2, 2, 2, 2);
    for (lcfops::connection_info i : connections_raw) {
        QRect a = i.xy.marginsAdded(m);
        bool found_cluster = false;
        for (lcfops::connection_info &j : clusters) {
            if ((j.xy & a).isValid()) {
                j.xy = j.xy.united(a);
                found_cluster = true;
                break;
            }
        }
        if (!found_cluster) {
            clusters.append(lcfops::connection_info(i.dest, a, i.id, i.path));
        }
    }
    for (lcfops::connection_info i : clusters) {
        connections.append(lcfops::mapstring(i.dest, i.xy.marginsRemoved(m), is_oneway(i.id, i.dest, QString(i.path + QString("/Map%1.lmu").arg(lcfops::paddedint(i.dest, 4))))));
    }
}

void PickerWidget::gendiff(QString orig_path, QString work_path) {
    // generate a list of files
    QDirIterator orig_iter(orig_path, QDirIterator::Subdirectories);
    QDirIterator work_iter(work_path, QDirIterator::Subdirectories);
    QStringList orig;
    while (orig_iter.hasNext()) {
        orig.push_back(orig_iter.next().remove(orig_path).removeFirst());
    }
    QStringList work;
    while (work_iter.hasNext()) {
        work.push_back(work_iter.next().remove(work_path).removeFirst());
    }
    // ...and use it to create a list of differences
    QStringList removals, additions, shared;
    for (QString i : orig) {
        if (!work.contains(i)) {
            removals.push_back(i);
        } else {
            // shared. only add them if the files actually differ
            QFileInfo a(orig_path + "/" + i);
            QFileInfo b(work_path + "/" + i);
            if (a.lastModified() != b.lastModified()){
                shared.push_back(i);
            }
        }
    }    
    for (QString i : work) {
        if (!orig.contains(i)) {
            additions.push_back(i);
        }
    }
    // clean up the lists
    removals.removeIf( [](const auto& i) { return !(i.contains("/") || i.endsWith(".lmu")) || i.endsWith("."); } );
    additions.removeIf( [](const auto& i) { return !(i.contains("/") || i.endsWith(".lmu")) || i.endsWith("."); } );
    shared.removeIf( [](const auto& i) { return !(i.contains("/") || i.endsWith(".lmu")) || i.endsWith("."); } );
    // populate the model
    // files are split into folders. maps are placed in the maps category
    for (QString i : removals) {
        if (i.contains("/")){
            QStringList temp = i.split("/");
            addModelItem(temp[0], temp[1], "-");
        } else {
            addModelItem("Map", i, "-");
        }
    }
    for (QString i : additions) {
        if (i.contains("/")){
            QStringList temp = i.split("/");
            addModelItem(temp[0], temp[1], "+");
        } else {
            addModelItem("Map", i, "+");
        }
    }
    for (QString i : shared) {
        if (i.contains("/")){
            QStringList temp = i.split("/");
            addModelItem(temp[0], temp[1], "*");
        } else {
            // due to map files being pre-filled, we need an alternative way of checking if they've been added or removed
            // the md5 hash seen below is the hash for an empty map
            QFile orig_map(orig_path + "/" + i);
            QFile work_map(work_path + "/" + i);

            QCryptographicHash orig_hash(QCryptographicHash::Md5);
            orig_map.open(QFile::OpenMode::fromInt(1));
            orig_hash.addData(&orig_map);
            bool orig_empty = orig_hash.result() == QByteArray::fromHex("ad9759db24c2c26d63c86c6a75d18370");

            QCryptographicHash work_hash(QCryptographicHash::Md5);
            work_map.open(QFile::OpenMode::fromInt(1));
            work_hash.addData(&work_map);
            bool work_empty = work_hash.result() == QByteArray::fromHex("ad9759db24c2c26d63c86c6a75d18370");

            if (orig_empty && !work_empty) {
                addModelItem("Map", i, "+");
                continue;
            } else if (!orig_empty && work_empty) {
                addModelItem("Map", i, "-");
                continue;
            } else {
                addModelItem("Map", i, "*");
                continue;
            }
        }
    }
    ui->treeWidget->sortItems(0, Qt::SortOrder::AscendingOrder);
    // get ldb data
    std::unique_ptr<lcf::rpg::Database> orig_db = lcf::LDB_Reader::Load((orig_path + "/RPG_RT.ldb").toStdString());
    std::unique_ptr<lcf::rpg::Database> work_db = lcf::LDB_Reader::Load((work_path + "/RPG_RT.ldb").toStdString());
    // tilesets
    dbdiff(orig_db->chipsets, work_db->chipsets, "Tileset");
    // terrains
    dbdiff(orig_db->terrains, work_db->terrains, "Terrain");
    // items
    dbdiff(orig_db->items, work_db->items, "Item");
    // animations
    dbdiff(orig_db->animations, work_db->animations, "Animation");
    // actors
    dbdiff(orig_db->actors, work_db->actors, "Actor");
    // variables
    dbdiff(orig_db->variables, work_db->variables, "Variable");
    // switches
    dbdiff(orig_db->switches, work_db->switches, "Switch");
    // CEs
    dbdiff(orig_db->commonevents, work_db->commonevents, "CE");

    // yoink the maps and put them up top
    for (auto &i: ui->treeWidget->findItems("", Qt::MatchExactly, 1)) {
        if (i->text(0) == "Map") {
            ui->treeWidget->insertTopLevelItem(0, ui->treeWidget->takeTopLevelItem(ui->treeWidget->indexFromItem(i, 0).row()));
        }
    }
}

QString PickerWidget::genlog(QString orig_path, QString work_path) {
    // create log header
    QStringList log;
    std::unique_ptr<lcf::rpg::TreeMap> maptree = lcf::LMT_Reader::Load(QString(work_path + "/RPG_RT.lmt").toStdString());
    log.append("|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|");
    log.append("");
    log.append("Developer:");
    log.append("Date: " + QDate::currentDate().toString("dd/MMM/yyyy"));
    log.append("");
    log.append("---------------------------------------------------");
    log.append("Map policy:");
    log.append("Asset policy:");
    log.append("---------------------------------------------------");
    // iterate over each folder and add it to the changelog
    for (auto &i: ui->treeWidget->findItems("", Qt::MatchExactly, 1)) {
        for (int j = 0, total = i->childCount(); total > j; ++j) {
            auto item = i->child(j);
            if (!item->isDisabled()) {
                if (i->text(0) == "Map" && i->text(1) == "") {
                    // map
                    // create named entry
                    int id = item->text(0).split(".")[0].replace(0, 3, "").toInt();
                    QString name = ToQString(maptree->maps[id].name).replace(0, 5, "");
                    log.append(QStringList{item->text(1), QString("MAP[%1]").arg(lcfops::paddedint(id, 4)), (name.isEmpty() ? "" : QString("- %1").arg(name))}.join(" "));
                    // add bgm and connection info
                    QStringList bgm;
                    QStringList connections;
                    // if the map is new and has a bgm, add it
                    if (item->text(1) == "+" && maptree->maps[id].music.name != "(OFF)" && maptree->maps[id].music_type == 2 && maptree->maps[id].music.volume != 0) {
                        bgm.append("    | + " + lcfops::bgmstring(maptree->maps[id].music));
                    }
                    // load bgms and connections in both versions of the map
                    QStringList orig_bgm;
                    QStringList orig_connections;
                    // only diff if it's not a new map (removed maps have no meta info)
                    if (item->text(1) == "*") {
                        genmapmeta(orig_bgm, orig_connections, orig_path, id);
                    }
                    QStringList work_bgm;
                    QStringList work_connections;
                    genmapmeta(work_bgm, work_connections, work_path, id);
                    // diff the lists
                    if (item->text(1) == "*") {
                        // modified maps
                        for (QString i : orig_bgm) {
                            if (!work_bgm.contains(i)) {
                                // removed
                                bgm.append("    | - " + i);
                            }
                        }
                        for (QString i : work_bgm) {
                            if (!orig_bgm.contains(i)) {
                                // added
                                bgm.append("    | + " + i);
                            }
                        }
                        for (QString i : orig_connections) {
                            if (!work_connections.contains(i)) {
                                // removed
                                connections.append("    | - " + i);
                            }
                        }
                        for (QString i : work_connections) {
                            if (!orig_connections.contains(i)) {
                                // added
                                connections.append("    | + " + i);
                            }
                        }
                    } else if (item->text(1) == "+") {
                        // added maps
                        for (QString j : work_bgm) {
                            bgm.append("    | + " + j);
                        };
                        for (QString j : work_connections) {
                            connections.append("    | + " + j);
                        };
                    }
                    if (!bgm.isEmpty()) {
                        log.append(bgm.join("\n"));
                    }
                    if (!connections.isEmpty()) {
                        log.append(connections.join("\n"));
                    }
                } else if (item->text(2) == "0") {
                    // no id
                    log.append(QStringList{item->text(1), i->text(0), item->text(0)}.join(" "));
                } else {
                    // with id
                    log.append(QStringList{item->text(1), i->text(0) + item->text(0)}.join(" "));
                }
            }
        }
        log.append("---------------------------------------------------");
    }

    // log footer
    log.append("");
    log.append("|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|");
    return log.join("\n");
}
