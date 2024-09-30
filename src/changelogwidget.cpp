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
#include <bit7z/bitarchivewriter.hpp>

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
    QString out = QFileDialog::getSaveFileName(this, "Select save location", "", "7z Archive (*.7z)");
    if (!out.isEmpty()) {
        auto bytes = ui->plainTextEdit->toPlainText().toUtf8();
        std::vector<unsigned char> c(bytes.constData(), bytes.constData() + bytes.size());
        // create an archive from all the files and the changelog
        // i don't actually trust people to not remove stuff after the treeview step, so we treat the changelog as the file list instead
        // this does pose some annoyances with filenames, which can contain spaces, but we have a fallback
        try {
            using namespace bit7z;

            #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            Bit7zLibrary lib("7z.dll");
            #else
            Bit7zLibrary lib("7z.so");
            #endif
            BitArchiveWriter archive{ lib, BitFormat::SevenZip };
            QRegularExpression ex("^[^ ]* [^ ]*\[.*$");
            QRegularExpression fileex("(?:\\S+\\s+){1}(.+?)(?:\(|$)");

            for (QString i : ui->plainTextEdit->toPlainText().split("\n")) {
                if (QStringList{"+", "-", "*"}.contains(i.first(1))) {
                    if (ex.match(i).hasMatch() && i.mid(2, 3) == "MAP" && i.contains("]")) {
                        archive.addFile(QString(work_dir + QString("/Map%1.lmu").arg(i.split("[")[0].split("]")[0])).toStdString());
                    } else if (!ex.match(i).hasMatch() && i.split(" ").size() >= 2) {
                        archive.addFile(QString(work_dir + QString("/%1/%2").arg(i.split(" ")[1]).arg(fileex.match(i).captured())).toStdString());
                    }
                }
            }
            archive.addFile(c, "changelog.txt");
            archive.addFile(work_dir.toStdString() + "/RPG_RT.lmt");
            archive.addFile(work_dir.toStdString() + "/RPG_RT.ldb");

            archive.compressTo(out.toStdString());
            QMessageBox::information(this, "Success", "Patch compiled successfully.");
            this->close();
        } catch ( const bit7z::BitException& ex ) {
            QMessageBox::critical(this, "Error", QString("An error occured when compiling: %1 (error code %2) \nEnsure that you haven't broken the changelog formatting and the the files detected are present in the work copy, then try again. In case of continued failure, please report the issue in Mossball's repository.").arg(ex.what()).arg(ex.code().value()));
        }
    }
}

