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

#include <string>
#include <vector>
#include <qt6/QtCore/qdatetime.h>
#include <qt6/QtCore/QString>

#include "lcf/rpg/actor.h"
#include "lcf/rpg/animation.h"
#include "lcf/rpg/attribute.h"
#include "lcf/rpg/battleranimation.h"
#include "lcf/rpg/chipset.h"
#include "lcf/rpg/class.h"
#include "lcf/rpg/commonevent.h"
#include "lcf/rpg/enemy.h"
#include "lcf/rpg/item.h"
#include "lcf/rpg/mapinfo.h"
#include "lcf/rpg/skill.h"
#include "lcf/rpg/state.h"
#include "lcf/rpg/switch.h"
#include "lcf/rpg/terrain.h"
#include "lcf/rpg/troop.h"
#include "lcf/rpg/variable.h"


namespace data {
    /**
     * @brief Data structure used to represent wether an entry was added, removed or modified.
     */
    enum Status {
        ADDED,
        REMOVED,
        MODIFIED
    };

    std::string status_string(const Status &status);

    /**
     * @brief Wrapper for coordinate.
     */
    struct Coordinates {
        int x;
        int y;

        std::string stringify();
    };

    inline bool operator==(const Coordinates &lhs, const Coordinates &rhs) {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    inline bool operator!=(const Coordinates &lhs, const Coordinates &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent a BGM event.
     */
    struct BGMEvent {
        Coordinates coordinates;

        std::string track_name;
        int volume;
        int speed;

        std::string stringify();
    };

    inline bool operator==(const BGMEvent &lhs, const BGMEvent &rhs) {
        return lhs.coordinates == rhs.coordinates && lhs.track_name == rhs.track_name &&
               lhs.volume == rhs.volume && lhs.speed == rhs.speed;
    }

    inline bool operator!=(const BGMEvent &lhs, const BGMEvent &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent an open connection.
     */
    struct OpenConnection {
        Status status;
        Coordinates coordinates;

        std::string stringify();
    };

    inline bool operator==(const OpenConnection &lhs, const OpenConnection &rhs) {
        return lhs.coordinates == rhs.coordinates;
    }

    inline bool operator!=(const OpenConnection &lhs, const OpenConnection &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent a closed connection.
     */
    struct ClosedConnection {
        Status status;
        Coordinates coordinates;

        std::string stringify();
    };

    inline bool operator==(const ClosedConnection &lhs, const ClosedConnection &rhs) {
        return lhs.coordinates == rhs.coordinates;
    }

    inline bool operator!=(const ClosedConnection &lhs, const ClosedConnection &rhs) {
        return !(lhs == rhs);
    }

    /**
     * Converts an int in a string of a 4 digit number
     * @param id The number to turn into a string
     * @returns The 4 digit stringized version of the number
     */
    std::string id_string(unsigned int id);

    /**
     * @brief Data structure used to represent a map.
     */
    struct Map {
        Status status;
        lcf::rpg::MapInfo data;

        std::vector<std::string> notes;
        std::vector<BGMEvent> bgm_events;
        std::vector<OpenConnection> open_connections;
        std::vector<ClosedConnection> closed_connections;

        std::string stringify();
    };

    inline bool operator==(const Map &lhs, const Map &rhs) {
        return lhs.data == rhs.data && lhs.bgm_events == rhs.bgm_events &&
               lhs.open_connections == rhs.open_connections && lhs.closed_connections == rhs.closed_connections;
    }

    inline bool operator!=(const Map &lhs, const Map &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent a connection type.
     * @details ONEWAY: The connection is one-way. BOTHWAY: The connection is two-way. UNLOCKED: The player must unlock the connection from one side.
     */
    enum ConnectionType {
        ONEWAY,
        BOTHWAY,
        UNLOCKED
    };

    /**
     * @brief Data structure used to represent a connection between two maps.
     */
    struct Connection {
        Status status;

        Map from_map;
        Coordinates from_coordinates;

        Map to_map;
        Coordinates to_coordinates;

        ConnectionType type;

        std::vector<std::string> notes;

        std::string stringify();
    };

    inline bool operator==(const Connection &lhs, const Connection &rhs) {
        return lhs.from_map == rhs.from_map && lhs.from_coordinates == rhs.from_coordinates &&
               lhs.to_map == rhs.to_map && lhs.to_coordinates == rhs.to_coordinates && lhs.type == rhs.type;
    }

    inline bool operator!=(const Connection &lhs, const Connection &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent a common event.
     */
    struct CommonEvent {
        Status status;
        lcf::rpg::CommonEvent data;
        std::vector<std::string> notes;

        std::string stringify();
    };

    inline bool operator==(const CommonEvent &lhs, const CommonEvent &rhs) {
        return lhs.data == rhs.data;
    }

    inline bool operator!=(const CommonEvent &lhs, const CommonEvent &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent a tileset entry in the database.
     */
    struct TilesetInfo {
        Status status;
        lcf::rpg::Chipset data;
        std::vector<std::string> notes;

        std::string stringify();
    };

    inline bool operator==(const TilesetInfo &lhs, const TilesetInfo &rhs) {
        return lhs.data == rhs.data;
    }

    inline bool operator!=(const TilesetInfo &lhs, const TilesetInfo &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent a switch.
     */
    struct Switch {
        Status status;
        lcf::rpg::Switch data;
        std::vector<std::string> notes;

        std::string stringify();
    };

    inline bool operator==(const Switch &lhs, const Switch &rhs) {
        return lhs.data == rhs.data;
    }

    inline bool operator!=(const Switch &lhs, const Switch &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent a variable.
     */
    struct Variable {
        Status status;
        lcf::rpg::Variable data;
        std::vector<std::string> notes;

        std::string stringify();
    };

    inline bool operator==(const Variable &lhs, const Variable &rhs) {
        return lhs.data == rhs.data;
    }

    inline bool operator!=(const Variable &lhs, const Variable &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent an animation entry in the database.
     */
    struct Animation {
        Status status;
        lcf::rpg::Animation data;
        std::vector<std::string> notes;

        std::string stringify();
    };

    inline bool operator==(const Animation &lhs, const Animation &rhs) {
        return lhs.data == rhs.data;
    }

    inline bool operator!=(const Animation &lhs, const Animation &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent a battler animation entry in the database.
     */
    struct BattlerAnimation {
        Status status;
        lcf::rpg::BattlerAnimation data;
        std::vector<std::string> notes;

        std::string stringify();
    };

    inline bool operator==(const BattlerAnimation &lhs, const BattlerAnimation &rhs) {
        return lhs.data == rhs.data;
    }

    inline bool operator!=(const BattlerAnimation &lhs, const BattlerAnimation &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent a troop entry in the database.
     */
    struct Troop {
        Status status;
        lcf::rpg::Troop data;
        std::vector<std::string> notes;

        std::string stringify();
    };

    inline bool operator==(const Troop &lhs, const Troop &rhs) {
        return lhs.data == rhs.data;
    }

    inline bool operator!=(const Troop &lhs, const Troop &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent a terrain entry in the database.
     */
    struct Terrain {
        Status status;
        lcf::rpg::Terrain data;
        std::vector<std::string> notes;

        std::string stringify();
    };

    inline bool operator==(const Terrain &lhs, const Terrain &rhs) {
        return lhs.data == rhs.data;
    }

    inline bool operator!=(const Terrain &lhs, const Terrain &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent a state entry in the database.
     */
    struct State {
        Status status;
        lcf::rpg::State data;
        std::vector<std::string> notes;

        std::string stringify();
    };

    inline bool operator==(const State &lhs, const State &rhs) {
        return lhs.data == rhs.data;
    }

    inline bool operator!=(const State &lhs, const State &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent a skill entry in the database.
     */
    struct Skill {
        Status status;
        lcf::rpg::Skill data;
        std::vector<std::string> notes;

        std::string stringify();
    };

    inline bool operator==(const Skill &lhs, const Skill &rhs) {
        return lhs.data == rhs.data;
    }

    inline bool operator!=(const Skill &lhs, const Skill &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent an item entry in the database.
     */
    struct Item {
        Status status;
        lcf::rpg::Item data;
        std::vector<std::string> notes;

        std::string stringify();
    };

    inline bool operator==(const Item &lhs, const Item &rhs) {
        return lhs.data == rhs.data;
    }

    inline bool operator!=(const Item &lhs, const Item &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent an enemy entry in the database.
     */
    struct Enemy {
        Status status;
        lcf::rpg::Enemy data;
        std::vector<std::string> notes;

        std::string stringify();
    };

    inline bool operator==(const Enemy &lhs, const Enemy &rhs) {
        return lhs.data == rhs.data;
    }

    inline bool operator!=(const Enemy &lhs, const Enemy &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent an element entry in the database.
     */
    struct Element {
        Status status;
        lcf::rpg::Attribute data;
        std::vector<std::string> notes;

        std::string stringify();
    };

    inline bool operator==(const Element &lhs, const Element &rhs) {
        return lhs.data == rhs.data;
    }

    inline bool operator!=(const Element &lhs, const Element &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent a class entry in the database.
     */
    struct Class {
        Status status;
        lcf::rpg::Class data;
        std::vector<std::string> notes;

        std::string stringify();
    };

    inline bool operator==(const Class &lhs, const Class &rhs) {
        return lhs.data == rhs.data;
    }

    inline bool operator!=(const Class &lhs, const Class &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent an actor entry in the database.
     */
    struct Actor {
        Status status;
        lcf::rpg::Actor data;
        std::vector<std::string> notes;

        std::string stringify();
    };

    inline bool operator==(const Actor &lhs, const Actor &rhs) {
        return lhs.data == rhs.data;
    }

    inline bool operator!=(const Actor &lhs, const Actor &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Data structure used to represent an asset category.
     */
    enum AssetCategory {
        MENU_THEME,
        CHARSET,
        CHIPSET,
        MUSIC,
        SOUND,
        PANORAMA,
        PICTURE,
        BATTLE_ANIMATION
    };

    /**
     *
     * @param folder A folder in the game files
     * @return the asset category corresponding to the contents of the folder
     */
    AssetCategory asset_folder_to_category(const QString &folder);

    /**
     * @param category An asset caterogy
     * @return the stringified changelog format asset category
     */
    std::string asset_category_string(const AssetCategory &category);

    /**
     * @brief Data structure used to represent an asset.
     */
    struct Asset {
        Status status;
        AssetCategory category;
        std::string name;
        std::string filename;
        std::vector<std::string> notes;

        // outside contributors only
        std::string contributors;

        std::string stringify();
    };

    inline bool operator==(const Asset &lhs, const Asset &rhs) {
        return lhs.category == rhs.category && lhs.name == rhs.name;
    }

    inline bool operator!=(const Asset &lhs, const Asset &rhs) {
        return !(lhs == rhs);
    }

    /**
     * @brief Converts a date to a string according to the format specified in the changelog template.
     * @param date
     * @return A string representation of the date.
     */
    std::string date_string(QDate date);

    /**
     * @brief Data structure used to generate the changelog.
     */
    struct Changelog {
        /**
         * @brief Name of the developer
         */
        QString developer;
        /**
         * @brief Date of submit
         */
        QDate date = QDate::currentDate();

        /**
         * @brief Summary of the submit
         */
        QString summary;

        /**
         * @brief Map policy specifications (who is allowed to edit and/or add content to your maps, and in what ways: open connections only, can create a warp but it has to be this specific shape/color, etc.)
         */
        QString map_policy;
        /**
         * @brief Asset policy specifications (who is allowed to use and/or edit the assets you made)
         */
        QString asset_policy;

        /**
         * @brief List of maps
         */
        std::vector<Map> maps;
        /**
         * @brief List of connections between worlds (a world can consist of multiple maps. You don't have to write down warps between a map and its subareas)
         */
        std::vector<Connection> connections;

        /**
         * @brief List of common events
         */
        std::vector<CommonEvent> common_events;

        /**
         * @brief List of tileset entries in the database
         */
        std::vector<TilesetInfo> tilesets;

        /**
         * @brief List of switches
         */
        std::vector<Switch> switches;

        /**
         * @brief List of variables
         */
        std::vector<Variable> variables;

        /**
         * @brief List of animation entries in the database
         */
        std::vector<Animation> animations;

        /**
         * @brief List of battler_animation entries in the database
         */
        std::vector<BattlerAnimation> battler_animations;

        /**
         * @brief List of troop entries in the database
         */
        std::vector<Troop> troops;

        /**
         * @brief List of terrain entries in the database
         */
        std::vector<Terrain> terrains;

        /**
         * @brief List of state entries in the database
         */
        std::vector<State> states;

        /**
         * @brief List of skill entries in the database
         */
        std::vector<Skill> skills;

        /**
         * @brief List of item entries in the database
         */
        std::vector<Item> items;

        /**
         * @brief List of enemy entries in the database
         */
        std::vector<Enemy> enemies;

        /**
         * @brief List of element entries in the database
         */
        std::vector<Element> elements;

        /**
         * @brief List of class entries in the database
         */
        std::vector<Class> classes;

        /**
         * @brief List of actor entries in the database
         */
        std::vector<Actor> actors;

        /**
         * @brief List of menu theme files
         */
        std::vector<Asset> menu_themes;

        /**
         * @brief List of charsets
         */
        std::vector<Asset> charsets;

        /**
         * @brief List of chipsets
         */
        std::vector<Asset> chipsets;

        /**
         * @brief List of music files
         */
        std::vector<Asset> musics;

        /**
         * @brief List of sound effects
         */
        std::vector<Asset> sounds;

        /**
         * @brief List of panoramas
         */
        std::vector<Asset> panoramas;

        /**
         * @brief List of pictures
         */
        std::vector<Asset> pictures;

        /**
         * @brief List of animation files
         */
        std::vector<Asset> animation_files;

        QString stringify();

        Changelog() = default;
    };

    inline bool operator==(const Changelog &lhs, const Changelog &rhs) {
        return lhs.developer == rhs.developer && lhs.summary == rhs.summary && lhs.map_policy == rhs.map_policy &&
               lhs.asset_policy == rhs.asset_policy && lhs.maps == rhs.maps &&
               lhs.connections == rhs.connections && lhs.common_events == rhs.common_events &&
               lhs.tilesets == rhs.tilesets && lhs.switches == rhs.switches && lhs.variables == rhs.variables &&
               lhs.animations == rhs.animations && lhs.battler_animations == rhs.battler_animations && lhs.troops == rhs
               .troops && lhs.terrains == rhs.terrains && lhs.states == rhs.states && lhs.skills == rhs.skills && lhs.
               items == rhs.items && lhs.enemies == rhs.enemies && lhs.elements == rhs.elements && lhs.classes == rhs.
               classes && lhs.actors == rhs.actors && lhs.menu_themes == rhs.menu_themes &&
               lhs.charsets == rhs.charsets && lhs.chipsets == rhs.chipsets && lhs.musics == rhs.musics &&
               lhs.sounds == rhs.sounds && lhs.panoramas == rhs.panoramas && lhs.pictures == rhs.pictures &&
               lhs.animation_files == rhs.animation_files;
    }

    inline bool operator!=(const Changelog &lhs, const Changelog &rhs) {
        return !(lhs == rhs);
    }
}
