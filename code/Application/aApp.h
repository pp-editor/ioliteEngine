#pragma once

//! @class 
class aApp {
public:
	aApp() {}
	virtual ~aApp() {}
	virtual void init()    = 0;
	virtual void release() = 0;
	virtual void update()  = 0;
	virtual void draw()    = 0;
};