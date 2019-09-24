#pragma once
#include "Application/aApp.h"

class cDX11App : public aApp {
public:
	cDX11App();
	~cDX11App();

	void init();
	void release();
	void update();
	void draw();
};