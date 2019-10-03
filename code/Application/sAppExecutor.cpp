#include "sAppExecutor.h"
#include "sWindow.h"
#include "sTime.h"
#include "DirectX11/sDX11Device.h"
#include "DirectX12/sDX12Device.h"
#include <thread>
#include <mutex>

#define _DX11
#if   defined(_DX11)
#define IDevice IDX11Device
#elif defined(_DX12)
#define IDevice IDX12Device
#endif

//! static member
SINGLETON_INSTANCE(sAppExecutor);

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
	IDevice->setup();
	mApp->init();
}
//! @brief 
void sAppExecutor::run(){
	std::mutex mtx;
	bool isEndThread = false;

	//! application core
	std::thread mainThread([&]() {
		ITime->reset();
		ITime->setFiexedTime(1.f/60);
		while (true) {
			ITime->update();
			if (ITime->isElapsedFixedTime()) {
				mApp->update();
				mApp->draw();
				IDevice->render();
			}
			std::lock_guard<std::mutex> lock(mtx);
			if (isEndThread) break;
		}
	});

	//! current core is window-event proc only.
	while (!IWindow->shouldClose()) {
		IWindow->waitEvents();
	}
	{
		std::lock_guard<std::mutex> lock(mtx);
		isEndThread = true;
	}
	mainThread.join();
}
//! @brief 
void sAppExecutor::shutdown() {
	mApp->release();
	delete mApp;
	IDevice->shutdown();
	IDevice->destroy();
	IWindow->destroy();
	ITime->destroy();
}