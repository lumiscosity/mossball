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
    // checkbox + name, diff type
    model.setColumnCount(2);
    ui->treeView->setModel(&model);
    ui->treeView->hideColumn(1);
}

ChangelogWidget::~ChangelogWidget() {
    delete ui;
}

void ChangelogWidget::appendChild(QStandardItem* parent, QList<QStandardItem*> items) {
    int row = 0;
    while (parent->child(row) != nullptr) {
        row++;
    }
    int column = 0;
    for (auto *i : items) {
        parent->setChild(row, column, i);
        column++;
    }
}

void ChangelogWidget::addModelItem(QString folder, QString name, QString type) {
    // the model consists of two columns: a checkbox next to the name (of either a folder or a file) and a diff type (files only)
    // an empty diff type signifies a folder
    for (auto& i: model.findItems(folder, Qt::MatchExactly, 0)) {
        if (model.itemFromIndex(model.indexFromItem(i).siblingAtColumn(1))->text() == ""){
            QStandardItem *model_name = new QStandardItem(name);
            model_name->setCheckable(true);
            //model_name->setData(Qt::CheckStateRole);
            model_name->setFlags(Qt::ItemIsUserCheckable);
            model_name->setEnabled(true);
            model_name->setCheckState(Qt::CheckState::Unchecked);
            QStandardItem *model_type = new QStandardItem(type);
            QList<QStandardItem*> items = {model_name, model_type};
            // model.insertRow(0, items);
            // these two do nothing!
            appendChild(i, items);
            qWarning()<<i->text();
            return;
        }
    }
    // no folder found, make a new one and try again
    QStandardItem *model_name = new QStandardItem(folder);
    model_name->setCheckable(true);
    //model_name->setData(Qt::CheckStateRole);
    model_name->setFlags(Qt::ItemIsAutoTristate|Qt::ItemIsUserCheckable);
    model_name->setEnabled(true);
    QStandardItem *model_type = new QStandardItem("");
    QList<QStandardItem*> items = {model_name, model_type};
    model.appendRow(items);
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
            addModelItem("Maps", i, "*");
        }
    }
    // model.sort(0);
}
