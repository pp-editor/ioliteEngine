#pragma once

//! @class 
class sKeyboard {
SINGLETON_DEFINED(sKeyboard)

public:
	void init();
	void shutdown();
	void update();
	bool isDown   (int code); //!< during key down
	bool isTrigger(int code); //!< first  key down 
	bool isRepeat (int code); //!< mod N  key down
	bool isRelease(int code); //!< key up

private:
	LPDIRECTINPUT8       mpInput;
	LPDIRECTINPUTDEVICE8 mpDevice;
	int8_t               mKeyinfo[0xFF];
};

#define IKeyboard SINGLETON_ACCESSOR(sKeyboard)