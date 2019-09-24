#pragma once

#include "aApp.h"

//! @class 
class sAppExecutor {
SINGLETON_DEFINED(sAppExecutor)

public:
	void setup(aApp* app, HINSTANCE hInst, s32 nCmdShow);
	void run();
	void shutdown();

private:
	aApp* mApp;
};

//! accessor
#define IAppExecutor SINGLETON_ACCESSOR(sAppExecutor)