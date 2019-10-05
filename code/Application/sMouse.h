#pragma once

#include "Utility/cDeltaTime.h"

enum class eMouseInput : int {
	LB, MB, RB, _MAX,
};

//! @class 
class sMouse {
	SINGLETON_DEFINED(sMouse)

public:
	void  init();
	void  shutdown();
	void  update();
	float getMovement();
	void  getMoveDirection(float& x, float& y);
	void  setCurrentlyPosToFixedPos();
	void  setFixedCursor(bool isFixedCursor);
	void  setSensitivily(float sensi = 1.0f) { mSensitivily = sensi; }
	void  showCursor(bool isShow);
	bool  isDown   (eMouseInput button);
	bool  isTrigger(eMouseInput button);
	bool  isRepeat (eMouseInput button);
	bool  isRelease(eMouseInput button);

private:
	int8_t mMouseButton[(size_t)eMouseInput::_MAX];
	bool   mIsFixedCursor;
	float  mSensitivily;
	float  mMovement;
	float  mPosX;
	float  mPosY;
	float  mPosX_Old;
	float  mPosY_Old;
	int    mFixedPosX;
	int    mFixedPosY;
	bool   mFirstUpdate;
	int    mCursorShowCount;
};

//! accessor
#define IMouse SINGLETON_ACCESSOR(sMouse)