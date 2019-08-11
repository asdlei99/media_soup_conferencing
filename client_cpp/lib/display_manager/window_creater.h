#ifndef __WINDOW_CREATER__
#define __WINDOW_CREATER__

#include "window.h"
#include <memory>
namespace display {
	
	class window_creator {
	public:
		virtual window* create_window( std::string wnd_name) = 0;
		virtual HWND get_handle() = 0;
	};


	std::unique_ptr< window_creator> get_wnd_creator(int width, int height,
		HINSTANCE instance, const wchar_t* wnd_name, const wchar_t* class_name);

	std::unique_ptr< window_creator> get_wnd_creator(HINSTANCE instance);

	std::unique_ptr< window_creator> get_wnd_creator(HINSTANCE instance, const wchar_t* class_name);

	const wchar_t* get_default_class_name();

	const wchar_t* get_default_wnd_name();

	

}
#endif	//__WINDOW_CREATER__
