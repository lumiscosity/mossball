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

#include "changelogwidget.h"
#include "./ui_changelogwidget.h"
#include "lcfops.h"

#include <lcf/dbstring.h>
#include <lcf/ldb/reader.h>
#include <QCryptographicHash>
#include <QDirIterator>
#include <QTreeWidgetItem>

ChangelogWidget::ChangelogWidget(QWidget *parent) : QWidget(parent), ui(new Ui::ChangelogWidget) {
    ui->setupUi(this);
    // checkbox + name, diff type
    ui->treeWidget->hideColumn(1);
}

ChangelogWidget::~ChangelogWidget() {
    delete ui;
}

void ChangelogWidget::addModelItem(QString folder, QString name, QString type) {
    // the model consists of two columns: a checkbox next to the name (of either a folder or a file) and a diff type (files only)
    // an empty diff type signifies a folder
    for (auto &i: ui->treeWidget->findItems(folder, Qt::MatchExactly)) {
        auto bogus = i->text(1);
        if (i->text(1) == ""){
            QTreeWidgetItem *model_name = new QTreeWidgetItem(i);
            model_name->setText(0, name);
            model_name->setText(1, type);
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
    model_name->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate | Qt::ItemIsEnabled);
    model_name->setCheckState(0, Qt::CheckState::Unchecked);
    ui->treeWidget->addTopLevelItem(model_name);
    addModelItem(folder, name, type);
}

void ChangelogWidget::gendiff(QString orig_path, QString work_path) {
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
        }
    }
    for (QString i : additions) {
        if (i.contains("/")){
            QStringList temp = i.split("/");
            addModelItem(temp[0], temp[1], "+");
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
                addModelItem("Maps", i, "+");
                continue;
            } else if (!orig_empty && work_empty) {
                addModelItem("Maps", i, "-");
                continue;
            } else {
                addModelItem("Maps", i, "*");
                continue;
            }
        }
    }
    ui->treeWidget->sortItems(0, Qt::SortOrder::AscendingOrder);
    // get ldb data
    // std::unique_ptr<lcf::rpg::Database> orig_db = lcf::LDB_Reader::load(orig_path + "RPG_RT.ldb", "UTF-8");
}
