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

#include "../lcfops.h"
#include "../../third_party/easyrpg_editor/dbstring.h"
#include <QDialog>
#include <QStandardItemModel>
#include <QTreeWidget>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class PickerWidget;
}
QT_END_NAMESPACE

class PickerWidget : public QDialog {
    Q_OBJECT

public:
    PickerWidget(QWidget *parent = nullptr);
    ~PickerWidget();

    void sendWarning(const QString &message);

    void addModelItem(const QString& folder, QString name, QString type, int id = 0);

    void gendiff();
    QString genlog();
    void genmapmeta(QStringList &bgm, QStringList &connections, QString path, int id, const std::string& encoding);
private:
    Ui::PickerWidget *ui;
    QMap<int, QList<int>> map_outgoing;

    std::shared_ptr<data::Changelog> changelog;
    
    bool is_oneway(int from_id, int to_id, QString to_map, std::string encoding);

    QBrush addition_brush = QBrush(QColor(176, 237, 125, 50));
    QBrush removal_brush = QBrush(QColor(237, 127, 125, 50));
    QBrush modified_brush = QBrush(QColor(237, 208, 125, 50));
};
