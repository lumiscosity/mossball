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
#include <QMessageBox>

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
    if (!path.isEmpty()) {
        if (!QFile(path + "/RPG_RT.ldb").exists()) {
            if (
            !(QMessageBox::warning(
                this,
                "Error",
                "This directory doesn't have a database file! You may have meant to select the folder inside this one. Continue anyways?",
                QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
                QMessageBox::StandardButton::No)
             == QMessageBox::StandardButton::Yes)) {
                return;
            }
        }
        ui->origLabel->setText(path);
        ui->buttonBox->buttons()[0]->setEnabled(
            path != ui->workLabel->text() && ui->workLabel->text() != "..." && !ui->nameLineEdit->text().isEmpty());
    }
}

void DirectoryDialog::on_workPushButton_clicked() {
    QString path = QFileDialog::getExistingDirectory(this, "Select the work copy directory");
    qDebug() << "Selected work path: " + path;
    if (!path.isEmpty()) {
        if (!QFile(path + "/RPG_RT.ldb").exists()) {
            if (
            !(QMessageBox::warning(
                this,
                "Error",
                "This directory doesn't have a database file! You may have meant to select the folder inside this one. Continue anyways?",
                QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
                QMessageBox::StandardButton::No)
             == QMessageBox::StandardButton::Yes)) {
                return;
            }
        }
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
            return "932";
        case (2):
            return "949";
        case (3):
            return "1250";
        case (4):
            return "1251";
        case (5):
            return "1252";
        case (6):
            return "1253";
        case (7):
            return "1255";
        case (8):
            return "1256";
        case (9):
            return "874";
        case (10):
            return "936";
        case (11):
            return "950";
        case (12):
            return "1254";
        case (13):
            return "1257";
        default:
            return "UTF-8";
    }
}

QString DirectoryDialog::dev_name() {
    return ui->nameLineEdit->text();
}
