#include "sKeyboard.h"
#include "sWindow.h"

//! singleton instance
SINGLETON_INSTANCE(sKeyboard);

//! @brief 
sKeyboard::sKeyboard() {

}
//! @brief 
sKeyboard::~sKeyboard() {
	
}
//! @brief 
void sKeyboard::init() {
	memset(mKeyinfo, 0, sizeof(mKeyinfo));
	if (FAILED(DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)(&mpInput), NULL))) {
		throw std::runtime_error("error: create DirectInput");
	}
	if (FAILED(mpInput->CreateDevice(GUID_SysKeyboard, &mpDevice, NULL))) {
		throw std::runtime_error("error: create KeyboardDevice");
	}
	mpDevice->SetDataFormat(&c_dfDIKeyboard);
	mpDevice->SetCooperativeLevel(IWindow->getHandle(), DISCL_NONEXCLUSIVE|DISCL_BACKGROUND);
	mpDevice->Acquire();
}
//! @brief 
void sKeyboard::shutdown() {
	SAFE_RELEASE(mpDevice);
	SAFE_RELEASE(mpInput);
}
//! @brief 
void sKeyboard::update() {
	int input = 0;
	BYTE keys[256];
	mpDevice->GetDeviceState(sizeof(keys), &keys);
	for (int i = 0; i < _countof(mKeyinfo); i++) {
		if (mKeyinfo[i] < 0) {
			mKeyinfo[i] = 0;
		}
		if (keys[i] & 0x80) {
			mKeyinfo[i] = min(127, mKeyinfo[i]+1);
		} else 
		if (mKeyinfo[i] > 0){
			mKeyinfo[i] = -1;
		}
	}
}
//! @brief 
bool sKeyboard::isDown(int code) {
	return mKeyinfo[code] > 0;
}
//! @brief 
bool sKeyboard::isTrigger(int code) {
	return mKeyinfo[code] == 1;
}
//! @brief 
bool sKeyboard::isRepeat(int code) {
	return isTrigger(code) || (mKeyinfo[code] > 1 && (mKeyinfo[code]-1) % 7 == 0);
}
//! @brief 
bool sKeyboard::isRelease(int code) {
	return mKeyinfo[code] < 0;
}