#include <QFile>

#include <lcf/rpg/database.h>
#include <lcf/lmu/reader.h>
#include "../third_party/easyrpg_editor/dbstring.h"

#pragma once

namespace lcfops {
    inline QString id_with_name(int id, QString name) { return "[" + QString::number(id).rightJustified(4, char(48)) + (name.isEmpty() ? "]" : "] - ") + name; };
    inline QString bgmstring(lcf::rpg::Music bgm, lcf::rpg::Event *event = nullptr) {
        return QStringList{
            "BGM:",
            ToQString(bgm.name),
            (event ? QString("in event at (%1,%2)").arg(QString::number(event->x).rightJustified(3, char(48))).arg(QString::number(event->y).rightJustified(3, char(48))) : "")
        }.join(""); };

    template <class T> QString compare(T orig, T work) {
        T empty;
        if (orig == empty && work != empty) {
            return "+";
        } else if (orig != empty && work == empty) {
            return "-";
        } else {
            return "*";
        }
    }
}
