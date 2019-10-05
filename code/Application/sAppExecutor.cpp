//! -------------------------------------------------------------------------------------
//! include
//! -------------------------------------------------------------------------------------
#include "sAppExecutor.h"
#include "sWindow.h"
#include "sTime.h"
#include "sMouse.h"
#include "sKeyboard.h"
#include "DirectX11/sDX11Device.h"
#include "DirectX12/sDX12Device.h"
#include <thread>
#include <mutex>

//! -------------------------------------------------------------------------------------
//! define
//! -------------------------------------------------------------------------------------
//! switch render Device
#define _DX11
#if   defined(_DX11)
#define IDevice IDX11Device
#elif defined(_DX12)
#define IDevice IDX12Device
#endif

//! separate Thread to WindowsEvent & Game
#define WIN_MULTI_THREAD (1)

//! static member
SINGLETON_INSTANCE(sAppExecutor);

//! -------------------------------------------------------------------------------------
//! class functions
//! -------------------------------------------------------------------------------------
//! @brief constractor
sAppExecutor::sAppExecutor() : mApp(nullptr) {

}
//! @brief destractor
sAppExecutor::~sAppExecutor() {

}
//! @brief 
void sAppExecutor::setup(aApp* app, HINSTANCE hInst, s32 nCmdShow) {
	mApp = app;
	IWindow->setup("window", 1280, 720, hInst, nCmdShow);
	IMouse->init();
	IKeyboard->init();
	IDevice->setup();
	mApp->init();
}
//! @brief 
void sAppExecutor::run(){
	//! application core
	auto initFunc = [&]() {
		ITime->reset();
		ITime->setFiexedTime(1.f/60);
	};
	auto mainFunc = [&]() {
		ITime->update();
		if (ITime->isElapsedFixedTime()) {
			IMouse   ->update();
			IKeyboard->update();
			mApp     ->update();
			mApp     ->draw();
			IDevice  ->render();
		}
	};

#if WIN_MULTI_THREAD
	std::mutex mtx;
	bool isEndThread = false;
	DWORD sysThreadId = GetCurrentThreadId();
	std::thread mainThread([&]() {
		initFunc();
		while (true) {
			AttachThreadInput(sysThreadId, GetCurrentThreadId(), TRUE);
			mainFunc();
			std::lock_guard<std::mutex> lock(mtx);
			if (isEndThread) break;
		}
	});
	while (!IWindow->shouldClose()) {
		IWindow->waitEvents();
	}
	{
		std::lock_guard<std::mutex> lock(mtx);
		isEndThread = true;
	}
	mainThread.join();
#else
	initFunc();
	while (!IWindow->shouldClose()) {
		IWindow->waitEvents();
		mainFunc();
	}
#endif
}
//! @brief 
void sAppExecutor::shutdown() {
	mApp->release();
	delete mApp;
	IDevice  ->shutdown();
	IDevice  ->destroy();
	IKeyboard->shutdown();
	IKeyboard->destroy();
	IMouse   ->shutdown();
	IMouse   ->destroy();
	ITime    ->destroy();
	IWindow  ->destroy();
}