#include "sMouse.h"
#include "sWindow.h"

//! singleton instance
SINGLETON_INSTANCE(sMouse);

//! @brief 
sMouse::sMouse() {
	init();
}
//! @brief 
sMouse::~sMouse() {

}
//! @brief 
void sMouse::init() {
	mSensitivily   = 1.0f;
	mMovement      = 0.0f;
	mPosX          = 0.0f;
	mPosY          = 0.0f;
	mPosX_Old      = 0.0f;
	mPosY_Old      = 0.0f;
	mFirstUpdate   = true;
	mIsFixedCursor = false;
	mCursorShowCount  = 0;
}
//! @brief 
void sMouse::shutdown() {

}
//! @brief 
void sMouse::update() {
	if (IWindow->isActive()) {
		int codes[] = { VK_LBUTTON, VK_MBUTTON, VK_RBUTTON };
		for (int i = 0; i < _countof(codes); i++) {
			if (mMouseButton[i] < 0) {
				mMouseButton[i] = 0;
			}
			if (GetAsyncKeyState(codes[i]) & 0x8000) {
				mMouseButton[i] = min(127, mMouseButton[i]+1);
			} else 
			if (mMouseButton[i] > 0) {
				mMouseButton[i] = -1;
			}
		}
	}

	//! active check
	if (!IWindow->isActive() || !IWindow->isMouseInnerClient()) {
		mPosX     = 0.0f;
		mPosY     = 0.0f;
		mPosX_Old = 0.0f;
		mPosY_Old = 0.0f;
		mFirstUpdate = true;
		return;
	}
	
	//! get screen position
	POINT p = {};
	GetCursorPos(&p);
	if (mFirstUpdate) {
		showCursor(!mIsFixedCursor);
		mPosX_Old = (float)p.x;
		mPosY_Old = (float)p.y;
		mFirstUpdate = false;
	} else {
		mPosX_Old = mPosX;
		mPosY_Old = mPosY;
	}
	mPosX     = (float)p.x;
	mPosY     = (float)p.y;
	
	//! fixed Cursor
	if (mIsFixedCursor) {
		POINT pos;
		UINT width, height;
		IWindow->getClientRect(width, height);
		if (mFixedPosX < 0 || mFixedPosY < 0) {
			pos.x = width / 2;
			pos.y = height / 2;
			ClientToScreen(IWindow->getHandle(), &pos);
		} else {
			pos.x = mFixedPosX;
			pos.y = mFixedPosY;
		}
		SetCursorPos(pos.x, pos.y);
		mPosX_Old = (float)pos.x;
		mPosY_Old = (float)pos.y;
	}

	//! get movement
	float movX = mPosX - mPosX_Old;
	float movY = mPosY - mPosY_Old;
	if (movX != 0 || movY != 0) {
		mMovement = sqrtf(movX * movX + movY * movY);
	} else {
		mMovement = 0.f;
	}
}
//! @brief 
float sMouse::getMovement() {
	return mMovement * mSensitivily;
}
//! @brief 
void sMouse::getMoveDirection(float& x, float& y) {
	x = (mPosX - mPosX_Old);
	y = (mPosY - mPosY_Old);
	if (x != 0 || y != 0) {
		x /= mMovement;
		y /= mMovement;
	}
}
//! @brief 
void sMouse::setFixedCursor(bool isFixedCursor){
	mIsFixedCursor = isFixedCursor;
	showCursor(!mIsFixedCursor);
	if (!mIsFixedCursor) {
		mFixedPosX = -1;
		mFixedPosY = -1;
	}
}
//! @brief 
void sMouse::setCurrentlyPosToFixedPos() {
	POINT p;
	GetCursorPos(&p);
	mFixedPosX = p.x;
	mFixedPosY = p.y;
}
//! @brief 
void sMouse::showCursor(bool isShow) {
	CURSORINFO info;
	info.cbSize = sizeof(CURSORINFO);
	GetCursorInfo(&info);
	if (isShow) {
		while(mCursorShowCount < 0){ 
			mCursorShowCount = ShowCursor(TRUE); 
		}
//		std::cout << "show: " << mCursorShowCount << std::endl;
	} else {
		while(mCursorShowCount >= 0){
			mCursorShowCount = ShowCursor(FALSE);
		}
//		std::cout << "hide: " << mCursorShowCount << std::endl;
	}
}
//! @brief 
bool sMouse::isDown(eMouseInput button) {
	return mMouseButton[(int)button] > 0;
}
//! @brief 
bool sMouse::isTrigger(eMouseInput button) {
	return mMouseButton[(int)button] == 1;
}
//! @brief 
bool sMouse::isRepeat(eMouseInput button) {
	return isTrigger(button) && (mMouseButton[(int)button] > 1 && (mMouseButton[(int)button]-1) % 7 == 0);
}
//! @brief 
bool sMouse::isRelease(eMouseInput button) {
	return mMouseButton[(int)button] < 0;
}