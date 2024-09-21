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

#include <QDialog>

namespace Ui {
class DirectoryDialog;
}

class DirectoryDialog : public QDialog {
    Q_OBJECT

public:
    explicit DirectoryDialog(QWidget *parent = nullptr);
    ~DirectoryDialog();

    QString orig();
    QString work();
private slots:
    void on_origPushButton_clicked();
    void on_workPushButton_clicked();

private:
    Ui::DirectoryDialog *ui;
};
