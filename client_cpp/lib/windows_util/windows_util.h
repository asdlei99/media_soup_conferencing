#include <string>
#include <windows.h>

namespace util {
	HWND find_child_window(std::string className, std::string parent_wnd_name, std::string child_wnd_name);
	HWND find_child_window(const wchar_t* className, const wchar_t* parent_wnd_name,
		const wchar_t* child_wnd_name);
}