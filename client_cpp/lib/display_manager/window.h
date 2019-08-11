#ifndef __WINDOW__
#define __WINDOW__
#include <string>
#include <Windows.h>

namespace display {
	class window {
	public:
		virtual ~window() {}
		virtual void reposition(int x, int y, int w, int h) = 0;
		virtual HWND get_handle() const = 0;
		virtual std::string get_window_name() const = 0;
	};
}		//display
#endif	//__WINDOW__
