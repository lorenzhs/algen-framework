#pragma once

#include <sstream>

namespace common {
namespace term {

enum colour : int {
    fg_black = 30,
    fg_red = 31,
    fg_green = 32,
    fg_yellow = 33,
    fg_blue = 34,
    fg_magenta = 35,
    fg_cyan = 36,
    fg_white = 37,

    bg_black = 40,
    bg_red = 41,
    bg_green = 42,
    bg_yellow = 43,
    bg_blue = 44,
    bg_magenta = 45,
    bg_cyan = 46,
    bg_white = 47,
};

// ANSI escape codes
static constexpr auto bold = "\33[1m";
static constexpr auto underline = "\33[4m";
static constexpr auto reverse = "\33[7m";
static constexpr auto reset = "\33[0m";

static constexpr auto erase_line = "\33[K";

static constexpr auto clear_screen = "\33[2J";

const std::string set_colour(colour c) {
    std::stringstream s;
    s << "\33[" << (int)c << "m";
    return s.str();
}

}
}
