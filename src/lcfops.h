#include <QFile>

#include <lcf/rpg/database.h>
#include <lcf/lmu/reader.h>
#include "../third_party/easyrpg_editor/dbstring.h"

#pragma once

namespace lcfops {
    inline QString paddedint(int number, int count) { return QString::number(number).rightJustified(count, char(48)); };
    inline QString id_with_name(int id, QString name) { return "[" + QString::number(id).rightJustified(4, char(48)) + (name.isEmpty() ? "]" : "] - ") + name; };
    inline QString bgmstring(lcf::rpg::Music bgm, lcf::rpg::Event *event = nullptr) {
        return QStringList{
            "BGM:",
            ToQString(bgm.name),
            QString("(%1%, %2%)").arg(bgm.volume).arg(bgm.tempo),
            (event ? QString("- event at (%1,%2)").arg(paddedint(event->x, 3)).arg(paddedint(event->y, 3)) : "")
        }.join(" ");
    };
    inline QString mapstring(int id, int x, int y, bool oneway = false) {
        return QStringList{
            "Connection to",
            QString("MAP[%1]").arg(paddedint(id, 4)),
            QString("at (%1, %2)").arg(paddedint(x, 3)).arg(paddedint(y, 3)),
            (oneway ? "(one-way)" : "")
        }.join(" ");
    };

    template <class T> QString compare(T orig, T work) {
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
