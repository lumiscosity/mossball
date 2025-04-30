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

#include "../../third_party/zip/include/exceptions.hpp"

#include <QFileDialog>
#include <QMessageBox>
#include <QFontDatabase>

#include "../submission/submission.h"

ChangelogWidget::ChangelogWidget(QWidget *parent) : QWidget(parent), ui(new Ui::ChangelogWidget) {
    ui->setupUi(this);
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->plainTextEdit->setFont(font);
}

ChangelogWidget::~ChangelogWidget() {
    delete ui;
}

QString ChangelogWidget::get_text() {
    return ui->plainTextEdit->toPlainText();
}


void ChangelogWidget::set_changelog_text() {
    if (!changelog) {
        QMessageBox::critical(this, "Error", "No changelog scanned");
        return;
    }
    QString text = changelog->stringify();
    ui->plainTextEdit->setPlainText(text);
}

void ChangelogWidget::on_pushButton_clicked() {
    if (!changelog) {
        QMessageBox::critical(this, "Error", "No changelog scanned");
        return;
    }

    QString out = QFileDialog::getSaveFileName(this, "Select save location", "", "Archive (*.zip)");
    if (!out.isEmpty()) {
        auto c = ui->plainTextEdit->toPlainText().toStdString();

        try {
            submission::SubmissionBuilder::create_submission_archive(out, changelog);
            changelog.reset();
            QMessageBox::information(this, "Success", "Patch compiled successfully.");
            this->close();
        } catch (const minidocx::exception &ex) {
            QMessageBox::critical(this, "Error",
                                  QString(
                                      "An error occured while compiling: %1 \nEnsure that you haven't broken the changelog formatting and the the files detected are present in the work copy, then try again. In case of continued failure, please report the issue in Mossball's repository.")
                                  .arg(ex.what()));
            return;
        }
    }
}
