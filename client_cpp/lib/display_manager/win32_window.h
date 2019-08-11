#ifndef __WIN32_WINDOW__
#define __WIN32_WINDOW__

#include <Windows.h>
#include "window.h"
#include <utility>

namespace display {

	class win32_window :public window {
	public:
		win32_window(HWND handle, std::string name);
		~win32_window();
		void reposition(int x, int y, int w, int h) override;
		HWND get_handle() const override {return hwnd_;}
		std::string get_window_name() const override { return name_; }

	private:
		HWND hwnd_;
		const std::string name_;
	};

	std::pair<int, int>
		get_desktop_width_height();

	int get_desktop_width();
	int get_desktop_height();

}		//display


#endif	//__WIN32_WINDOW__

