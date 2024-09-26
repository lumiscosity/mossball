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

void PickerWidget::addModelItem(QString folder, QString name, QString type, int id = 0) {
    // the model consists of three columns:
    // - a checkbox next to the name (of either a folder or a file),
    // - a diff type (files only),
    // - and whether it is file-based or id-based (maps are an outlier and a re handled separately).
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

void PickerWidget::gendiff(QString orig_path, QString work_path) {
    // generate a list of files
    QDirIterator orig_iter(orig_path, QDirIterator::Subdirectories);
    QDirIterator work_iter(work_path, QDirIterator::Subdirectories);
    QList<QString> orig;
    while (orig_iter.hasNext()) {
        orig.push_back(orig_iter.next().remove(orig_path).removeFirst());
    }
    QList<QString> work;
    while (work_iter.hasNext()) {
        work.push_back(work_iter.next().remove(work_path).removeFirst());
    }
    // ...and use it to create a list of differences
    QList<QString> removals, additions, shared;
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
                    log.append(QStringList{item->text(1), QString("MAP[%1]").arg(QString::number(id).rightJustified(4, char(48))), (name.isEmpty() ? "" : QString("- %1").arg(name))}.join(" "));
                    // add bgm info
                    if (maptree->maps[id].music.name != "(OFF)" && maptree->maps[id].music_type == 0) {
                        QString mainbgm = lcfops::bgmstring(maptree->maps[id].music);
                    }
                    // add other bgms and connections found in the map
                    // this also checks for changed and removed connections
                    QStringList miscbgm;
                    QStringList connections;
                    log.append(miscbgm.join("\n"));
                    log.append(connections.join("\n"));
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
