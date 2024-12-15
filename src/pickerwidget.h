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

#include "lcfops.h"
#include "../third_party/easyrpg_editor/dbstring.h"
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

    void addModelItem(QString folder, QString name, QString type, int id = 0);
    void gendiff(QString orig_path, QString work_path);
    QString genlog(QString orig_path, QString work_path);
    void genmapmeta(QStringList &bgm, QStringList &connections, QString path, int id);
private:
    Ui::PickerWidget *ui;
    QMap<int, QList<int>> map_outgoing;

    bool is_oneway(int from_id, int to_id, QString to_map);

    template <class T> void dbdiff(std::vector<T> orig, std::vector<T> work, QString folder) {
        if (orig.size() < work.size()) {
            // note non-empty additions in new chunks
            for (int i = orig.size(); i <= work.size(); i++) {
                if (work[i-1] != T()) {
                    addModelItem(folder, lcfops::id_with_name(i+1, ToQString(work[i-1].name)), "+", 1);
                }
            }
        } else if (orig.size() > work.size()) {
            // note non-empty removals in removed chunks
            for (int i = work.size(); i <= orig.size(); i++) {
                if (orig[i-1] != T()) {
                    addModelItem(folder, lcfops::id_with_name(i+1, ToQString(orig[i-1].name)), "i", 1);
                }
            }
        }
        // note additions for slots shared between both databases
        for (int i = 0, total = (work.size() < orig.size() ? work.size() - 1 : orig.size() - 1); i <= total; ++i) {
            if (orig[i] != work[i]) {
                QString comp = lcfops::compare<T>(orig[i], work[i]);
                if (comp == "-") {
                    addModelItem(folder, lcfops::id_with_name(i+1, ToQString(orig[i].name)), comp, 1);
                } else {
                    addModelItem(folder, lcfops::id_with_name(i+1, ToQString(work[i].name)), comp, 1);
                }
            }
        }
    }

    QBrush addition_brush = QBrush(QColor(176, 237, 125, 50));
    QBrush removal_brush = QBrush(QColor(237, 127, 125, 50));
    QBrush modified_brush = QBrush(QColor(237, 208, 125, 50));
};
