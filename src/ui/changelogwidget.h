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

#include <QWidget>
#include "../data/changelog.h"

namespace Ui {
    class ChangelogWidget;
}

class ChangelogWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChangelogWidget(QWidget *parent = nullptr);

    ~ChangelogWidget();

    QString get_text();

    /**
     * @brief Prints the changelog in the UI
     * @attention A changelog needs to be scanned and set as the ChangelogWidget::changelog variable beforehand
     */
    void set_changelog_text();

    std::shared_ptr<data::Changelog> changelog;

private slots:
    /**
     * @brief Creates an archive from all the files and the changelog
     * @attention A changelog needs to be scanned and set as the ChangelogWidget::changelog variable beforehand
     */
    void on_pushButton_clicked();

private:
    Ui::ChangelogWidget *ui;
};
