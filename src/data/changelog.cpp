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

#include "changelog.h"

#include "reader_struct.h"

namespace data {
    std::string Coordinates::stringify() {
        return "(" + std::to_string(x) + "," + std::to_string(y) + ")";
    }

    std::string status_string(const Status &status) {
        switch (status) {
            case ADDED:
                return "+";
            case REMOVED:
                return "-";
            case MODIFIED:
                return "*";
        }

        return "";
    }

    std::string BGMEvent::stringify() {
        std::string s = "| BGM in event at " + coordinates.stringify();

        if (!track_name.empty() && volume != 0 && speed != 0) {
            s += " (track: " + track_name + ", volume: " + std::to_string(volume) + "%, speed: " +
                    std::to_string(speed) +
                    "%)";
        }

        return s;
    }

    std::string OpenConnection::stringify() {
        return status_string(status) + " Open connection at " + coordinates.stringify();
    }

    std::string ClosedConnection::stringify() {
        return status_string(status) + " Closed connection at " + coordinates.stringify();
    }

    std::string id_string(unsigned int id) {
        std::string s;

        if (id < 1000) {
            s += "0";
        }

        if (id < 100) {
            s += "0";
        }

        if (id < 10) {
            s += "0";
        }

        s += std::to_string(id);
        return s;
    }

    std::string Map::stringify() {
        std::string s = status_string(status) + " MAP[" + id_string(data.ID) + "] - " + ToString(data.name);

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        if (!bgm_events.empty()) {
            for (auto &bgm_event: bgm_events) {
                s += "\n\t" + bgm_event.stringify();
            }
        }

        if (!open_connections.empty()) {
            for (auto &open_connection: open_connections) {
                s += "\n\t" + open_connection.stringify();
            }
        }

        if (!closed_connections.empty()) {
            for (auto &closed_connection: closed_connections) {
                s += "\n\t" + closed_connection.stringify();
            }
        }

        return s;
    }

    std::string connection_type_string(ConnectionType connection_type) {
        switch (connection_type) {
            case ONEWAY:
                return "one-way";
            case BOTHWAY:
                return "both-way";
            case UNLOCKED:
                return "unlocked from the other side";
        }

        return "";
    }

    std::string Connection::stringify() {
        std::string s =
                status_string(status) + " Connection from MAP[" + id_string(from_map.data.ID) + "]." +
                from_coordinates.stringify() + " to MAP[" + id_string(to_map.data.ID) + "]." +
                to_coordinates.stringify() +
                "(" + connection_type_string(type) + ")";

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        return s;
    }

    std::string CommonEvent::stringify() {
        std::string s = status_string(status) + " CE[" + id_string(data.ID) + "] - " + ToString(data.name);

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        return s;
    }

    std::string TilesetInfo::stringify() {
        std::string s = status_string(status) + " Tileset[" + id_string(data.ID) + "] - " + ToString(data.name);

        if (!data.chipset_name.empty() && status == ADDED) {
            s += " (" + ToString(data.chipset_name) + ")";
        }

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        return s;
    }

    std::string Switch::stringify() {
        std::string s = status_string(status) + " S[" + id_string(data.ID) + "] - " + ToString(data.name);

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        return s;
    }

    std::string Variable::stringify() {
        std::string s = status_string(status) + " V[" + id_string(data.ID) + "] - " + ToString(data.name);

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        return s;
    }

    std::string Animation::stringify() {
        std::string s = status_string(status) + " Animation[" + id_string(data.ID) + "] - " + ToString(data.name);

        if (!data.animation_name.empty() && status == ADDED) {
            s += " (file: " + ToString(data.animation_name) + ")";
        }

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        return s;
    }

    std::string BattlerAnimation::stringify() {
        std::string s = status_string(status) + " BattlerAnimation[" + id_string(data.ID) + "] - " + ToString(data.name);

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        return s;
    }

    std::string Troop::stringify() {
        std::string s = status_string(status) + " Troop[" + id_string(data.ID) + "] - " + ToString(data.name);

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        return s;
    }

    std::string Terrain::stringify() {
        std::string s = status_string(status) + " Terrain[" + id_string(data.ID) + "] - " + ToString(data.name);

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        return s;
    }

    std::string State::stringify() {
        std::string s = status_string(status) + " State[" + id_string(data.ID) + "] - " + ToString(data.name);

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        return s;
    }

    std::string Skill::stringify() {
        std::string s = status_string(status) + " Skill[" + id_string(data.ID) + "] - " + ToString(data.name);

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        return s;
    }

    std::string Item::stringify() {
        std::string s = status_string(status) + " Item[" + id_string(data.ID) + "] - " + ToString(data.name);

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        return s;
    }

    std::string Enemy::stringify() {
        std::string s = status_string(status) + " Enemy[" + id_string(data.ID) + "] - " + ToString(data.name);

        if (!data.battler_name.empty() && status == ADDED) {
            s += " (battler: " + ToString(data.battler_name) + ")";
        }

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        return s;
    }

    std::string Element::stringify() {
        std::string s = status_string(status) + " Element[" + id_string(data.ID) + "] - " + ToString(data.name);

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        return s;
    }

    std::string Class::stringify() {
        std::string s = status_string(status) + " Class[" + id_string(data.ID) + "] - " + ToString(data.name);

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        return s;
    }

    std::string Actor::stringify() {
        std::string s = status_string(status) + " Actor[" + id_string(data.ID) + "] - " + ToString(data.name);

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        return s;
    }

    AssetCategory asset_folder_to_category(const QString &folder) {
        switch (folder) {
            case "System":
                return MENU_THEME;
            case "CharSet":
                return CHARSET;
            case "ChipSet":
                return CHIPSET;
            case "Music":
                return MUSIC;
            case "Sound":
                return SOUND;
            case "Panorama":
                return PANORAMA;
            case "Picture":
                return PICTURE;
            case "Battle":
                return BATTLE_ANIMATION;
        }
    }

    std::string asset_category_string(const AssetCategory &category) {
        switch (category) {
            case MENU_THEME:
                return "System";
            case CHARSET:
                return "CharSet";
            case CHIPSET:
                return "ChipSet";
            case MUSIC:
                return "Music";
            case SOUND:
                return "Sound";
            case PANORAMA:
                return "Panorama";
            case PICTURE:
                return "Picture";
            case BATTLE_ANIMATION:
                return "Battle";
            default:
                return "";
        }
    }

    std::string Asset::stringify() {
        std::string s = status_string(status);

        s += " " + asset_category_string(category) + " ";

        s += name;

        if (!contributors.empty()) {
            s += " (by " + contributors + ")";
        }

        if (!notes.empty()) {
            for (auto &note: notes) {
                s += "\n\t| " + note;
            }
        }

        return s;
    }

    std::string date_string(QDate date) {
        std::string s;

        int day = date.day();
        if (day < 10) {
            s += '0';
        }
        s += std::to_string(day) + "/";

        switch (date.month()) {
            case 1:
                s += "Jan";
                break;
            case 2:
                s += "Feb";
                break;
            case 3:
                s += "Mar";
                break;
            case 4:
                s += "Apr";
                break;
            case 5:
                s += "May";
                break;
            case 6:
                s += "Jun";
                break;
            case 7:
                s += "Jul";
                break;
            case 8:
                s += "Aug";
                break;
            case 9:
                s += "Sep";
                break;
            case 10:
                s += "Oct";
                break;
            case 11:
                s += "Nov";
                break;
            case 12:
                s += "Dec";
                break;
        }

        s += "/" + std::to_string(date.year());

        return s;
    }

    QString Changelog::stringify() {
        const QString big_separator = "|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|=|\n";
        const QString separator = "---------------------------------------------------\n";

        QString s = big_separator + "\nDeveloper: " + developer + "\nDate: " + date_string(date) + "\n\n";

        if (!map_policy.isEmpty() || !asset_policy.isEmpty()) {
            s += separator;
        }

        if (!map_policy.isEmpty()) {
            s += "Map policy: " + map_policy + "\n";
        }

        if (!asset_policy.isEmpty()) {
            s += "Asset policy: " + asset_policy + "\n";
        }

        if (!summary.isEmpty()) {
            s += "Comments: " + summary + "\n";
        }

        // Maps
        if (!maps.empty()) {
            s += separator;
        }

        for (auto &map: maps) {
            s += map.stringify() + "\n";
        }

        // Connections
        if (!connections.empty()) {
            s += separator;
        }

        for (auto &connection: connections) {
            s += connection.stringify() + "\n";
        }

        // Common events
        if (!common_events.empty()) {
            s += separator;
        }

        for (auto &common_event: common_events) {
            s += common_event.stringify() + "\n";
        }

        // Tilesets
        if (!tilesets.empty()) {
            s += separator;
        }

        for (auto &tileset: tilesets) {
            s += tileset.stringify() + "\n";
        }

        // Switches
        if (!switches.empty()) {
            s += separator;
        }

        for (auto &sw: switches) {
            s += sw.stringify() + "\n";
        }

        // Variables
        if (!variables.empty()) {
            s += separator;
        }

        for (auto &var: variables) {
            s += var.stringify() + "\n";
        }

        // Animations
        if (!animations.empty()) {
            s += separator;
        }

        for (auto &animation: animations) {
            s += animation.stringify() + "\n";
        }

        // Battler Animations
        if (!battler_animations.empty()) {
            s += separator;
        }

        for (auto &battler_animation: battler_animations) {
            s += battler_animation.stringify() + "\n";
        }

        // Troops
        if (!troops.empty()) {
            s += separator;
        }

        for (auto &troop: troops) {
            s += troop.stringify() + "\n";
        }

        // Terrains
        if (!terrains.empty()) {
            s += separator;
        }

        for (auto &terrain: terrains) {
            s += terrain.stringify() + "\n";
        }

        // States
        if (!states.empty()) {
            s += separator;
        }

        for (auto &state: states) {
            s += state.stringify() + "\n";
        }

        // Skills
        if (!skills.empty()) {
            s += separator;
        }

        for (auto &skill: skills) {
            s += skill.stringify() + "\n";
        }

        // Items
        if (!items.empty()) {
            s += separator;
        }

        for (auto &item: items) {
            s += item.stringify() + "\n";
        }

        // Enemies
        if (!enemies.empty()) {
            s += separator;
        }

        for (auto &enemy: enemies) {
            s += enemy.stringify() + "\n";
        }

        // Elements
        if (!elements.empty()) {
            s += separator;
        }

        for (auto &element: elements) {
            s += element.stringify() + "\n";
        }

        // Classes
        if (!classes.empty()) {
            s += separator;
        }

        for (auto &rpg_class: classes) {
            s += rpg_class.stringify() + "\n";
        }

        // Actors
        if (!actors.empty()) {
            s += separator;
        }

        for (auto &actor: actors) {
            s += actor.stringify() + "\n";
        }

        // Menu themes
        if (!menu_themes.empty()) {
            s += separator;
        }

        for (auto &menu_theme: menu_themes) {
            s += menu_theme.stringify() + "\n";
        }

        // Charsets
        if (!charsets.empty()) {
            s += separator;
        }

        for (auto &charset: charsets) {
            s += charset.stringify() + "\n";
        }

        // Chipsets
        if (!chipsets.empty()) {
            s += separator;
        }

        for (auto &chipset: chipsets) {
            s += chipset.stringify() + "\n";
        }

        // Musics
        if (!musics.empty()) {
            s += separator;
        }

        for (auto &music: musics) {
            s += music.stringify() + "\n";
        }

        // Sound effects
        if (!sounds.empty()) {
            s += separator;
        }

        for (auto &sound: sounds) {
            s += sound.stringify() + "\n";
        }

        // Panoramas
        if (!panoramas.empty()) {
            s += separator;
        }

        for (auto &panorama: panoramas) {
            s += panorama.stringify() + "\n";
        }

        // Pictures
        if (!pictures.empty()) {
            s += separator;
        }

        for (auto &picture: pictures) {
            s += picture.stringify() + "\n";
        }

        // Animation files
        if (!animation_files.empty()) {
            s += separator;
        }

        for (auto &animation: animation_files) {
            s += animation.stringify() + "\n";
        }

        s += "\n" + big_separator;

        return s;
    }
} // data
