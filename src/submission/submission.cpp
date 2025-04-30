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
#include "../mossball.h"
#include "../../third_party/zip/include/zip.hpp"
#include "../../third_party/zip/include/exceptions.hpp"

#include <QMessageBox>

namespace submission {
    void add_file(const QString &filename, minidocx::Zip &z) {
        try {
            z.addFileFromDisk(filename.toUtf8().data(),
                              QString(Mossball::work_directory + filename).toUtf8().data());
        } catch (const minidocx::io_error &ex) {
            QMessageBox::warning(nullptr, "Warning",
                                 QString(
                                     "Could not include file %1 in the zip file! It is already in the changelog. Add the file to the archive manually. Error code: %2")
                                 .arg(filename).arg(ex.what()));
        }
    }

    void add_asset_file(const std::vector<data::Asset> &assets, minidocx::Zip &z) {
        for (const auto &a: assets) {
            if (a.status == data::Status::ADDED || a.status == data::Status::MODIFIED) {
                QString asset_string = QString("/%1/%2").arg(
                    QString::fromStdString(data::asset_category_string(a.category)),
                    QString::fromStdString(a.filename));
                add_file(asset_string, z);
            }
        }
    }

    void SubmissionBuilder::create_submission_archive(const QString &out, std::shared_ptr<data::Changelog> changelog) {
        minidocx::Zip z;
        const QString out_extension = QString("%1.zip").arg(out.toUtf8().data());

        z.open(out_extension.toUtf8().data(), minidocx::Zip::OpenMode::Create);

        for (const auto &map: changelog->maps) {
            if (map.status == data::Status::ADDED || map.status == data::Status::MODIFIED) {
                QString map_string = QString("/Map%1.lmu").arg(QString::fromStdString(data::id_string(map.data.ID)));
                add_file(map_string, z);
            }
        }

        add_asset_file(changelog->menu_themes, z);
        add_asset_file(changelog->charsets, z);
        add_asset_file(changelog->chipsets, z);
        add_asset_file(changelog->musics, z);
        add_asset_file(changelog->sounds, z);
        add_asset_file(changelog->panoramas, z);
        add_asset_file(changelog->pictures, z);
        add_asset_file(changelog->animation_files, z);

        add_file("/RPG_RT.lmt", z);
        add_file("/RPG_RT.ldb", z);

        z.addFileFromString("/changelog.txt", changelog->stringify().toUtf8().data());

        z.close();
    }
}
