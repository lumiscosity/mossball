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

#include <QDirIterator>

ChangelogWidget::ChangelogWidget(QWidget *parent) : QWidget(parent), ui(new Ui::ChangelogWidget) {
    ui->setupUi(this);
    // checkbox, name, diff type
    model.setColumnCount(3);
    ui->treeView->setModel(&model);
}

ChangelogWidget::~ChangelogWidget() {
    delete ui;
}

void ChangelogWidget::addModelItem(QString folder, QString name, QString type) {
    // the model consists of three columns: a checkbox, a name (of either a folder or a file) and a diff type (files only)
    // an empty diff type signifies a folder
    for (auto i: model.findItems("", Qt::MatchExactly, 2)) {
        QStandardItem model_checkbox;
        QStandardItem model_name(name);
        QStandardItem model_type(type);
        QList<QStandardItem*> items = {&model_checkbox, &model_name, &model_type};
        i->appendRow(items);
        return;
    }
    // no folder found, make a new one and try again
    QStandardItem model_checkbox;
    QStandardItem model_name(folder);
    QStandardItem model_type;
    QList<QStandardItem*> items = {&model_checkbox, &model_name, &model_type};
    model.appendRow(items);
    return;
    addModelItem(folder, name, type);
}

void ChangelogWidget::gendiff(QString orig_path, QString work_path) {
    // generate a list of files
    QDirIterator orig_iter(orig_path, QDirIterator::Subdirectories);
    QDirIterator work_iter(work_path, QDirIterator::Subdirectories);
    QVector<QString> orig;
    while (orig_iter.hasNext()) {
        orig.push_back(orig_iter.next().remove(orig_path).removeFirst());
    }
    QVector<QString> work;
    while (work_iter.hasNext()) {
        work.push_back(work_iter.next().remove(work_path).removeFirst());
    }
    // ...and use it to create a list of differences
    QVector<QString> removals, additions, shared;
    for (QString i : orig) {
        if (!work.contains(i)) {
            removals.push_back(i);
        } else {
            // shared. only add them if the files actually differ
            QFileInfo a(orig_path + i);
            QFileInfo b(work_path + i);
            if (a.lastModified() != b.lastModified()){
                shared.push_back(i);
            }
        }
    }
    // ignore the ldb and lmt
    // the lmt is always copied in in full if map changes are found
    // the ldb changes will be added later
    shared.removeAll("RPG_RT.ldb");
    shared.removeAll("RPG_RT.lmt");
    for (QString i : work) {
        if (!orig.contains(i)) {
            additions.push_back(i);
        }
    }
    // populate the model
    // files are split into folders. maps are placed in the maps category
    for (QString i : removals) {
        if (i.contains("/")){
            QStringList temp = i.split("/");
            addModelItem(temp[0], temp[1], "-");
        } else {
            addModelItem("Maps", i, "-");
        }
    }
    for (QString i : additions) {
        if (i.contains("/")){
            QStringList temp = i.split("/");
            addModelItem(temp[0], temp[1], "+");
        } else {
            addModelItem("Maps", i, "+");
        }
    }
    for (QString i : shared) {
        if (i.contains("/")){
            QStringList temp = i.split("/");
            addModelItem(temp[0], temp[1], "*");
        } else {
            addModelItem("Maps", i, "*");
        }
    }
}
