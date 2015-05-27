#pragma once

#include <sstream>

namespace common {

enum term_colour : int {
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
#define term_bold "\33[1m"
#define term_underline "\33[4m"
#define term_reverse "\33[7m"
#define term_reset "\33[0m"

#define term_erase_line "\33[K"

#define term_clear_screen "\33[2J"

std::string term_set_colour(term_colour colour) {
	std::stringstream s;
	s << "\33[" << (int)colour << "m";
	return s.str();
}

}