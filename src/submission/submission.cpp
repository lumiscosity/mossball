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

#include "submission.h"

#include <QMessageBox>

namespace minidocx {
    class io_error;
}

namespace submission {
    void ChangelogGenerator::create_submission_archive() {
        // TODO use a changelog data struct instead of parsing
        minidocx::Zip z;
            z.open(out.toUtf8().data(), minidocx::Zip::OpenMode::Create);

            QRegularExpression ex("^[^ ]* [^ ]*\\[.*$");
            QRegularExpression fileex("^(?:\\S+\\s+){2}(.*?)(?:\\s*\\(|$)");

            const QString plain_text = SubmissionBuilder::ui->get_text();

            for (QString i : plain_text.split("\n")) {
                if (i.length() > 5) {
                    if (QStringList{"+", "*"}.contains(i.first(1))) {
                        if (ex.match(i).hasMatch() && i.mid(2, 3) == "MAP" && i.contains("]")) {
                            // map
                            QString map_string = QString("/Map%1.lmu").arg(i.split("[")[1].split("]")[0]);
                            try {
                                z.addFileFromDisk(map_string.toUtf8().data(), QString(Mossball::work_directory + map_string).toUtf8().data());
                            } catch (const minidocx::io_error& ex) {
                                QMessageBox::warning(this, "Warning", QString("Could not include file %1 in the zip file! It is already in the changelog. Add the file to the archive manually. Error code: %2").arg(map_string).arg(ex.what()));
                            }
                        } else if (!ex.match(i).hasMatch() && i.split(" ").size() >= 2) {
                            // file
                            QString file_string = QString("/%1/%2").arg(i.split(" ")[1]).arg(fileex.match(i).captured(1));
                            try {
                                z.addFileFromDisk(file_string.toUtf8().data(), QString(Mossball::work_directory + file_string).toUtf8().data());
                            } catch (const minidocx::io_error& ex) {
                                QMessageBox::warning(this, "Warning", QString("Could not include file %1 in the zip file! It is already in the changelog. Add the file to the archive manually. Error code: %2").arg(file_string).arg(ex.what()));
                            }
                        }
                    }
                }
            }

            z.addFileFromDisk("/RPG_RT.lmt", (Mossball::work_directory + "/RPG_RT.lmt").toUtf8().data());
            z.addFileFromDisk("/RPG_RT.ldb", (Mossball::work_directory + "/RPG_RT.ldb").toUtf8().data());
            z.addFileFromString("/changelog.txt", c);

            z.close();
    }
} // Submission