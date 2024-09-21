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

#include "directorydialog.h"
#include "ui_directorydialog.h"

#include <QFileDialog>

DirectoryDialog::DirectoryDialog(QWidget *parent) : QDialog(parent), ui(new Ui::DirectoryDialog) {
    ui->setupUi(this);
    ui->buttonBox->buttons()[0]->setDisabled(true);
}

DirectoryDialog::~DirectoryDialog() {
    delete ui;
}

void DirectoryDialog::on_origPushButton_clicked() {
    QString path = QFileDialog::getExistingDirectory(this, "Select the original copy directory");
    if (!path.isEmpty()){
        ui->origLabel->setText(path);
        ui->buttonBox->buttons()[0]->setEnabled(path != ui->workLabel->text() && ui->workLabel->text() != "...");
    }
}

void DirectoryDialog::on_workPushButton_clicked() {
    QString path = QFileDialog::getExistingDirectory(this, "Select the work copy directory");
    if (!path.isEmpty()){
        ui->workLabel->setText(path);
        ui->buttonBox->buttons()[0]->setEnabled(path != ui->origLabel->text() && ui->origLabel->text() != "...");
    }
}

QString DirectoryDialog::orig() {
    return ui->origLabel->text();
}

QString DirectoryDialog::work() {
    return ui->workLabel->text();
}
