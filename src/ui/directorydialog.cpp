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
    ui->encodingComboBox->addItems({
        "UTF-8", "932 (Japanese)", "949 (Korean)", "1250 (Central Europe)", "1251 (Cyryllic)", "1252 (Occidental)",
        "1253 (Greek)", "1255 (Hebrew)", "1256 (Arabic)", "874 (Thai)", "936 (Chinese Simplified)",
        "950 (Chinese Traditional)", "1254 (Turkish)", "1257 (Baltic)"
    });
}

DirectoryDialog::~DirectoryDialog() {
    delete ui;
}

void DirectoryDialog::on_origPushButton_clicked() {
    QString path = QFileDialog::getExistingDirectory(this, "Select the original copy directory");
    qDebug() << "Selected origin path: " + path;
    if (!path.isEmpty()) {
        ui->origLabel->setText(path);
        ui->buttonBox->buttons()[0]->setEnabled(
            path != ui->workLabel->text() && ui->workLabel->text() != "..." && !ui->nameLineEdit->text().isEmpty());
    }
}

void DirectoryDialog::on_workPushButton_clicked() {
    QString path = QFileDialog::getExistingDirectory(this, "Select the work copy directory");
    qDebug() << "Selected work path: " + path;
    if (!path.isEmpty()) {
        ui->workLabel->setText(path);
        ui->buttonBox->buttons()[0]->setEnabled(
            path != ui->origLabel->text() && ui->origLabel->text() != "..." && !ui->nameLineEdit->text().isEmpty());
    }
}

void DirectoryDialog::on_nameLineEdit_textChanged(const QString &text) {
    ui->buttonBox->buttons()[0]->setEnabled(
        ui->workLabel->text() != ui->origLabel->text() && ui->origLabel->text() != "..." && ui->workLabel->text() !=
        "..." && !text.isEmpty());
}

QString DirectoryDialog::orig() {
    return ui->origLabel->text();
}

QString DirectoryDialog::work() {
    return ui->workLabel->text();
}

std::string DirectoryDialog::encoding() {
    switch (ui->encodingComboBox->currentIndex()) {
    case (1):
        return "ibm-943_P15A-2003";
    case (2):
        return "windows-949-2000";
    case (3):
        return "ibm-5346_P100-1998";
    case (4):
        return "ibm-5347_P100-1998";
    case (5):
        return "ibm-5348_P100-1997";
    case (6):
        return "ibm-5349_P100-1998";
    case (7):
        return "ibm-9447_P100-2002";
    case (8):
        return "ibm-9448_X100-2005";
    case (9):
        return "windows-874-2000";
    case (10):
        return "windows-936-2000";
    case (11):
        return "windows-950-2000";
    case (12):
        return "ibm-5350_P100-1998";
    case (13):
        return "ibm-9449_P100-2002";
    default:
        return "UTF-8";
    }
}

QString DirectoryDialog::dev_name() {
    return ui->nameLineEdit->text();
}
