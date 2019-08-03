#ifndef __WIN32_WINDOW__
#define __WIN32_WINDOW__

#include <Windows.h>
#include "window.h"
#include <utility>

namespace display {

	class win32_window :public window {
	public:
		win32_window(HWND handle);
		~win32_window();
		void reposition(int x, int y, int w, int h) override;


	private:
		HWND hwnd_;
	};

	std::pair<int, int>
		get_desktop_width_height();

	int get_desktop_width();
	int get_desktop_height();

}		//display


#endif	//__WIN32_WINDOW__

