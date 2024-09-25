#include "lcfops.h"

QString lcfops::compare_actor(lcf::rpg::Actor orig, lcf::rpg::Actor work) {
    lcf::rpg::Actor empty;
    if (orig == empty && work != empty) {
        return "+";
    } else if (orig != empty && work == empty) {
        return "-";
    } else {
        return "*";
    }
}
