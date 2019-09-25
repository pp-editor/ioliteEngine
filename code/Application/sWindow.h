#pragma once

//! @class 
class sWindow {
SINGLETON_DEFINED(sWindow)

public:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK ClassWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void setup(const char* title, u32 width, u32 height, HINSTANCE hInst, s32 nCmdShow);
	bool shouldClose();
	void waitEvents();
	void closeRequest();
	HWND getHandle();
	void getClientRect(UINT& width, UINT& height);

private:
	const char*  mTitle;
	u32          mWidth;
	u32          mHeight;
	HWND         mHandle;
	bool         mIsClose;
};

//! accessor
#define IWindow SINGLETON_ACCESSOR(sWindow)