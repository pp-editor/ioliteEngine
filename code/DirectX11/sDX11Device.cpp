//! -------------------------------------------------------------------------------------
//! Include
//! -------------------------------------------------------------------------------------
#include "Application/sWindow.h"
#include "DirectX11/sDX11Device.h"
#include "DirectX11/uDX11RenderScene_Demo00.h"
#include "Utility/nString.h"


//! -------------------------------------------------------------------------------------
//! Flags
//! -------------------------------------------------------------------------------------
// device flags
namespace nDeviceSettings {
#if defined(_DEBUG)
	UINT deviceFlags  = D3D11_CREATE_DEVICE_DEBUG;
	UINT compileFlags = D3DCOMPILE_DEBUG|D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT deviceFlags  = 0;
	UINT compileFlags = 0;
#endif

	D3D_DRIVER_TYPE driverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
}

//! -------------------------------------------------------------------------------------
//! Called by Engine
//! -------------------------------------------------------------------------------------
//! static member
SINGLETON_INSTANCE(cDX11Device);

//! @brief constractor
cDX11Device::cDX11Device():
				 mDepthStencilFormat      (DXGI_FORMAT_D24_UNORM_S8_UINT)
				,mDepthStencilSampleCount (0)
	            ,mpRenderScene            (nullptr)
{

}
//! @brief destractor
cDX11Device::~cDX11Device() {

}

//! @brief 
void cDX11Device::setup() {
	createDevice();
	mpRenderScene = new uDX11RenderScene_Demo00();
	mpRenderScene->init();
}
//! @brief 
void cDX11Device::shutdown() {
	if (mpContext) {
		mpContext->ClearState();
		mpContext->Flush();
	}

	SAFE_RELEASE(mpRenderScene);

	SAFE_RELEASE(mpContext);
	SAFE_RELEASE(mpDevice);
	SAFE_RELEASE(mpSwapchain);

#if _DEBUG	//!< check Resource leak
	OSVERSIONINFOEX OSver;
	OSver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	OSver.dwMajorVersion      = 6;
	OSver.dwMinorVersion      = 2;
	ULONGLONG condition = 0;
	VER_SET_CONDITION(condition, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(condition, VER_MINORVERSION, VER_GREATER_EQUAL);
	if (VerifyVersionInfo(&OSver, VER_MAJORVERSION | VER_MINORVERSION, condition)) {
		typedef HRESULT(__stdcall *fPtr)(const IID&, void**);
		HMODULE hDll = GetModuleHandleW(L"dxgidebug.dll");
		fPtr DXGIGetDebugInterface = (fPtr)GetProcAddress(hDll, "DXGIGetDebugInterface");
		IDXGIDebug* pDebug;
		DXGIGetDebugInterface(__uuidof(IDXGIDebug), (void**)&pDebug);
		if (FAILED(pDebug->ReportLiveObjects(DXGI_DEBUG_D3D11, DXGI_DEBUG_RLO_DETAIL))) {
			std::cout << "failed to ReportLiveObjects()." << std::endl;
		}
		SAFE_RELEASE(pDebug);
	}
#endif
}
//! @brief 
void cDX11Device::render() {
	if (mpRenderScene) {
		mpRenderScene->update();
		mpRenderScene->render(mpContext);
	}
	mpSwapchain->Present(0, 0);
}

//! -------------------------------------------------------------------------------------
//! Inner Function
//! -------------------------------------------------------------------------------------
//! @brief
void cDX11Device::createDevice() {
	UINT width, height;
	IWindow->getClientRect(width, height);

	//! create SwapChain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount                        = 2;
	sd.BufferDesc.Width                   = width;
	sd.BufferDesc.Height                  = height;
	sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator   = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT|DXGI_USAGE_SHADER_INPUT;
	sd.OutputWindow                       = IWindow->getHandle();
	sd.SampleDesc.Count                   = 1;
	sd.SampleDesc.Quality                 = 0;
	sd.Windowed                           = TRUE;

	HRESULT hr = S_OK;
	for (int i = 0; i < _countof(nDeviceSettings::driverTypes); i++) {
		hr = D3D11CreateDeviceAndSwapChain(NULL, nDeviceSettings::driverTypes[i], NULL, nDeviceSettings::deviceFlags,
		                                   nDeviceSettings::featureLevels, _countof(nDeviceSettings::featureLevels), D3D11_SDK_VERSION,
		                                   &sd, &mpSwapchain, &mpDevice, &mFeatureLevel, &mpContext);
		if(SUCCEEDED(hr)) break;
	}
	if (FAILED(hr)) {
		throw std::runtime_error("error: createDeviceAndSwapChain");
	}
}

//! -------------------------------------------------------------------------------------
//! DirectX Resource Helper
//! -------------------------------------------------------------------------------------
//! @brief 
ID3DBlob* cDX11Device::createShaderBlob(eShaderType type, const std::string& filename, const std::string& entrypoint) {
	ID3DBlob *pBlob, *pErrBlob;
	wchar_t filenamew[MAX_PATH*2];
	nString::multiByteCharToWideChar(filename.c_str(), filenamew, filename.length()+1);
	std::string buildtype = "";
	switch (type) {
	case eShaderType::vs:     buildtype = "vs_5_0"; break;
	case eShaderType::gs:     buildtype = "gs_5_0"; break;
	case eShaderType::ps:     buildtype = "ps_5_0"; break;
	case eShaderType::layout: buildtype = "vs_5_0"; break;
	}
	auto hr = D3DCompileFromFile(filenamew, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint.c_str(), 
	                             buildtype.c_str(), nDeviceSettings::compileFlags, 0, &pBlob, &pErrBlob);

	if (FAILED(hr)) {
		if (pErrBlob != nullptr) {
			std::cout << __FUNCTION__ << ": " << (char*)pErrBlob->GetBufferPointer() << std::endl;
			throw std::runtime_error("error: createShaderBlob");
		}
	}
	else if(pErrBlob != nullptr){
		std::string er = (char*)pErrBlob->GetBufferPointer();
		if (er.find("entrypoint not found") == std::string::npos) {
			std::cout << __FUNCTION__ << ": " << (char*)pErrBlob->GetBufferPointer() << std::endl;
			throw std::runtime_error("error: shader's entrypoint is not found");
		}
		pErrBlob->Release();
	}
	return pBlob;
}
//! @brief 
ID3D11VertexShader* cDX11Device::createVSShader(const std::string& filename, const std::string& entrypoint) {
	auto pBlob = createShaderBlob(eShaderType::vs, filename, entrypoint);
	ID3D11VertexShader* shader;
	if (FAILED(mpDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &shader))) {
		throw std::runtime_error("error: createVertexShader");
	}
	pBlob->Release();
	return shader;
}
//! @brief 
ID3D11PixelShader* cDX11Device::createPSShader(const std::string& filename, const std::string& entrypoint) {
	auto pBlob = createShaderBlob(eShaderType::ps, filename, entrypoint);
	ID3D11PixelShader* shader;
	if (FAILED(mpDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &shader))) {
		throw std::runtime_error("error: CreatePixelShader");
	}
	pBlob->Release();
	return shader;
}
//! @brief 
ID3D11GeometryShader* cDX11Device::createGSShader(const std::string& filename, const std::string& entrypoint) {
	auto pBlob = createShaderBlob(eShaderType::gs, filename, entrypoint);
	ID3D11GeometryShader* shader;
	if (FAILED(mpDevice->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &shader))) {
		throw std::runtime_error("error: CreateGeometryShader");
	}
	pBlob->Release();
	return shader;
}
//! @brief 
ID3D11InputLayout* cDX11Device::createInputLayout(D3D11_INPUT_ELEMENT_DESC* layout, UINT num, const std::string& filename, const std::string& entrypoint) {
	auto pBlob = createShaderBlob(eShaderType::layout, filename, entrypoint);
	ID3D11InputLayout* shader;
	if (FAILED(mpDevice->CreateInputLayout(layout, num, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &shader))) {
		throw std::runtime_error("error: CreateInputLayout");
	}
	pBlob->Release();
	return shader;
}
//! @brief 
ID3D11Buffer* cDX11Device::createBuffer(eBufferType type, void* data, UINT type_size, UINT num) {
	ID3D11Buffer* buffer = nullptr;
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc,sizeof(bufferDesc));
	bufferDesc.Usage          = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth      = type_size * num;
	bufferDesc.CPUAccessFlags = 0;
	switch (type) {
	case eBufferType::vertex:   bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;   break;
	case eBufferType::index:    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;    break;
	case eBufferType::constant: bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; break;
	}
	D3D11_SUBRESOURCE_DATA *subresource = nullptr, _subresource;
	if (data != nullptr) {
		ZeroMemory(&subresource, sizeof(subresource));
		_subresource.pSysMem = data;
		subresource = &_subresource;
	}
	if (FAILED(mpDevice->CreateBuffer(&bufferDesc, subresource, &buffer))) {
		throw std::runtime_error("error: createBuffer");
	}
	return buffer;
}
//! @brief 
ID3D11Buffer* cDX11Device::createVertexBuffer(Vertex* data, UINT num) {
	return createBuffer(eBufferType::vertex, data, sizeof(Vertex), num);
}
//! @brief 
ID3D11Buffer* cDX11Device::createIndexBuffer(UINT* data, UINT num) {
	return createBuffer(eBufferType::index, data, sizeof(UINT), num);
}
//! @brief
ID3D11Buffer* cDX11Device::createConstantBuffer(size_t size, UINT num) {
	return createBuffer(eBufferType::constant, nullptr, (UINT)size, num);
}
//! @brief 
ID3D11RasterizerState* cDX11Device::createRasterizerState(D3D11_CULL_MODE cull, D3D11_FILL_MODE fill) {
	ID3D11RasterizerState* pRasterizerState;
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode              = fill;
	rasterizerDesc.CullMode              = cull;
	rasterizerDesc.DepthClipEnable       = FALSE;
	rasterizerDesc.MultisampleEnable     = FALSE;
	rasterizerDesc.DepthBiasClamp        = 0;
	rasterizerDesc.SlopeScaledDepthBias  = 0;
	if (FAILED(mpDevice->CreateRasterizerState(&rasterizerDesc, &pRasterizerState))) {
		throw std::runtime_error("error: createRasterizerState");
	}
	return pRasterizerState;
}
//! @brief
void cDX11Device::setViewport(ID3D11DeviceContext* pContext, UINT width, UINT height) {
	D3D11_VIEWPORT viewport;
	viewport.Width    = (FLOAT)width;
	viewport.Height   = (FLOAT)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	pContext->RSSetViewports(1, &viewport);
}
//! @brief 
ID3D11Texture2D* cDX11Device::getBackBuffer() {
	ID3D11Texture2D* pBackBuffer = nullptr;
	if (FAILED(mpSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer))) {
		throw std::runtime_error("error: create BackBuffer");
	}
	return pBackBuffer;
}