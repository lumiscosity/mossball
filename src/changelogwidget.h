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

#pragma once

#include <QStandardItemModel>
#include <QTreeWidget>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class ChangelogWidget;
}
QT_END_NAMESPACE

class ChangelogWidget : public QWidget {
    Q_OBJECT

public:
    ChangelogWidget(QWidget *parent = nullptr);
    ~ChangelogWidget();

    void appendChild(QTreeWidgetItem *parent, QList<QTreeWidgetItem *> items);
    void addModelItem(QString folder, QString name, QString type);
    void gendiff(QString orig_path, QString work_path);

private:
    Ui::ChangelogWidget *ui;
    QStandardItemModel model;
};
