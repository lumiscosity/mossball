#include <QFile>

#include <lcf/rpg/database.h>
#include <lcf/lmu/reader.h>
#include <qregion.h>
#include "../third_party/easyrpg_editor/dbstring.h"

#pragma once

namespace lcfops {
    struct connection_info {
        connection_info(int dest, int dest_x, int dest_y, int x, int y, int id, QString path) : dest(dest), dest_xy(dest_x, dest_y, 1, 1), xy(x, y, 1, 1), id(id), path(std::move(path)) {}
        connection_info(int dest, QRect dest_xy, QRect xy, int id, QString path) : dest(dest), dest_xy(dest_xy), xy(xy), id(id), path(std::move(path)) {}
        int dest;
        QRect dest_xy;
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
    inline QString mapstring(int id, QRect dest_xy, QRect xy, bool oneway = false) {
        return QStringList{
            "Connection from",
            QString("(%1, %2) to (%3, %4)")
                .arg((xy.width() <= 1 ? paddedint(xy.x(), 3) : QString("%1-%2").arg(paddedint(xy.x(), 3)).arg(paddedint(xy.right(), 3))))
                .arg((xy.height() <= 1 ? paddedint(xy.y(), 3) : QString("%1-%2").arg(paddedint(xy.y(), 3)).arg(paddedint(xy.bottom(), 3))))
                .arg((dest_xy.width() <= 1 ? paddedint(dest_xy.x(), 3) : QString("%1-%2").arg(paddedint(dest_xy.x(), 3)).arg(paddedint(dest_xy.right(), 3))))
                .arg((dest_xy.height() <= 1 ? paddedint(dest_xy.y(), 3) : QString("%1-%2").arg(paddedint(dest_xy.y(), 3)).arg(paddedint(dest_xy.bottom(), 3)))),
            QString("in MAP[%1]").arg(paddedint(id, 4)),
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
