#include <QFile>

#include <lcf/rpg/database.h>

#pragma once

namespace lcfops {
inline QString id_with_name(int id, QString name) { return "[" + QString::number(id).rightJustified(4, char(48)) + (name.isEmpty() ? "]" : "] - ") + name; };

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
