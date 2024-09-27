#include <QFile>

#include <lcf/rpg/database.h>
#include <lcf/lmu/reader.h>
#include <qregion.h>
#include "../third_party/easyrpg_editor/dbstring.h"

#pragma once

namespace lcfops {
    struct connection_info {
        connection_info(int dest, int x, int y, int id, QString path) : dest(dest), xy(x, y, 1, 1), id(id), path(std::move(path)) {}
        connection_info(int dest, QRect xy, int id, QString path) : dest(dest), xy(xy), id(id), path(std::move(path)) {}
        int dest;
        QRect xy;
        int id;
        QString path;
    };

    inline QString paddedint(int number, int count) { return QString::number(number).rightJustified(count, char(48)); };
    inline QString id_with_name(int id, QString name) { return "[" + QString::number(id).rightJustified(4, char(48)) + (name.isEmpty() ? "]" : "] - ") + name; };
    inline QString bgmstring(lcf::rpg::Music bgm, lcf::rpg::Event *event = nullptr) {
        return QStringList{
            "BGM:",
            ToQString(bgm.name),
            QString("(%1%, %2%)").arg(bgm.volume).arg(bgm.tempo),
            (event ? QString("- event at (%1,%2)").arg(paddedint(event->x, 3)).arg(paddedint(event->y, 3)) : "- map setting")
        }.join(" ");
    };
    inline QString mapstring(int id, QRect xy, bool oneway = false) {
        return QStringList{
            "Connection to",
            QString("MAP[%1]").arg(paddedint(id, 4)),
            QString("at (%1, %2)")
                .arg((xy.width() <= 1 ? paddedint(xy.x(), 3) : QString("%1-%2").arg(paddedint(xy.x(), 3)).arg(paddedint(xy.right(), 3))))
                .arg((xy.height() <= 1 ? paddedint(xy.y(), 3) : QString("%1-%2").arg(paddedint(xy.y(), 3)).arg(paddedint(xy.bottom(), 3)))),
            (oneway ? "(one-way)" : "")
        }.join(" ");
    };

    template <class T> QString compare(T orig, T work) {
        // if i had a nickel for every time liblcf object defaults not being the same as rm2k3 defaults tripped up what would otherwise be a simple comparison i'd have two nickels. which isn't a lot but maybe i should make a PR about this
        T empty;
        if (orig == empty && work != empty) {
            return "+";
        } else if (orig != empty && work == empty) {
            return "-";
        } else {
            return "*";
        }
    };
}
