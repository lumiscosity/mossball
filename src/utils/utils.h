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
#include <filesystem>

namespace fs = std::filesystem;

namespace utils {
    /**
     * @brief Compares two files. Returns true if they are the same, false otherwise.
     * @return True if the files are the same, false otherwise.
     */
    bool compareFiles(const fs::path &path1, const fs::path &path2);
}
