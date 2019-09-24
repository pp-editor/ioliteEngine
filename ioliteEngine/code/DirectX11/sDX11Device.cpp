﻿//! -------------------------------------------------------------------------------------
//! Include
//! -------------------------------------------------------------------------------------
#include "Utility/nFile.h"
#include "Utility/nString.h"
#include "Application/sWindow.h"
#include "Application/sTime.h"
#include "sDX11Device.h"
#include "nFigureData.h"

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
				,mBackBuffer              ()
				,mDepthStencil            ()
{

}
//! @brief destractor
cDX11Device::~cDX11Device() {

}

//! @brief 
void cDX11Device::setup() {
	createDevice();

	//! write clear Texture
	cDX11Texture dynamicTex;
	dynamicTex.createTexture(128, 128, cDX11Texture::eTexDescType::CPUdynamic);
	{
		D3D11_MAPPED_SUBRESOURCE resource;
		dynamicTex.map(resource);
		UINT   width, height;
		dynamicTex.getTextureSize(width, height);
		float* pixels = (float*)resource.pData;
		for (UINT y = 0; y < height; y++) {
			for (UINT x = 0; x < width; x++) {
				float xl = abs((float)width /2 - x);
				float yl = abs((float)height/2 - y);
				float len = sqrtf((float)(xl*xl+yl*yl));
				float col = min(1.f, max(0.f, 1.0f - len/128));
				pixels[y*resource.RowPitch/4 + x*4 + 0] = 1.0f;
				pixels[y*resource.RowPitch/4 + x*4 + 1] = 1.0f;
				pixels[y*resource.RowPitch/4 + x*4 + 2] = 1.0f;
				pixels[y*resource.RowPitch/4 + x*4 + 3] = col*0.8f;
			}
		}
		dynamicTex.unmap();
	}
	dynamicTex.createShaderResourceView();
	dynamicTex.createSamplerState();

	//! write dice Texture
	cDX11Texture dynamicCube;
	dynamicCube.createTexture(128, 128, cDX11Texture::eTexDescType::CubeMap);
	{
		D3D11_MAPPED_SUBRESOURCE resource;
		UINT width, height;
		dynamicCube.getTextureSize(width, height);
		float* pixels = nullptr;
		auto setPixels = [](float* pixels, UINT row, UINT x, UINT y, float col){
			pixels[y*row/4 + x*4 + 0] = col;
			pixels[y*row/4 + x*4 + 1] = col;
			pixels[y*row/4 + x*4 + 2] = col;
			pixels[y*row/4 + x*4 + 3] = 1.0f;
		};
		for (UINT i = 0; i < dynamicCube.getTextureArraySize(); i++) {
			cDX11Texture tempTex;
			tempTex.createTexture(width, height, cDX11Texture::eTexDescType::CPUdynamic);
			tempTex.map(resource);
			pixels = (float*)resource.pData;
			for (UINT y = 0; y < height; y++) {
				for (UINT x = 0; x < width; x++) {
					bool isFill = false;
					for (UINT m = 0; m < 9; m++) {
						if(i == 0 && m != 4) continue;
						if(i == 1 && !(m == 0 || m == 8)) continue;
						if(i == 2 && !(m == 0 || m == 4 || m == 8)) continue;
						if(i == 3 && !(m == 0 || m == 2 || m == 6 || m == 8)) continue;
						if(i == 4 && !(m == 0 || m == 2 || m == 4 || m == 6 || m == 8)) continue;
						if(i == 5 && !(m == 0 || m == 2 || m == 3 || m == 5 || m == 6 || m == 8)) continue;
						UINT nn = m/3;
						UINT mm = m%3;
						UINT w0  = width  / 3 * (mm+0) + 8;
						UINT w1  = width  / 3 * (mm+1) - 8;
						UINT h0  = height / 3 * (nn+0) + 8;
						UINT h1  = height / 3 * (nn+1) - 8;
						if (w0 <= x && x <= w1 && h0 <= y && y <= h1) {
							setPixels(pixels, resource.RowPitch, x, y, 0.0f);
							isFill = true;
						}
					}
					if (isFill == false) {
						setPixels(pixels, resource.RowPitch, x, y, 1.0f);
					}
				}
			}
			tempTex.unmap();
			dynamicCube.copySubResource(*tempTex.get(), i);
			tempTex.Release();
		}
	}
	dynamicCube.createShaderResourceView();
	dynamicCube.createSamplerState();

	//! create renderObject
	for (int i = 0; i < 8; i++) {
		cDX11RenderObject* pCube = new cDX11RenderObject();
		Vertex* vertices = nullptr;
		UINT vNum, iNum, *indices = nullptr;
		nFigureData::getCube(vertices, vNum, indices, iNum);
		pCube->createVertexBuffer(vertices, vNum);
		pCube->createIndexBuffer(indices, iNum);
		pCube->attach(&mpVSShader, nullptr, &mpPSShader, &mpInputLayout);
		if (i == 7) {
			pCube->setTexture(dynamicTex);
			pCube->createRasterizerState(D3D11_CULL_NONE);
		} else 
		if (i == 6) {
			pCube->setTexture(dynamicCube);
			pCube->attach(&mpPSShaderCube);
		} else 
		if (i == 5) {
			pCube->createRasterizerState(D3D11_CULL_NONE, D3D11_FILL_WIREFRAME);
			pCube->attach(&mpPSShaderNoTexture);
		} else {
			pCube->createTexture(nFile::getResourcePath("resource/texture/mono.png"));
			pCube->createSamplerState();
		}
		DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
		pCube->updateWorldMatrix(matrix);
		Material material;
		material.ambient  = { 1.0f, 1.0f, 1.0f, 0.0f };
		material.diffuse  = { 1.0f, 1.0f, 1.0f, 0.0f };
		material.specular = { 1.0f, 1.0f, 1.0f,20.0f };
		pCube->updateMaterial(material);
		mpRenderObjectList.push_back(pCube);
	}

	//! settings point light
	mPointLight.position    = { 0.0f, 0.0f, 0.0f, 0.0f };
	mPointLight.diffuse     = { 1.0f, 1.0f, 1.0f, 0.0f };
	mPointLight.specular    = { 1.0f, 1.0f, 1.0f, 0.0f };
	mPointLight.shininess   = { 1.0f, 0.2f, 0.2f, 0.0f };
	mAmbientLight.color     = { 0.3f, 0.3f, 0.3f, 1.0f };
	mDirectionLight.color   = { 0.9f, 0.9f, 0.9f, 1.0f };
	mDirectionLight.position= { 0.0f, 3.0f, 0.1f, 1.0f };
}
//! @brief 
void cDX11Device::shutdown() {
	if (mpContext) {
		mpContext->ClearState();
		mpContext->Flush();
	}
	//! constant buffer
	SAFE_RELEASE(mpConstant_PointLight);
	SAFE_RELEASE(mpConstant_DirectionLight);
	SAFE_RELEASE(mpConstant_AmbientLight);
	SAFE_RELEASE(mpConstant_LightProperty);
	SAFE_RELEASE(mpConstant_Material);
	SAFE_RELEASE(mpConstant_WorldMatrix);
	SAFE_RELEASE(mpConstant_PerspectiveLight);
	SAFE_RELEASE(mpConstant_Perspective);

	//! shader resource
	SAFE_RELEASE(mpInputLayout);
	SAFE_RELEASE(mpPSShaderDepth);
	SAFE_RELEASE(mpVSShaderDepth);
	SAFE_RELEASE(mpPSShaderCube);
	SAFE_RELEASE(mpPSShaderNoTexture);
	SAFE_RELEASE(mpPSShader);
	SAFE_RELEASE(mpGSShader);
	SAFE_RELEASE(mpVSShader);

	//! objects
	auto it = mpRenderObjectList.begin();
	for (; it != mpRenderObjectList.end(); it++) {
		delete *it;
		*it = nullptr;
	}
	mpRenderObjectList.clear();

	//! unique resource
	SAFE_RELEASE(mpBlendState);
	mDepthStencil.Release();
	mBackBuffer.Release();
	SAFE_RELEASE(mpContext);
	SAFE_RELEASE(mpDevice);
	SAFE_RELEASE(mpSwapchain);
}
//! @brief 
void cDX11Device::render() {
	float depthClearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float backGroundColor[] = { 0.0f, 0.125f, 0.6f, 1.0f };
	mpContext->ClearRenderTargetView(*mBackBuffer.getRTV(), backGroundColor);
	mpContext->ClearDepthStencilView(*mDepthStencil.getDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//! update
	static float time = 0.0f;
	time += ITime->getDeltaTime();
	//	mDirectionLight.position.x = sinf(time) * 10.f;
	//	mDirectionLight.position.y = cosf(time) * 10.f;
	//	mDirectionLight.position.z = 0.0f;
	int i = 0;
	for (auto obj : mpRenderObjectList) {
		if (i == 0) {
			DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
			matrix *= DirectX::XMMatrixScaling(10.f, 0.3f, 10.f);
			matrix *= DirectX::XMMatrixTranslation(0.f, -1.f, 0.f);
			obj->updateWorldMatrix(matrix);
		}
		else {
			float theta = (float)(3.14f * 2) / (mpRenderObjectList.size() - 1) * (i - 1);// + time;
			float rotate = (float)(3.14f * 2) / (mpRenderObjectList.size() - 1) * (i - 1);
			float radius = 2.f;
			DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
			matrix *= DirectX::XMMatrixRotationY(theta + rotate);
			matrix *= DirectX::XMMatrixTranslation(sinf(theta)*radius, 0.0f, cosf(theta)*radius);
			obj->updateWorldMatrix(matrix);
/*
			DirectX::XMVECTOR litPos = DirectX::XMVectorSet( mDirectionLight.position.x, mDirectionLight.position.y, mDirectionLight.position.z, 1.0f);
			DirectX::XMVECTOR litTar = DirectX::XMVectorSet( 0.0f, -5.0f, 0.0f, 0.0f);
			DirectX::XMVECTOR litUp  = DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f);
			mPerspectiveLight.view = DirectX::XMMatrixLookAtLH(litPos, litTar, litUp);
			mPerspectiveLight.proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, 1.0f, 0.01f, 1000.0f);
			DirectX::XMVECTOR pos = {1, -5, 0, 1 };
			auto world = DirectX::XMVector4Transform(pos, matrix);
			auto view  = DirectX::XMVector4Transform(world, mPerspectiveLight.view);
			auto proj  = DirectX::XMVector4Transform(view, mPerspectiveLight.proj);
			std::cout << "(" << proj.m128_f32[0] << ", " 
				             << proj.m128_f32[1] << ", "
				             << proj.m128_f32[2] << ", " 
				             << proj.m128_f32[3] << ")" << std::endl;
*/
		}
		i++;
	}
	auto printVec = [&]() {
		std::cout << "(" << mDirectionLight.position.x << ", "
			<< mDirectionLight.position.y << ", "
			<< mDirectionLight.position.z << ")" << std::endl;
	};
	if (GetAsyncKeyState(VK_UP) & 0x8000) {
		mDirectionLight.position.z += 0.01f;
		printVec();
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
		mDirectionLight.position.z -= 0.01f;
		printVec();
	}
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
		mDirectionLight.position.x -= 0.01f;
		printVec();
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
		mDirectionLight.position.x += 0.01f;
		printVec();
	}
	static bool isHit = false;
	if ((GetAsyncKeyState(VK_SPACE) & 0x8000)) {
		if (isHit == false) {
			DirectX::ScratchImage image;
			if (FAILED(DirectX::CaptureTexture(mpDevice, mpContext, *mpShadowMapTexture.get(), image))) {
				throw std::runtime_error("error: CaptureTexture");
			}
			if (FAILED(DirectX::SaveToDDSFile(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::DDS_FLAGS_NONE, L"shadowMap.dds"))) {
				throw std::runtime_error("error: SaveToDDSFile");
			}
			isHit = true;
		}
	} else {
		isHit = false;
	}
	UINT width, height, screenW, screenH;
	IWindow->getClientRect(screenW, screenH);
	mpShadowMapTexture.getTextureSize(width, height);
	DirectX::XMVECTOR litPos = DirectX::XMVectorSet( mDirectionLight.position.x, mDirectionLight.position.y, mDirectionLight.position.z, 0.0f);
	DirectX::XMVECTOR litTar = DirectX::XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR litUp  = DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f);
	mPerspectiveLight.view = DirectX::XMMatrixLookAtLH(litPos, litTar, litUp);
	mPerspectiveLight.proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, 1.0f, 1.f, 200.0f);
//	mPerspectiveLight.proj = DirectX::XMMatrixOrthographicLH((float)width, (float)height, 1.0f, 100.f);


	//! update resource
	mpContext->UpdateSubresource(mpConstant_Perspective, 0, NULL, &mPerspective, 0, 0);
	mpContext->UpdateSubresource(mpConstant_PerspectiveLight, 0, NULL, &mPerspectiveLight, 0, 0);
	mpContext->UpdateSubresource(mpConstant_LightProperty, 0, NULL, &mLightProperty, 0, 0);
	mpContext->UpdateSubresource(mpConstant_AmbientLight, 0, NULL, &mAmbientLight, 0, 0);
	mpContext->UpdateSubresource(mpConstant_DirectionLight, 0, NULL, &mDirectionLight, 0, 0);
	mpContext->UpdateSubresource(mpConstant_PointLight, 0, NULL, &mPointLight, 0, 0);

	//! clear buffer
	ID3D11ShaderResourceView *const pSRV[1] = { NULL };
	mpContext->PSSetShaderResources(2, 1, pSRV);

	//! pre-render depth
	setViewport(mpContext, width, height);
	mpContext->VSSetShader(mpVSShaderDepth, nullptr, 0);
	mpContext->GSSetShader(mpGSShader,      nullptr, 0);
	mpContext->PSSetShader(mpPSShaderDepth, nullptr, 0);
	mpContext->OMSetRenderTargets(1, mpShadowMapTexture.getRTV(), *mpShadowMapDepth.getDSV());
	mpContext->ClearRenderTargetView(*mpShadowMapTexture.getRTV(), depthClearColor);
	mpContext->ClearDepthStencilView(*mpShadowMapDepth.getDSV(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	updateConstantBuffer();
	for (auto obj : mpRenderObjectList) {
		obj->render(mpContext, false);
	}

	//! render objects
	setViewport(mpContext, screenW, screenH);
	mpContext->OMSetRenderTargets(1, mBackBuffer.getRTV(), *mDepthStencil.getDSV());
	mpContext->PSSetShaderResources(2, 1, mpShadowMapTexture.getSRV());
	mpContext->PSSetSamplers(1, 1, mpShadowMapTexture.getSample());
	for (auto obj : mpRenderObjectList) {
		obj->render(mpContext);
	}

	mpSwapchain->Present(0, 0);
}

//! -------------------------------------------------------------------------------------
//! Render Helper
//! -------------------------------------------------------------------------------------
//! @brief
void cDX11Device::updateWolrdMatrix(ID3D11DeviceContext* context, const WorldMatrix* world) {
	context->UpdateSubresource(mpConstant_WorldMatrix, 0, NULL, world, 0, 0);
	context->VSSetConstantBuffers(eCBufferSlot::WORLD_MATRIX, 1, &mpConstant_WorldMatrix);
}
//! @brief 
void cDX11Device::updateMaterial(ID3D11DeviceContext* context, const Material* material) {
	context->UpdateSubresource(mpConstant_Material, 0, NULL, material, 0, 0);
	context->PSSetConstantBuffers(eCBufferSlot::MATERIAL, 1, &mpConstant_Material);
}
//! @brief 
void cDX11Device::updateConstantBuffer() {
	mpContext->VSSetConstantBuffers(eCBufferSlot::PERSPECTIVE,     1, &mpConstant_Perspective);
	mpContext->VSSetConstantBuffers(eCBufferSlot::LIGHT_PERSPECT,  1, &mpConstant_PerspectiveLight);
	mpContext->PSSetConstantBuffers(eCBufferSlot::LIGHT_PROPERTY,  1, &mpConstant_LightProperty);
	mpContext->PSSetConstantBuffers(eCBufferSlot::AMBIENT_LIGNT,   1, &mpConstant_AmbientLight);
	mpContext->PSSetConstantBuffers(eCBufferSlot::DIRECTION_LIGHT, 1, &mpConstant_DirectionLight);
	mpContext->PSSetConstantBuffers(eCBufferSlot::POINT_LIGHT,     1, &mpConstant_PointLight);
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

	//! create BackBaffer
	ID3D11Texture2D* pBackBuffer = nullptr;
	if (FAILED(mpSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer))) {
		throw std::runtime_error("error: create BackBuffer");
	}
	mBackBuffer.assignTexture(pBackBuffer);
	mBackBuffer.createRenderTargetView(cDX11Texture::eDescUse::None);
	mBackBuffer.createShaderResourceView(cDX11Texture::eDescUse::None);

	//! create DepthStencil
	mDepthStencil.createTexture(width, height, cDX11Texture::eTexDescType::DepthStencil);
	mDepthStencil.createDepthStencilView();
	mDepthStencil.createShaderResourceView();

	//! create BlendState
	D3D11_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable  = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	for(int i = 0; i < _countof(blendDesc.RenderTarget); i++){
		blendDesc.RenderTarget[i].BlendEnable           = TRUE;
		blendDesc.RenderTarget[i].SrcBlend              = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[i].BlendOp               = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].SrcBlendAlpha         = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[i].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	if (FAILED(mpDevice->CreateBlendState(&blendDesc, &mpBlendState))) {
		throw std::runtime_error("error : createBlendState");
	}

	//! create shadowmap resource
	mpShadowMapTexture.createTexture(1024, 1024, cDX11Texture::eTexDescType::Render);
	mpShadowMapTexture.createRenderTargetView();
	mpShadowMapTexture.createShaderResourceView();
	mpShadowMapTexture.createSamplerState(cDX11Texture::eSamplerType::DepthClamp);
	mpShadowMapDepth.createTexture(1024, 1024, cDX11Texture::eTexDescType::Depth);
	mpShadowMapDepth.createDepthStencilView();

	//! create shader
	mpVSShader = createVSShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "vsMain");
	mpGSShader = createGSShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "gsMain");
	mpPSShader = createPSShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "psMain");
	mpPSShaderNoTexture = createPSShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "psMainNoTexture");
	mpPSShaderCube      = createPSShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "psMainCube");
	mpVSShaderDepth     = createVSShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "vsMainDepth");
	mpPSShaderDepth     = createPSShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "psMainDepth");

	//! create inputLayout
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	mpInputLayout = createInputLayout(layout, _countof(layout), nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "vsMain");

	//! assign render pipeline
	mpContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	mpContext->OMSetRenderTargets(1, mBackBuffer.getRTV(), *mDepthStencil.getDSV());
	setViewport(mpContext, width, height);

	float blendFactor[4] = {D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
	mpContext->OMSetBlendState(mpBlendState, blendFactor, 0xFFFFFFFF);

	//! create PerspectiveView(ConstantBuffer)
	mpConstant_Perspective      = createConstantBuffer(sizeof(PerspectiveViewMatrix), 1);
	mpConstant_PerspectiveLight = createConstantBuffer(sizeof(PerspectiveViewMatrix), 1);
	mpConstant_WorldMatrix      = createConstantBuffer(sizeof(WorldMatrix), 1);
	mpConstant_LightProperty    = createConstantBuffer(sizeof(LightProperty), 1);
	mpConstant_Material         = createConstantBuffer(sizeof(Material), 1);
	mpConstant_AmbientLight     = createConstantBuffer(sizeof(AmbientLight), 1);
	mpConstant_DirectionLight   = createConstantBuffer(sizeof(DirectionalLight), 1);
	mpConstant_PointLight       = createConstantBuffer(sizeof(PointLight), 1);

	DirectX::XMVECTOR camPos = DirectX::XMVectorSet(2.0f, 2.0f, -2.0f, 1.0f);
	DirectX::XMVECTOR camTar = DirectX::XMVectorSet(0.0f, 0.0f,  0.0f, 1.0f);
	DirectX::XMVECTOR camUp  = DirectX::XMVectorSet(0.0f, 1.0f,  0.0f, 1.0f);
	FLOAT aspectRatio = (FLOAT)width / height;
	mPerspective.view = DirectX::XMMatrixLookAtLH(camPos, camTar, camUp);
	mPerspective.proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, aspectRatio, 1.f, 200.0f);
	memcpy(&mLightProperty.eyePosition, camPos.m128_f32, sizeof(mLightProperty.eyePosition));
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
	rasterizerDesc.FillMode             = fill;
	rasterizerDesc.CullMode             = cull;
	rasterizerDesc.DepthClipEnable      = FALSE;
	rasterizerDesc.MultisampleEnable    = FALSE;
	rasterizerDesc.DepthBiasClamp       = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
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