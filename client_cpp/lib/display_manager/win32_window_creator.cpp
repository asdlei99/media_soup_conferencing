#include <cassert>

#include "win32_window_creator.h"
#include "win32_window.h"

constexpr const wchar_t* WNDCLASS_NAME = L"Sample Window Class";
constexpr const wchar_t* WND_NAME = L"Main Window";

namespace display {

	create_win32_window::create_win32_window(int width, int height, HINSTANCE instance, const wchar_t* wnd_name, const wchar_t* class_name)
		: instance_{instance},
		class_name_{ class_name }  {
		HWND hwnd = CreateWindowEx(
			0,                      // no extended styles           
			class_name_,           // class name                   
			wnd_name,          // window name                  
			WS_OVERLAPPEDWINDOW,   // overlapped window    
			0,          // default horizontal position  
			0,          // default vertical position    
			width,          // default width                
			height,          // default height               
			(HWND)NULL,            // no parent or owner window    
			(HMENU)NULL,           // class menu used              
			(HINSTANCE)instance_,              // instance handle              
			NULL);                  // no window creation data 
		if (!hwnd)
		{
			assert(false); //todo : need to handle this case.
			return;
		}

		parent_window_ = hwnd;
	}
	create_win32_window::~create_win32_window() {
		DestroyWindow(parent_window_);
	}

	HWND create_win32_window::get_handle() {
		return parent_window_;
	}

	window* create_win32_window::create_window(std::string wnd_name) {
		HWND child_window = CreateWindowEx(0, class_name_, std::wstring{ wnd_name.begin(), wnd_name.end() }.c_str(), WS_CHILD | WS_BORDER,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent_window_, (HMENU)(int)(0), instance_, NULL);

		if (!child_window) {
			assert(false);
			return nullptr;
		}
		return new win32_window{ child_window, wnd_name };

	}
	
	std::unique_ptr< window_creator> get_wnd_creator(int width, int height,
		HINSTANCE instance, const wchar_t* wnd_name, const wchar_t* class_name) {
		return std::make_unique< create_win32_window>(width, height, instance, wnd_name, class_name);
	}

	std::unique_ptr< window_creator> get_wnd_creator(HINSTANCE instance) {
		return get_wnd_creator(get_desktop_width(), get_desktop_height(),
			instance, get_default_wnd_name(), get_default_class_name());
	}

	std::unique_ptr< window_creator> get_wnd_creator(HINSTANCE instance, const wchar_t* class_name) {
		return get_wnd_creator(get_desktop_width(), get_desktop_height(),
			instance, get_default_wnd_name(), class_name);
	}
	
	const wchar_t* get_default_class_name() {
		return WNDCLASS_NAME;
	}

	const wchar_t* get_default_wnd_name() {
		return WND_NAME;
	}

}		//display
