#pragma once

#include "Utility/cDeltaTime.h"

class sTime : public cDeltaTime {
SINGLETON_DEFINED(sTime)
public:
	float getDeltaTime();
};

//! accessor
#define ITime SINGLETON_ACCESSOR(sTime)