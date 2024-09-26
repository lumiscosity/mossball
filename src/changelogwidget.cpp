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

ChangelogWidget::ChangelogWidget(QWidget *parent) : QWidget(parent), ui(new Ui::ChangelogWidget) {
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
