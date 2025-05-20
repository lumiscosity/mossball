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

#include "utils.h"

#include <fstream>
#include <QMessageBox>
#include <vector>

#include "../../third_party/easyrpg_editor/dbstring.h"

namespace utils {
    bool compareFiles(const fs::path &path1, const fs::path &path2) {
        std::ifstream img1(path1, std::ios::binary);
        std::ifstream img2(path2, std::ios::binary);

        if (!img1.is_open() || !img2.is_open()) {
            QMessageBox::critical(nullptr, "Error",
                                  ToQString(
                                      "Could not open image file " + (img1.is_open() ? path1.string() : path2.string())
                                      + " for comparison"));
            return false;
        }

        img1.seekg(0, std::ios::end);
        img2.seekg(0, std::ios::end);

        const auto img1_size = img1.tellg();
        const auto img2_size = img2.tellg();

        if (img1_size != img2_size) {
            return false;
        }

        img1.seekg(0, std::ios::beg);
        img2.seekg(0, std::ios::beg);

        std::vector<char> img1_data(img1_size);
        std::vector<char> img2_data(img2_size);

        img1.read(img1_data.data(), img1_size);
        img2.read(img2_data.data(), img2_size);

        return img1_data == img2_data;
    }
}
