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

#include "pickerwidget.h"
#include "directorydialog.h"
#include "changelogwidget.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    #ifdef _WIN32
    setlocale(LC_ALL, ".UTF8");
    #endif
    QApplication a(argc, argv);
    DirectoryDialog d;
    if (d.exec()) {
        PickerWidget p;
        p.gendiff(d.orig(), d.work());
        if (p.exec()) {
            ChangelogWidget c(d.work());
            c.set_text(p.genlog(d.orig(), d.work()));
            c.show();
            a.exec();
        }
    }
}
