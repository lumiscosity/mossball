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
#include "ui_changelogwidget.h"

#include "../third_party/zip/include/exceptions.hpp"
#include "../third_party/zip/include/zip.hpp"

#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>

ChangelogWidget::ChangelogWidget(QString work_dir, QWidget *parent) : work_dir(work_dir), QWidget(parent), ui(new Ui::ChangelogWidget) {
    ui->setupUi(this);
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->plainTextEdit->setFont(font);
}

ChangelogWidget::~ChangelogWidget() {
    delete ui;
}

void ChangelogWidget::set_text(QString text) {
    ui->plainTextEdit->setPlainText(text);
}

void ChangelogWidget::on_pushButton_clicked() {
    QString out = QFileDialog::getSaveFileName(this, "Select save location", "", "Archive (*.zip)");
    if (!out.isEmpty()) {
        auto c = ui->plainTextEdit->toPlainText().toUtf8().data();
        // create an archive from all the files and the changelog
        // i don't actually trust people to not remove stuff after the treeview step, so we treat the changelog as the file list instead
        // this does pose some annoyances with filenames, which can contain spaces, but we have a fallback

        try {
            minidocx::Zip z;
            z.open(out.toUtf8().data(), minidocx::Zip::OpenMode::Create);

            QRegularExpression ex("^[^ ]* [^ ]*\\[.*$");
            QRegularExpression fileex("^(?:\\S+\\s+){2}(.*?)(?:\\s*\\(|$)");

            for (QString i : ui->plainTextEdit->toPlainText().split("\n")) {
                if (i.length() > 5) {
                    if (QStringList{"+", "*"}.contains(i.first(1))) {
                        if (ex.match(i).hasMatch() && i.mid(2, 3) == "MAP" && i.contains("]")) {
                            // map
                            QString map_string = QString("Map%1.lmu").arg(i.split("[")[1].split("]")[0]);
                            try {
                                z.addFileFromDisk(map_string.toUtf8().data(), QString(work_dir + map_string).toUtf8().data());
                            } catch (const minidocx::io_error& ex) {
                                QMessageBox::warning(this, "Warning", QString("Could not include file %1 in the zip file! It is already in the changelog. Add the file to the archive manually.").arg(map_string));
                            }
                        } else if (!ex.match(i).hasMatch() && i.split(" ").size() >= 2) {
                            // file
                            QString file_string = QString("%1/%2").arg(i.split(" ")[1]).arg(fileex.match(i).captured(1));
                            try {
                                z.addFileFromDisk(file_string.toUtf8().data(), QString(work_dir + file_string).toUtf8().data());
                            } catch (const minidocx::io_error& ex) {
                                QMessageBox::warning(this, "Warning", QString("Could not include file %1 in the zip file! It is already in the changelog. Add the file to the archive manually.").arg(file_string));
                            }
                        }
                    }
                }
            }

            z.addFileFromString("changelog.txt", c);
            z.addFileFromDisk("/RPG_RT.lmt", (work_dir + "/RPG_RT.lmt").toUtf8().data());
            z.addFileFromDisk("/RPG_RT.ldb", (work_dir + "/RPG_RT.ldb").toUtf8().data());

            z.close();
            QMessageBox::information(this, "Success", "Patch compiled successfully.");
            this->close();

        }
        catch (const minidocx::exception& ex) {
            QMessageBox::critical(this, "Error", QString("An error occured while compiling: %1 \nEnsure that you haven't broken the changelog formatting and the the files detected are present in the work copy, then try again. In case of continued failure, please report the issue in Mossball's repository.").arg(ex.what()));
            return;
        }
    }
}

