#ifndef __WINDOW__
#define __WINDOW__

namespace display {
	class window {
	public:
		virtual ~window() {}
		virtual void reposition(int x, int y, int w, int h) = 0;
	};
}		//display
#endif	//__WINDOW__
