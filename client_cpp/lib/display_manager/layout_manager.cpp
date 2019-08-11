#include "layout_manager.h"
#include <algorithm>

namespace display {
	layout_manager::layout_manager(int width, int height, int aspect_w, int aspect_h) : 
		width_{ width }, height_{ height }, aspect_w_{ aspect_w }, aspect_h_{aspect_h}{
	}

	void layout_manager::add(window* child) {
		v_child_wind_.push_back(child);
		reposition();
	}
	
	void layout_manager::remove(window* child) {

		auto iter = std::find(v_child_wind_.begin(), v_child_wind_.end(), child);
		v_child_wind_.erase(iter);
		reposition();
		/*int vector_index{ 0 };
		for (const auto& i : v_child_wind_) {
			if (child == i) {				
				v_child_wind_.erase(v_child_wind_.begin() + vector_index);
				delete child;
				reposition();
				break;
			}
			vector_index++;
		}*/
	}

	void layout_manager::reposition() {
		int x{ 0 };
		int y{ 0 };
		int w{ 0 };
		int h{ 0 };
		int i = 1;

		auto it = v_child_wind_.begin();
		for (; it != v_child_wind_.end(); it++) {
			auto chd = *it;
			int size = v_child_wind_.size()-1;
			
			get_window_position(i, size, x, y, w, h);
			i++;

			chd->reposition(x, y, w, h);
		}
	}

	int layout_manager::get_window_position(int window_number, int total_window, int &x, int &y, int &w, int &h) {
		if (total_window == 0) {
			x = 0;
			y = 0;
			w = width_;
			h = height_;
		}
		else if (total_window == 1) {
			w = width_ / 2;
			h = (w * aspect_h_) / aspect_w_;
			if (window_number == 1) {
				x = 0;
				y = height_ / 4;
			}
			else if (window_number == 2) {
				x = width_ / 2;
				y = height_ / 4;
			}
			return 1;
		}
		else if (total_window < 4) {
			w = width_ / 2;
			h = (w * aspect_h_) / aspect_w_;
			if (window_number == 1) {
				x = 0;
				y = 0;
			}
			else if (window_number == 2) {
				x = width_ / 2;
				y = 0;
			}
			else if (window_number == 3) {
				if (total_window == 2)
					x = width_ / 4;
				else
					x = 0;

				y = height_ / 2;
			}
			else if (window_number == 4) {
				x = width_ / 2;
				y = height_ / 2;
			}
			return 1;
		}
		else if (total_window < 6) {
			w = width_ / 3;
			h = (w * aspect_h_) / aspect_w_;
			int remaining_height = height_ - h * 2;
			if (window_number == 1) {
				x = 0;
				y = remaining_height / 2;
			}
			else if (window_number == 2) {
				x = width_ / 3;
				y = remaining_height / 2;
			}
			else if (window_number == 3) {
				x = (width_ / 3) * 2;
				y = remaining_height / 2;
			}
			else if (window_number == 4) {
				x = 0;
				if (total_window == 4)
					x = (width_ / 3) / 2;
				y = height_ / 2;
			}
			else if (window_number == 5) {
				x = width_ / 3;
				if (total_window == 4)
					x = ((width_ / 3) / 2) + x;

				y = height_ / 2;
			}
			else if (window_number == 6) {
				x = (width_ / 3) * 2;
				y = height_ / 2;
			}
		}
		return -1;
	}

}