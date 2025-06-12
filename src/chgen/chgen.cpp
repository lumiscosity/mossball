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

#include "chgen.h"

#include <QDebug>
#include <QDirIterator>
#include <QMessageBox>

#include "../mossball.h"
#include  "../utils/utils.h"
#include "lcf/ldb/reader.h"
#include "lcf/lmu/reader.h"
#include "lcf/rpg/map.h"

using Commands = lcf::rpg::EventCommand::Code;

namespace chgen {
    /**
     * @brief Lists the content of a directory
     * @param path the path of the directory
     * @return a vector containing the names of the files in the directory
     */
    std::vector<std::string> list_directory_content(std::filesystem::path path) {
        std::vector<std::string> content;

        try {
            if (!fs::exists(path)) {
                QMessageBox::critical(nullptr, "Error", "The specified path does not exist.");
                return content;
            }

            if (!fs::is_directory(path)) {
                QMessageBox::critical(nullptr, "Error", "The specified path is not a directory.");
                return content;
            }

            for (const auto &file: fs::directory_iterator(path)) {
                content.push_back(file.path().filename().string());
            }
        } catch (const std::exception &e) {
            QMessageBox::critical(nullptr, "Error", e.what());
        }

        return content;
    }

    /**
     * @brief Lists all events containing a play music event in a map
     * @param map the map we want to analyze
     * @param base_track the name of the main music of the map; we don't want to scan bgm events that return to this track
     * @return a list of the BGMEvent objects
     */
    std::vector<data::BGMEvent> list_bgm_events(std::unique_ptr<lcf::rpg::Map> map, std::string base_track) {
        std::vector<data::BGMEvent> bgm_events;

        for (const auto &event: map->events) {
            for (const auto &page: event.pages) {
                for (const auto &command: page.event_commands) {
                    // We don't list the BGM event if it returns to the main music of the map
                    if (command.code == static_cast<int>(Commands::PlayBGM) && command.string.data() != base_track) {
                        data::BGMEvent bgm_event;

                        bgm_event.coordinates.x = event.x;
                        bgm_event.coordinates.y = event.y;
                        bgm_event.track_name = command.string.data();
                        auto params = command.parameters.begin();
                        bgm_event.volume = *std::next(params, 1);
                        bgm_event.speed = *std::next(params, 2);

                        bgm_events.push_back(bgm_event);
                    }
                }
            }
        }

        return bgm_events;
    }

    std::vector<data::Asset>
    add_assets(data::AssetCategory category) {
        const std::string base_path = Mossball::origin_directory.toStdString();
        const std::string modified_path = Mossball::work_directory.toStdString();
        std::vector<data::Asset> assets;

        std::string folder = data::asset_category_string(category);

        auto base_asset_content = list_directory_content(base_path / fs::path(folder));
        auto modified_asset_content = list_directory_content(modified_path / fs::path(folder));

        for (const auto &asset: base_asset_content) {
            const auto asset_name = asset.substr(0, asset.find_first_of('.'));
            /*if (asset_name.rfind("record_player_", 0) == 0) {
                // ignore record player assets
                continue;
            }*/

            if (std::find(std::begin(modified_asset_content), std::end(modified_asset_content), asset) ==
                std::end(modified_asset_content)) {
                // asset removed
                data::Asset changelog_asset;
                changelog_asset.category = category;
                changelog_asset.status = data::Status::REMOVED;
                changelog_asset.name = asset_name;
                changelog_asset.filename = asset;
                assets.push_back(changelog_asset);
            }
        }

        for (const auto &asset: modified_asset_content) {
            const auto asset_name = asset.substr(0, asset.find_first_of('.'));
            /*if (asset_name.rfind("record_player_", 0) == 0) {
                // ignore record player assets
                continue;
            }*/

            if (std::find(std::begin(base_asset_content), std::end(base_asset_content), asset) ==
                std::end(base_asset_content)) {
                // asset added
                data::Asset changelog_asset;
                changelog_asset.category = category;
                changelog_asset.status = data::Status::ADDED;
                changelog_asset.name = asset_name;
                changelog_asset.filename = asset;
                assets.push_back(changelog_asset);
            } else {
                // asset modified
                try {
                    const auto base_asset_path = base_path / fs::path(folder) / fs::path(asset);
                    const auto modified_asset_path = modified_path / fs::path(folder) / fs::path(asset);

                    const auto base_lastwritetime = fs::last_write_time(base_asset_path);
                    const auto modified_lastwritetime = fs::last_write_time(modified_asset_path);


                    if (base_lastwritetime != modified_lastwritetime) {
                        if (utils::compareFiles(base_asset_path, modified_asset_path)) {
                            // asset unchanged
                            continue;
                        }

                        data::Asset changelog_asset;
                        changelog_asset.category = category;
                        changelog_asset.status = data::Status::MODIFIED;
                        changelog_asset.name = asset_name;
                        changelog_asset.filename = asset;
                        assets.push_back(changelog_asset);
                    }
                } catch (const std::exception &e) {
                    QMessageBox::critical(nullptr, "Error", e.what());
                }
            }
        }

        return assets;
    }

    template<typename T, typename U>
    std::vector<T>
    add_db_entries(std::vector<U> base_list,
                   std::vector<U> modified_list) {
        std::vector<T> db_entries;

        for (size_t i = 0; i < std::min(base_list.size(), modified_list.size()); i++) {
            const auto &base = base_list[i];
            const auto &modified = modified_list[i];

            if (base == modified) {
                // no modification
                continue;
            }

            T changelog_entry;
            changelog_entry.data.ID = modified.ID;
            changelog_entry.data.name = modified.name;

            if (base.name.empty()) {
                // added
                changelog_entry.status = data::Status::ADDED;
            } else if (modified.name.empty()) {
                // removed
                changelog_entry.status = data::Status::REMOVED;
                changelog_entry.data.name = base.name;
            } else {
                // modified
                changelog_entry.status = data::Status::MODIFIED;
            }

            db_entries.push_back(changelog_entry);
        }

        return db_entries;
    }

    // different functio because we have another field to fill
    std::vector<data::TilesetInfo>
    add_tilesets(std::vector<lcf::rpg::Chipset> base_tileset_list,
                 std::vector<lcf::rpg::Chipset> modified_tileset_list) {
        std::vector<data::TilesetInfo> tilesets;

        for (size_t i = 0; i < std::min(base_tileset_list.size(), modified_tileset_list.size()); i++) {
            const auto &base_tileset = base_tileset_list[i];
            const auto &modified_tileset = modified_tileset_list[i];

            if (base_tileset == modified_tileset) {
                // no modification
                continue;
            }

            data::TilesetInfo changelog_tileset;
            changelog_tileset.data.ID = modified_tileset.ID;
            changelog_tileset.data.name = modified_tileset.name;
            changelog_tileset.data.chipset_name = modified_tileset.chipset_name;


            if (base_tileset.name.empty()) {
                // added
                changelog_tileset.status = data::Status::ADDED;
            } else if (modified_tileset.name.empty()) {
                // removed
                changelog_tileset.status = data::Status::REMOVED;
                changelog_tileset.data.name = base_tileset.name;
            } else {
                // modified
                changelog_tileset.status = data::Status::MODIFIED;
            }

            tilesets.push_back(changelog_tileset);
        }

        return tilesets;
    }

    /**
     * @brief Lists all events containing a transfer player event to another map in a map
     * @param map The map we want to analyze
     * @param map_id The ID of the other map; We don't want to list warps within the same map
     * @return A list of the Connection objects
     */
    std::vector<data::Connection> list_warp_events(std::unique_ptr<lcf::rpg::Map> map, int map_id) {
        std::vector<data::Connection> connections;
        data::Map map_data;
        map_data.data.ID = map_id;

        for (const auto &event: map->events) {
            for (const auto &page: event.pages) {
                for (const auto &command: page.event_commands) {
                    if (command.code != static_cast<int>(Commands::Teleport)) {
                        continue;
                    }
                    data::Connection connection;
                    connection.type = data::ConnectionType::ONEWAY;
                    connection.status = data::Status::ADDED;
                    connection.from_map = map_data;
                    connection.from_coordinates = data::Coordinates{event.x, event.y};

                    auto params = command.parameters.begin();
                    connection.to_map = data::Map{};
                    connection.to_map.data.ID = *params;
                    connection.to_coordinates = data::Coordinates{*std::next(params, 1), *std::next(params, 2)};

                    if (connection.from_map.data.ID == connection.to_map.data.ID) {
                        // same map
                        continue;
                    }
                    connections.push_back(connection);
                }
            }
        }

        return connections;
    }


    /**
     * @brief Scans two RPG Maker game directories for changes.
     * @param base_path The base path, usually the newest devbuild
     * @param modified_path The path of the build we made changes on
     * @return A changelog object containing the changes between the two builds
     */
    std::shared_ptr<data::Changelog>
    ChangelogGenerator::scan() {
        const std::string base_path = Mossball::origin_directory.toStdString();
        const std::string modified_path = Mossball::work_directory.toStdString();
        //log("Scanning changes between " + base_path + " and " + modified_path + "...");

        auto base_content = list_directory_content(Mossball::origin_directory.toStdString());
        if (base_content.empty()) {
            return nullptr;
        }

        auto modified_content = list_directory_content(Mossball::work_directory.toStdString());
        if (modified_content.empty()) {
            return nullptr;
        }

        auto changelog = std::make_shared<data::Changelog>();

        changelog->developer = Mossball::dev_name;

        changelog->date = QDate::currentDate();

        // TODO: get summary name
        changelog->summary = "...";

        // TODO: get policies
        changelog->map_policy = "...";
        changelog->asset_policy = "...";

        // maps
        std::vector<std::string> base_maps;
        std::remove_copy_if(base_content.begin(), base_content.end(), std::back_inserter(base_maps),
                            [](const std::string &s) {
                                return QFileInfo(QString::fromStdString(s)).completeSuffix() != "lmu";
                            });

        std::sort(base_maps.begin(), base_maps.end(), [](const std::string &a, const std::string &b) {
            return std::stoi(a.substr(3, a.find_first_of('.'))) < std::stoi(b.substr(3, b.find_first_of('.')));
        });

        std::vector<std::string> modified_maps;
        std::remove_copy_if(modified_content.begin(), modified_content.end(), std::back_inserter(modified_maps),
                            [](const std::string &s) {
                                return QFileInfo(QString::fromStdString(s)).completeSuffix() != "lmu";
                            });

        std::sort(modified_maps.begin(), modified_maps.end(), [](const std::string &a, const std::string &b) {
            return std::stoi(a.substr(3, a.find_first_of('.'))) < std::stoi(b.substr(3, b.find_first_of('.')));
        });

        // map tree
        fs::path base_lmt_path = base_path / fs::path("RPG_RT.lmt");
        if (!fs::exists(base_lmt_path)) {
            QMessageBox::critical(nullptr, "Error", "Missing file: " + QString::fromStdString(base_lmt_path.string()));
            return nullptr;
        }

        auto base_map_tree = lcf::LMT_Reader::Load(base_lmt_path.string());

        fs::path modified_lmt_path = modified_path / fs::path("RPG_RT.lmt");
        if (!fs::exists(modified_lmt_path)) {
            QMessageBox::critical(nullptr, "Error",
                                  QString::fromStdString("Missing file: " + modified_lmt_path.string()));
            return nullptr;
        }

        auto modified_map_tree = lcf::LMT_Reader::Load(modified_lmt_path.string());

        // maps scan
        for (const auto &map: modified_maps) {
            if (std::find(base_maps.begin(), base_maps.end(), map) == base_maps.end()) {
                // empty map file added
                continue;
            }

            const auto base_lastwritetime = fs::last_write_time(fs::path(base_path) / fs::path(map));
            const auto modified_lastwritetime = fs::last_write_time(fs::path(modified_path) / fs::path(map));

            if (base_lastwritetime == modified_lastwritetime) {
                // map file unchanged
                continue;
            }

            const int map_id = std::stoi(map.substr(3, map.find_first_of('.')));

            lcf::rpg::MapInfo base_map_info = base_map_tree->maps[map_id];
            lcf::rpg::MapInfo modified_map_info = modified_map_tree->maps[map_id];
            std::unique_ptr<lcf::rpg::Map> modified_lmu = lcf::LMU_Reader::Load((fs::path(modified_path) / fs::path(map)).string());
            std::unique_ptr<lcf::rpg::Map> modified_lmu_copy = std::make_unique<lcf::rpg::Map>(*modified_lmu);
            std::unique_ptr<lcf::rpg::Map> base_lmu = lcf::LMU_Reader::Load((fs::path(base_path) / fs::path(map)).string());

            // map info and contents unchanged
            if ((base_map_info == modified_map_info) && (*base_lmu == *modified_lmu)) {
                continue;
            }

            const std::string modified_map_name = modified_map_info.name.data();
            const std::string base_map_name = base_map_info.name.data();
            //if (modified_map_name.length() < 5) {
            //    // empty map (just the id in the name)
            //    continue;
            //}

            data::Map changelog_map;

            if (base_map_name != modified_map_name) {
                changelog_map.status = data::Status::ADDED;
            } else {
                changelog_map.status = data::Status::MODIFIED;
            }

            changelog_map.data.ID = map_id;
            changelog_map.data.name = modified_map_info.name;
            changelog_map.data.music = modified_map_info.music;



            changelog->maps.push_back(changelog_map);

            // connections
            auto base_warps = list_warp_events(std::move(base_lmu), map_id);
            auto modified_warps = list_warp_events(std::move(modified_lmu_copy), map_id);

            std::vector<data::Connection> warps;

            for (const auto &warp: modified_warps) {
                if (std::find(base_warps.begin(), base_warps.end(), warp) == base_warps.end()) {
                    // warp added
                    warps.push_back(warp);
                }
            }

            for (const auto &warp: base_warps) {
                if (std::find(modified_warps.begin(), modified_warps.end(), warp) == modified_warps.end()) {
                    // warp removed
                    auto removed_warp = warp;
                    removed_warp.status = data::Status::REMOVED;
                    warps.push_back(removed_warp);
                } else {
                    // warp modified
                    auto modified_warp = std::find(modified_warps.begin(), modified_warps.end(), warp);
                    if (warp != *modified_warp) {
                        auto modified_warp_ = warp;
                        modified_warp_.status = data::Status::MODIFIED;
                        warps.push_back(modified_warp_);
                    }
                }
            }

            for (const auto &warp: warps) {
                changelog->connections.push_back(warp);
            }
        }


        // database stuff
        auto base_db = lcf::LDB_Reader::Load((fs::path(base_path) / fs::path("RPG_RT.ldb")).string());
        auto modified_db = lcf::LDB_Reader::Load((fs::path(modified_path) / fs::path("RPG_RT.ldb")).string());

        changelog->common_events = add_db_entries<data::CommonEvent, lcf::rpg::CommonEvent>(
            base_db->commonevents, modified_db->commonevents);
        changelog->tilesets = add_tilesets(base_db->chipsets, modified_db->chipsets);
        changelog->switches = add_db_entries<data::Switch, lcf::rpg::Switch>(base_db->switches, modified_db->switches);
        changelog->variables = add_db_entries<data::Variable, lcf::rpg::Variable>(
            base_db->variables, modified_db->variables);
        changelog->animations = add_db_entries<data::Animation, lcf::rpg::Animation>(
            base_db->animations, modified_db->animations);
        changelog->troops = add_db_entries<data::Troop, lcf::rpg::Troop>(base_db->troops, modified_db->troops);
        changelog->terrains = add_db_entries<data::Terrain,
            lcf::rpg::Terrain>(base_db->terrains, modified_db->terrains);
        changelog->states = add_db_entries<data::State, lcf::rpg::State>(base_db->states, modified_db->states);
        changelog->skills = add_db_entries<data::Skill, lcf::rpg::Skill>(base_db->skills, modified_db->skills);
        changelog->items = add_db_entries<data::Item, lcf::rpg::Item>(base_db->items, modified_db->items);
        changelog->enemies = add_db_entries<data::Enemy, lcf::rpg::Enemy>(base_db->enemies, modified_db->enemies);
        changelog->elements = add_db_entries<data::Element, lcf::rpg::Attribute>(
            base_db->attributes, modified_db->attributes);
        changelog->classes = add_db_entries<data::Class, lcf::rpg::Class>(base_db->classes, modified_db->classes);
        changelog->actors = add_db_entries<data::Actor, lcf::rpg::Actor>(base_db->actors, modified_db->actors);

        // assets
        changelog->menu_themes = add_assets(data::AssetCategory::MENU_THEME);
        changelog->charsets = add_assets(data::AssetCategory::CHARSET);
        changelog->chipsets = add_assets(data::AssetCategory::CHIPSET);
        changelog->musics = add_assets(data::AssetCategory::MUSIC);
        changelog->sounds = add_assets(data::AssetCategory::SOUND);
        changelog->panoramas = add_assets(data::AssetCategory::PANORAMA);
        changelog->pictures = add_assets(data::AssetCategory::PICTURE);
        changelog->animation_files = add_assets(data::AssetCategory::BATTLE_ANIMATION);

        return changelog;
    }
}
