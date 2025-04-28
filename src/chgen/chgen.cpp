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

#include <QCryptographicHash>
#include <QDirIterator>

#include "../mossball.h"
#include "lcf/ldb/chunks.h"
#include "lcf/ldb/reader.h"

using Commands = lcf::rpg::EventCommand::Code;

namespace chgen {

    /**
     * @brief Lists the content of a directory
     * @param path the path of the directory
     * @return a vector containing the names of the files in the directory
     */
    QStringList list_directory_content(QString path) {
        QStringList content;

        try {
            if (!fs::exists(path.toStdString())) {
                error("The specified path does not exist.");
                return content;
            }

            if (!fs::is_directory(path.toStdString())) {
                error("The specified path is not a directory.");
                return content;
            }

            // list only child files
            QDirIterator file_it(path, QDirIterator::NoIteratorFlags);
            while (file_it.hasNext()) {
                const QString file = file_it.next();
                content.push_back(file);
            }

        } catch (const std::exception &e) {
            error(std::string(e.what()));
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

        auto base_asset_content = list_directory_content(std::string(base_path / fs::path(folder)));
        auto modified_asset_content = list_directory_content(std::string(modified_path / fs::path(folder)));

        for (const auto &asset: base_asset_content) {
            const auto asset_name = asset.substr(0, asset.find_first_of('.'));
            /*if (asset_name.rfind("record_player_", 0) == 0) {
                // ignore record player assets
                continue;
            }*/

            if (std::find(begin(modified_asset_content), end(modified_asset_content), asset) ==
                end(modified_asset_content)) {
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

            if (std::find(begin(base_asset_content), end(base_asset_content), asset) ==
                end(base_asset_content)) {
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
                    error(std::string(e.what()));
                }
            }
        }

        return assets;
    }

    std::vector<data::CommonEvent>
    add_ce(std::vector<lcf::rpg::CommonEvent> base_ce_list, std::vector<lcf::rpg::CommonEvent> modified_ce_list) {
        std::vector<data::CommonEvent> commonEvents;

        for (size_t i = 0; i < std::min(base_ce_list.size(), modified_ce_list.size()); i++) {
            const auto &base_ce = base_ce_list[i];
            const auto &modified_ce = modified_ce_list[i];

            if (base_ce == modified_ce) {
                // no modification
                continue;
            }

            data::CommonEvent changelog_ce;
            changelog_ce.data.ID = modified_ce.ID;
            changelog_ce.data.name = modified_ce.name;


            if (base_ce.name.empty()) {
                // added
                changelog_ce.status = data::Status::ADDED;
            } else if (modified_ce.name.empty()) {
                // removed
                changelog_ce.status = data::Status::REMOVED;
                changelog_ce.data.name = base_ce.name;
            } else {
                // modified
                changelog_ce.status = data::Status::MODIFIED;
            }

            commonEvents.push_back(changelog_ce);
        }

        return commonEvents;
    }

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

    std::vector<data::Switch>
    add_switches(std::vector<lcf::rpg::Switch> base_switch_list, std::vector<lcf::rpg::Switch> modified_variable_list) {
        std::vector<data::Switch> switches;

        for (size_t i = 0; i < std::min(base_switch_list.size(), modified_variable_list.size()); i++) {
            const auto &base_switch = base_switch_list[i];
            const auto &modified_switch = modified_variable_list[i];

            if (base_switch == modified_switch) {
                // no modification
                continue;
            }

            data::Switch changelog_switch;
            changelog_switch.data.ID = modified_switch.ID;
            changelog_switch.data.name = modified_switch.name;

            if (base_switch.name.empty()) {
                // added
                changelog_switch.status = data::Status::ADDED;
            } else if (modified_switch.name.empty()) {
                // removed
                changelog_switch.status = data::Status::REMOVED;
                changelog_switch.data.name = base_switch.name;
            } else {
                // modified
                changelog_switch.status = data::Status::MODIFIED;
            }

            switches.push_back(changelog_switch);
        }

        return switches;
    }

    std::vector<data::Variable>
    add_variables(std::vector<lcf::rpg::Variable> base_var_list, std::vector<lcf::rpg::Variable> modified_var_list) {
        std::vector<data::Variable> variables;

        for (size_t i = 0; i < std::min(base_var_list.size(), modified_var_list.size()); i++) {
            const auto &base_var = base_var_list[i];
            const auto &modified_var = modified_var_list[i];

            if (base_var == modified_var) {
                // no modification
                continue;
            }

            data::Variable changelog_var;
            changelog_var.data.ID = modified_var.ID;
            changelog_var.data.name = modified_var.name;


            if (base_var.name.empty()) {
                // added
                changelog_var.status = data::Status::ADDED;
            } else if (modified_var.name.empty()) {
                // removed
                changelog_var.status = data::Status::REMOVED;
                changelog_var.data.name = base_var.name;
            } else {
                // modified
                changelog_var.status = data::Status::MODIFIED;
            }

            variables.push_back(changelog_var);
        }

        return variables;
    }

    std::vector<data::Animation>
    add_animations(std::vector<lcf::rpg::Animation> base_anim_list,
                   std::vector<lcf::rpg::Animation> modified_anim_list) {
        std::vector<data::Animation> animations;

        for (size_t i = 0; i < std::min(base_anim_list.size(), modified_anim_list.size()); i++) {
            const auto &base_anim = base_anim_list[i];
            const auto &modified_anim = modified_anim_list[i];

            if (base_anim == modified_anim) {
                // no modification
                continue;
            }

            data::Animation changelog_anim;
            changelog_anim.data.ID = modified_anim.ID;
            changelog_anim.data.name = modified_anim.name;

            if (base_anim.name.empty()) {
                // added
                changelog_anim.status = data::Status::ADDED;
            } else if (modified_anim.name.empty()) {
                // removed
                changelog_anim.status = data::Status::REMOVED;
                changelog_anim.data.name = base_anim.name;
            } else {
                // modified
                changelog_anim.status = data::Status::MODIFIED;
            }

            animations.push_back(changelog_anim);
        }

        return animations;
    }

    /**
     * @brief Lists all events containing a transfer player event to another map  in a map
     * @param map The map we want to analyze
     * @param map_id The ID of the map in argument map; We don't want to list inter-map warps
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
     * @brief Scans two RPG Maker game files for changes that are relevant in Collective Unconscious.
     * @param base_path The base path, usually the newest devbuild
     * @param modified_path The path of the build we made changes on
     * @return A changelog object containing the changes between the two builds
     */
    std::shared_ptr<data::Changelog>
    ChangelogGenerator::scan() {
        const std::string base_path = Mossball::origin_directory.toStdString();
        const std::string modified_path = Mossball::work_directory.toStdString();
        log("Scanning changes between " + base_path + " and " + modified_path + "...");

        auto base_content = list_directory_content(base_path);
        if (base_content.empty()) {
            return nullptr;
        }

        auto modified_content = list_directory_content(modified_path);
        if (modified_content.empty()) {
            return nullptr;
        }

        auto changelog = std::make_shared<data::Changelog>();

        changelog->developer = Mossball::dev_name;

        changelog->date = QDate::currentDate();

        // TODO: get summary name
        changelog->summary = "";

        // TODO: get policies
        changelog->map_policy = "";
        changelog->asset_policy = "";

        // maps
        std::vector<std::string> base_maps;
        std::remove_copy_if(begin(base_content), end(base_content), std::back_inserter(base_maps),
                            [](const std::string &s) {
                                return s.substr(s.find_last_of('.') + 1) != "lmu";
                            });

        std::sort(begin(base_maps), end(base_maps), [](const std::string &a, const std::string &b) {
            return std::stoi(a.substr(3, a.find_first_of('.'))) < std::stoi(b.substr(3, b.find_first_of('.')));
        });

        std::vector<std::string> modified_maps;
        std::remove_copy_if(begin(modified_content), end(modified_content), std::back_inserter(modified_maps),
                            [](const std::string &s) {
                                return s.substr(s.find_last_of('.') + 1) != "lmu";
                            });

        std::sort(begin(modified_maps), end(modified_maps), [](const std::string &a, const std::string &b) {
            return std::stoi(a.substr(3, a.find_first_of('.'))) < std::stoi(b.substr(3, b.find_first_of('.')));
        });

        // map tree
        fs::path base_lmt_path = base_path / fs::path("RPG_RT.lmt");
        if (!fs::exists(base_lmt_path)) {
            error("Missing file: " + std::string(base_lmt_path));
            return nullptr;
        }

        auto base_map_tree = lcf::LMT_Reader::Load(std::string(base_lmt_path));

        fs::path modified_lmt_path = modified_path / fs::path("RPG_RT.lmt");
        if (!fs::exists(modified_lmt_path)) {
            error("Missing file: " + std::string(modified_lmt_path));
            return nullptr;
        }

        auto modified_map_tree = lcf::LMT_Reader::Load(std::string(modified_lmt_path));

        // maps scan
        for (const auto &map: modified_maps) {

            if (std::find(begin(base_maps), end(base_maps), map) == end(base_maps)) {
                // empty map file added
                continue;
            }

            const auto base_lastwritetime = fs::last_write_time(base_path / fs::path(map));
            const auto modified_lastwritetime = fs::last_write_time(modified_path / fs::path(map));

            if (base_lastwritetime == modified_lastwritetime) {
                // map file unchanged
                continue;
            }

            const int map_id = std::stoi(map.substr(3, map.find_first_of('.')));

            if (map_id == 7) {
                // ignore record player
                continue;
            }

            auto base_map = base_map_tree->maps[map_id];
            auto modified_map = modified_map_tree->maps[map_id];

            if (base_map == modified_map) {
                // map unchanged
                continue;
            }

            const std::string modified_map_name = modified_map.name.data();
            const std::string base_map_name = base_map.name.data();
            if (modified_map_name.length() < 5) {
                // empty map (just the id in the name)
                continue;
            }

            data::Map changelog_map;

            if (base_map_name != modified_map_name) {
                changelog_map.status = data::Status::ADDED;
            } else {
                changelog_map.status = data::Status::MODIFIED;
            }

            changelog_map.data.ID = map_id;
            changelog_map.data.name = modified_map.name;
            changelog_map.data.music = modified_map.music;

            auto modified_lmu = lcf::LMU_Reader::Load(std::string(modified_path / fs::path(map)));
            auto modified_lmu_copy = std::make_unique<lcf::rpg::Map>(*modified_lmu);

            auto base_lmu = lcf::LMU_Reader::Load(std::string(base_path / fs::path(map)));

            if (map_id != 7) {
                // ignore bgm events for record player
                changelog_map.bgm_events = list_bgm_events(std::move(modified_lmu), modified_map.music.name);
            }

            changelog->maps.push_back(changelog_map);

            // connections
            // TODO: put a warning to tell the user that all connections to a different map ID will be noted
            auto base_warps = list_warp_events(std::move(base_lmu), map_id);
            auto modified_warps = list_warp_events(std::move(modified_lmu_copy), map_id);

            std::vector<data::Connection> warps;

            for (const auto &warp: modified_warps) {
                if (std::find(begin(base_warps), end(base_warps), warp) == end(base_warps)) {
                    // warp added
                    warps.push_back(warp);
                }
            }

            for (const auto &warp: base_warps) {
                if (std::find(begin(modified_warps), end(modified_warps), warp) == end(modified_warps)) {
                    // warp removed
                    auto removed_warp = warp;
                    removed_warp.status = data::Status::REMOVED;
                    warps.push_back(removed_warp);
                } else {
                    // warp modified
                    auto modified_warp = std::find(begin(modified_warps), end(modified_warps), warp);
                    if (warp != *modified_warp) {
                        auto modified_warp = warp;
                        modified_warp.status = data::Status::MODIFIED;
                        warps.push_back(modified_warp);
                    }
                }
            }

            for (const auto &warp: warps) {
                changelog->connections.push_back(warp);
            }
        }


        // database stuff
        auto base_db = lcf::LDB_Reader::Load(std::string(base_path / fs::path("RPG_RT.ldb")));
        auto modified_db = lcf::LDB_Reader::Load(std::string(modified_path / fs::path("RPG_RT.ldb")));

        changelog->common_events = add_ce(base_db->commonevents, modified_db->commonevents);
        changelog->tilesets = add_tilesets(base_db->chipsets, modified_db->chipsets);
        changelog->switches = add_switches(base_db->switches, modified_db->switches);
        changelog->variables = add_variables(base_db->variables, modified_db->variables);
        changelog->animations = add_animations(base_db->animations, modified_db->animations);
        // TODO for missing database stuff (troops, etc)

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

    /**
     * @brief Generates a plain text changelog file from a Changelog object. The name of the file will be <developer name>_<date>_changelog.txt
     * @param changelog The changelog we want to save in a file
     * @param at The path where we want to save the file. If empty, the file will be saved in the current directory.
     */
    void ChangelogGenerator::generate_changelog_file(const std::shared_ptr<data::Changelog> &changelog, const std::string& at) {
        std::string date = data::date_string(changelog->date);
        std::string date_formatted = date.substr(0, 2) + date.substr(3, 3) + date.substr(7, date.length());

        std::string filename = at / fs::path(changelog->developer + "_" + date_formatted + "_changelog.txt");

        // this ensures that we don't overwrite any existing file
        for (int i = 2; fs::exists(filename); i++) {
            filename = at / fs::path(changelog->developer.toStdString() + "_" + date_formatted + "_changelog_" +
                       std::to_string(i) + ".txt");
        }

        std::ofstream file;
        file.open(filename);

        file << changelog->stringify() << '\n';

        if (!fs::exists(filename)) {
            error("Could not generate file " + filename);
        } else {
            log("File generated: " + filename);
        }

        file.close();
    }

