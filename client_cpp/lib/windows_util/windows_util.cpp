#include "windows_util.h"
#include <cassert>

namespace util {
	HWND find_child_window(const wchar_t* className, const wchar_t* parent_wnd_name,
		const wchar_t* child_wnd_name) {
		auto parentWnd = FindWindow(className, parent_wnd_name);
		assert(parentWnd);
		if (parentWnd == nullptr) return nullptr;
		auto wnd = FindWindowEx(parentWnd, nullptr, className, child_wnd_name);
		assert(wnd);
		return wnd;
	}

	std::wstring to_wstring(std::string v) {
		return std::wstring(v.begin(), v.end());
	}

	HWND find_child_window(std::string className, std::string parent_wnd, std::string child_name) {
		return find_child_window(to_wstring(className).c_str(),
			to_wstring(parent_wnd).c_str(), to_wstring(child_name).c_str());
	}

}