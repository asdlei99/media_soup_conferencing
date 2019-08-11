#ifndef __WIN32_WIN_CREATOR__
#define __WIN32_WIN_CREATOR__

#include <Windows.h>
#include "window_creater.h"

namespace display {

	class create_win32_window : public window_creator{
	public:
		create_win32_window(int width, int height, HINSTANCE instance, const wchar_t* wnd_name, const wchar_t* class_name);
		~create_win32_window();

		window* create_window(std::string wnd_name) override;
		HWND get_handle() override;

	private:
		HWND parent_window_;
		HINSTANCE instance_;
		const wchar_t* class_name_{ nullptr };
	};

}		//display
#endif		//		__WIN32_WIN_CREATOR__
