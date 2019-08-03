#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "../lib/display_manager/win32_window.h"

TEST_CASE("window resolution", "[wnd_resolution]") {
	const auto w_h = display::get_desktop_width_height();
	REQUIRE(w_h.first > 0);
	REQUIRE(w_h.second > 0);
	std::cout << "Please enter width of desktop\n";
	int width = 0; 
	std::cin >> width;
	std::cout << "\nPlease enter height of desktop\n";
	int height = 0;
	std::cin >> height;
	REQUIRE(w_h.first == width);
	REQUIRE(w_h.second == height);
	{
		const auto h = display::get_desktop_height();
		REQUIRE(h == height);
	}

	{
		const auto w = display::get_desktop_width();
		REQUIRE(w == width);
	}

	
}