#pragma once
#include "Application/aRenderDevice.h"

class sDX12Device : public aRenderDevice {
SINGLETON_DEFINED(sDX12Device)

public:
	void setup();
	void shutdown();

private:
	void createDevice();

private:
	IDXGIFactory4* mDXGIFactory;
	IDXGIAdapter3* mAdapter;
	ID3D12Device*  mDevice;
};

//! accessor
#define IDX12Device SINGLETON_ACCESSOR(sDX12Device)