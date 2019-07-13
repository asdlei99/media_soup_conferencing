#ifndef __ROOM_H____
#define __ROOM_H____

namespace grt {
	class room {
	public:
		virtual ~room(){}
		virtual void enter() = 0;
	};
}//namespace grt

#endif//__ROOM_H____