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

#include "ui/pickerwidget.h"
#include "ui/directorydialog.h"
#include "ui/changelogwidget.h"

#include <QApplication>

#include "mossball.h"
#include "chgen/chgen.h"
#include "submission/submission.h"

int main(int argc, char *argv[]) {
    #ifdef _WIN32
    setlocale(LC_ALL, ".UTF8");
    #endif
    QApplication a(argc, argv);
    DirectoryDialog d;
    if (d.exec()) {
        Mossball::origin_directory = d.orig();
        Mossball::work_directory = d.work();
        Mossball::encoding = d.encoding();
        Mossball::dev_name = d.dev_name();

        PickerWidget p;
        p.gendiff();

        if (p.exec()) {
            ChangelogWidget c;
            submission::SubmissionBuilder::ui = std::make_unique<ChangelogWidget>(&c);
            c.set_text(p.genlog());
            c.show();
            a.exec();
        }
    }
}
