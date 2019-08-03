#ifndef __LAYOUT_MANAGER__
#define __LAYOUT_MANAGER__

#include <vector>
#include "window.h"

namespace display {

	class layout_manager {
	public:
		layout_manager(int width, int height, int aspect_w, int aspect_h);
		void add(window* child);
		void remove(window* child);

	private:
		int get_window_position(int window_number, int total_window, int &x, int &y, int &w, int &h);

	private:
		void reposition();
		std::vector<window*> v_child_wind_;

		int width_{ 0 };
		int height_{ 0 };
		int aspect_w_{ 0 };
		int aspect_h_{ 0 };
	};

}	//display
#endif	//__LAYOUT_MANAGER__
