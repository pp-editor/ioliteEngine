#include "sDX12Device.h"

//! static member
SINGLETON_INSTANCE(sDX12Device);

//! @brief constractor
sDX12Device::sDX12Device() {

}
//! @brief destractor
sDX12Device::~sDX12Device() {

}

//! @brief 
void sDX12Device::setup() {
	createDevice();
}
//! @brief 
void sDX12Device::shutdown() {
	SAFE_RELEASE(mDevice);
	SAFE_RELEASE(mAdapter);
	SAFE_RELEASE(mDXGIFactory);
}

//! @brief
void sDX12Device::createDevice() {

	if (CreateDXGIFactory2(0, IID_PPV_ARGS(&mDXGIFactory)) < 0) {
		throw std::runtime_error("failed to create DXGIFactory.");
	}
	if (mDXGIFactory->EnumAdapters(0, (IDXGIAdapter**)&mAdapter) < 0) {
		throw std::runtime_error("faild to EnumAdapters.");
	}
	DXGI_ADAPTER_DESC desc;
	mAdapter->GetDesc(&desc);
	if (D3D12CreateDevice(mAdapter, D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&mDevice)) < 0) {
		throw std::runtime_error("failed to create D3D12Device.");
	}
}