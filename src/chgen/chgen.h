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
#include "../data/changelog.h"
#include "../ui/pickerwidget.h"
#include <lcf/lmt/reader.h>

namespace fs = std::filesystem;

namespace chgen {

    class ChangelogGenerator {
    public:
        /**
         * @brief Scans the base and modified paths for changes.
         * @return A changelog object.
         */
        static std::shared_ptr<data::Changelog> scan();

        /**
         * @brief Generates a changelog file.
         * @param changelog
         */
        static void generate_changelog_file(const std::shared_ptr<data::Changelog> &changelog, const std::string &at = "");
    };

}
