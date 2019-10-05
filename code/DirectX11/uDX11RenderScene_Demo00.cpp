#include "Application/sTime.h"
#include "Application/sWindow.h"
#include "DirectX11/sDX11Device.h"
#include "DirectX11/nFigureData.h"
#include "Utility/nFile.h"
#include "Utility/nModelLoader.h"
#include "uDX11RenderScene_Demo00.h"

//! accessor
#define IGetDevice  IDX11Device->getDevice()

//! @brief 
uDX11RenderScene_Demo00::uDX11RenderScene_Demo00() : 
	cDX11RenderScene () 
	,mBackBuffer     ()
	,mDepthStencil   ()
{

}
//! @brief 
uDX11RenderScene_Demo00::~uDX11RenderScene_Demo00() {

}
//! @brief 
void uDX11RenderScene_Demo00::Release() {
	//! constant buffer
	SAFE_RELEASE(mpConstant_ScreenProj);
	SAFE_RELEASE(mpConstant_PointLight);
	SAFE_RELEASE(mpConstant_DirectionLight);
	SAFE_RELEASE(mpConstant_AmbientLight);
	SAFE_RELEASE(mpConstant_LightProperty);
	SAFE_RELEASE(mpConstant_Material);
	SAFE_RELEASE(mpConstant_WorldMatrix);
	SAFE_RELEASE(mpConstant_PerspectiveLight);
	SAFE_RELEASE(mpConstant_Perspective);

	//! shader resource
	SAFE_RELEASE(mpInputLayout2D);
	SAFE_RELEASE(mpPShader2D);
	SAFE_RELEASE(mpVShader2D);
	SAFE_RELEASE(mpInputLayout);
	SAFE_RELEASE(mpVShaderDepth);
	SAFE_RELEASE(mpPShaderCube);
	SAFE_RELEASE(mpPShaderNoTexture);
	SAFE_RELEASE(mpPShader);
	SAFE_RELEASE(mpGShader);
	SAFE_RELEASE(mpVShader);

	//! objects
	for (auto it = mpRenderObjectList.begin(); it != mpRenderObjectList.end(); it++) {
		(*it)->Release();
		delete *it;
		*it = nullptr;
	}
	mpRenderObjectList.clear();
	for (auto it = mpRenderObject2DList.begin(); it != mpRenderObject2DList.end(); it++) {
		(*it)->Release();
		delete *it;
		*it = nullptr;
	}
	mpRenderObject2DList.clear();

	//! unique resource
	SAFE_RELEASE(mpShadowRasterizer);
	SAFE_RELEASE(mpBlendState);
	m2DOverlayRender.Release();
	m2DOverlay.Release();
	mpShadowMapDepth.Release();
	mDepthStencil.Release();
	mBackBuffer.Release();
}
//! @brief 
void uDX11RenderScene_Demo00::init() {
	UINT width, height;
	IWindow->getClientRect(width, height);

	//! create BackBuffer
	mBackBuffer.assignTexture(IDX11Device->getBackBuffer());
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
	if (FAILED(IGetDevice->CreateBlendState(&blendDesc, &mpBlendState))) {
		throw std::runtime_error("error : createBlendState");
	}

	//! create shadowmap resource
	mpShadowMapDepth.createTexture(1024, 1024, cDX11Texture::eTexDescType::Depth);
	mpShadowMapDepth.createDepthStencilView();
	mpShadowMapDepth.createShaderResourceView();
	mpShadowMapDepth.createSamplerState(cDX11Texture::eSamplerType::DepthBorder);

	//! create shadow-map rasterizer
	mpShadowRasterizer = IDX11Device->createRasterizerState(D3D11_CULL_FRONT);

	//! create 2D overlay
	m2DOverlay.createTexture(width, height, cDX11Texture::eTexDescType::Render);
	m2DOverlay.createRenderTargetView();
	m2DOverlay.createShaderResourceView();
	m2DOverlay.createSamplerState();

	//! create shader3D
	mpVShader          = IDX11Device->createVShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "vsMain");
	mpGShader          = IDX11Device->createGShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "gsMain");
	mpPShader          = IDX11Device->createPShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "psMain");
	mpPShaderNoTexture = IDX11Device->createPShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "psMainNoTexture");
	mpPShaderCube      = IDX11Device->createPShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "psMainCube");
	mpVShaderDepth     = IDX11Device->createVShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "vsMainDepth");
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	mpInputLayout = IDX11Device->createInputLayout(layout, _countof(layout), nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "vsMain");

	//! create shadrer2D
	mpVShader2D       = IDX11Device->createVShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "vsMain2D");
	mpPShader2D       = IDX11Device->createPShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "psMain2D");
/*
	D3D11_INPUT_ELEMENT_DESC layout2D[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
*/
	mpInputLayout2D = IDX11Device->createInputLayout(layout, _countof(layout), nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "vsMain2D");

	//! create PerspectiveView(ConstantBuffer)
	mpConstant_Perspective      = IDX11Device->createConstantBuffer(sizeof(nCBStruct::PerspectiveViewMatrix), 1);
	mpConstant_PerspectiveLight = IDX11Device->createConstantBuffer(sizeof(nCBStruct::PerspectiveViewMatrix), 1);
	mpConstant_WorldMatrix      = IDX11Device->createConstantBuffer(sizeof(nCBStruct::WorldMatrix),           1);
	mpConstant_LightProperty    = IDX11Device->createConstantBuffer(sizeof(nCBStruct::LightProperty),         1);
	mpConstant_Material         = IDX11Device->createConstantBuffer(sizeof(nCBStruct::Material),              1);
	mpConstant_AmbientLight     = IDX11Device->createConstantBuffer(sizeof(nCBStruct::AmbientLight),          1);
	mpConstant_DirectionLight   = IDX11Device->createConstantBuffer(sizeof(nCBStruct::DirectionalLight),      1);
	mpConstant_PointLight       = IDX11Device->createConstantBuffer(sizeof(nCBStruct::PointLight),            1);
	mpConstant_ScreenProj       = IDX11Device->createConstantBuffer(sizeof(nCBStruct::ScreenProj),            1);

	DirectX::XMVECTOR camPos = DirectX::XMVectorSet(2.0f, 2.0f, -2.0f, 1.0f);
	DirectX::XMVECTOR camTar = DirectX::XMVectorSet(0.0f, 0.0f,  0.0f, 1.0f);
	DirectX::XMVECTOR camUp  = DirectX::XMVectorSet(0.0f, 1.0f,  0.0f, 1.0f);
	FLOAT aspectRatio = (FLOAT)width / height;
	mPerspective.view = DirectX::XMMatrixLookAtLH(camPos, camTar, camUp);
	mPerspective.proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, aspectRatio, 1.f, 200.0f);
	memcpy(&mLightProperty.eyePosition, camPos.m128_f32, sizeof(mLightProperty.eyePosition));

	//! create Directional Light
	mDirectionLight.position = { 5.0f, 3.0f, -5.0f, 1.0f };
	DirectX::XMVECTOR litPos = DirectX::XMVectorSet( mDirectionLight.position.x, mDirectionLight.position.y, mDirectionLight.position.z, 0.0f);
	DirectX::XMVECTOR litTar = DirectX::XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR litUp  = DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f);
	mPerspectiveLight.view = DirectX::XMMatrixLookAtLH(litPos, litTar, litUp);
	mPerspectiveLight.proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, 1.0f, 1.f, 200.0f);
//	mPerspectiveLight.proj = DirectX::XMMatrixOrthographicLH((float)width, (float)height, 1.0f, 100.f);

	//! settings light
	mPointLight.position    = { 0.0f, 0.0f, 0.0f, 0.0f };
	mPointLight.diffuse     = { 1.0f, 1.0f, 1.0f, 0.0f };
	mPointLight.specular    = { 1.0f, 1.0f, 1.0f, 0.0f };
	mPointLight.shininess   = { 0.8f, 0.4f, 0.2f, 0.0f };
	mAmbientLight.color     = { 0.2f, 0.2f, 0.2f, 1.0f };
	mDirectionLight.color   = { 0.4f, 0.4f, 0.4f, 1.0f };

	//! create 2d overlay projection
	mScreenProj.proj = {
		 2.f / width,  0.f,          0.f, 0.f,
		 0.f,         -2.f / height, 0.f, 0.f,
		 0.f,          0.f,          1.f, 0.f,
		-1.f,          1.f,          0.f, 1.f,
	};

	//! write clear-color Texture
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
		cDX11RenderObject* pObj = new cDX11RenderObject();
		pObj->attach(&mpVShader, nullptr, &mpPShader, &mpInputLayout);
		switch(i){
		case 7:	//!< alpha cube
			pObj->loadFigure(nFigureData::getCube);
			pObj->setTexture(dynamicTex, true);
			pObj->createRasterizerState(D3D11_CULL_NONE);
			pObj->setShadowCasting(false);
			break;
		case 6: //!< cube-mapping
			pObj->loadFigure(nFigureData::getCube);
			pObj->setTexture(dynamicCube, true);
			pObj->attach(&mpPShaderCube);
			break;
		case 5: //!< wireframe cube
			pObj->loadFigure(nFigureData::getCube);
			pObj->createRasterizerState(D3D11_CULL_NONE, D3D11_FILL_WIREFRAME);
			pObj->attach(&mpPShaderNoTexture);
			pObj->setShadowCasting(false);
			break;
		case 4: //!< stanford bunny
			pObj->loadModel(nFile::getResourcePath("resource/mesh/bunny.obj"));
			pObj->attach(&mpPShaderNoTexture);
			break;
		case 3: //!< built with line list cube
			pObj->loadFigure(nFigureData::getWiredCube);
			pObj->attach(&mpPShaderNoTexture);
			pObj->setShadowCasting(false);
			pObj->setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			break;
		default:
			pObj->loadFigure(nFigureData::getCube);
			pObj->createTexture(nFile::getResourcePath("resource/texture/mono.png"));
			pObj->createSamplerState();
		}
		DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
		pObj->updateWorldMatrix(matrix);
		nCBStruct::Material material;
		material.ambient  = { 1.0f, 1.0f, 1.0f, 0.0f };
		material.diffuse  = { 1.0f, 1.0f, 1.0f, 0.0f };
		material.specular = { 1.0f, 1.0f, 1.0f,20.0f };
		pObj->updateMaterial(material);
		mpRenderObjectList.push_back(pObj);
	}

	//! create 2D render object
	float w = (float)width, h = (float)height;
	m2DOverlayRender.attach(&mpVShader2D, nullptr, &mpPShader2D, &mpInputLayout2D);
	m2DOverlayRender.setTexture(m2DOverlay, false);
	m2DOverlayRender.loadFigure(nFigureData::getRectangle);
	m2DOverlayRender.updateWorldMatrix(DirectX::XMMatrixIdentity() * DirectX::XMMatrixScaling(w, h, 1.f));
}
//! @brief 
void uDX11RenderScene_Demo00::update() {
	static float time = 0.0f;
	time += ITime->getDeltaTime();
	int i = 0;
	for (auto obj : mpRenderObjectList) {
		if (i == 0) {
			DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
			matrix *= DirectX::XMMatrixScaling(5.f, 0.15f, 5.f);
			matrix *= DirectX::XMMatrixTranslation(0.f, -0.925f, 0.f);
			obj->updateWorldMatrix(matrix);
		}
		else {
			float theta  = (float)(3.14f * 2) / (mpRenderObjectList.size() - 1) * (i - 1) + time;
			float rotate = (float)(3.14f * 2) / (mpRenderObjectList.size() - 1) * (i - 1);
			float radius = 2.f;
			DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
			if (i == 4) {
				float size = 7.f;
				matrix *= DirectX::XMMatrixScaling(size, size, size);
				matrix *= DirectX::XMMatrixRotationY(theta + rotate);
				matrix *= DirectX::XMMatrixTranslation(sinf(theta)*radius, -1.f, cosf(theta)*radius);
			} else {
				float size = 0.5f;
				matrix *= DirectX::XMMatrixScaling(size, size, size);
				matrix *= DirectX::XMMatrixRotationY(theta + rotate);
				matrix *= DirectX::XMMatrixTranslation(sinf(theta)*radius, 0.0f, cosf(theta)*radius);
			}
			obj->updateWorldMatrix(matrix);
		}
		i++;
	}

}
//! @brief 
void uDX11RenderScene_Demo00::render(ID3D11DeviceContext* context) {
	//! property
	float depthClearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float backGroundColor[] = { 0.0f, 0.125f, 0.6f, 1.0f };
	float overlay2DColor[]  = { 0.0f, 0.0f, 0.0f, 0.0f };
	ID3D11ShaderResourceView *const pEmptySRV[1] = { NULL };
	ID3D11RenderTargetView   *const pEmptyRTV[1] = { NULL };
	ID3D11DepthStencilView   *const pEmptyDSV    = NULL;
	context->ClearRenderTargetView(*mBackBuffer.getRTV(), backGroundColor);
	context->ClearDepthStencilView(*mDepthStencil.getDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//! update resource
	context->UpdateSubresource(mpConstant_Perspective,      0, NULL, &mPerspective,      0, 0);
	context->UpdateSubresource(mpConstant_PerspectiveLight, 0, NULL, &mPerspectiveLight, 0, 0);
	context->UpdateSubresource(mpConstant_LightProperty,    0, NULL, &mLightProperty,    0, 0);
	context->UpdateSubresource(mpConstant_AmbientLight,     0, NULL, &mAmbientLight,     0, 0);
	context->UpdateSubresource(mpConstant_DirectionLight,   0, NULL, &mDirectionLight,   0, 0);
	context->UpdateSubresource(mpConstant_PointLight,       0, NULL, &mPointLight,       0, 0);
	context->UpdateSubresource(mpConstant_ScreenProj,       0, NULL, &mScreenProj,       0, 0);
	context->VSSetConstantBuffers(nCBSlot::PERSPECTIVE,     1, &mpConstant_Perspective);
	context->VSSetConstantBuffers(nCBSlot::LIGHT_PERSPECT,  1, &mpConstant_PerspectiveLight);
	context->PSSetConstantBuffers(nCBSlot::LIGHT_PROPERTY,  1, &mpConstant_LightProperty);
	context->PSSetConstantBuffers(nCBSlot::AMBIENT_LIGNT,   1, &mpConstant_AmbientLight);
	context->PSSetConstantBuffers(nCBSlot::DIRECTION_LIGHT, 1, &mpConstant_DirectionLight);
	context->PSSetConstantBuffers(nCBSlot::POINT_LIGHT,     1, &mpConstant_PointLight);
	context->VSSetConstantBuffers(nCBSlot::SCREEN_PROJ,     1, &mpConstant_ScreenProj);

	//! set Common States
	float blendFactor[4] = {D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
	context->OMSetBlendState(mpBlendState, blendFactor, 0xFFFFFFFF);

	//! unbind resources
	context->PSSetShaderResources(2, 1, pEmptySRV);

	//! prepare viewportRect
	UINT width, height, screenW, screenH;
	IWindow->getClientRect(screenW, screenH);
	mpShadowMapDepth.getTextureSize(width, height);

	//! pre-render depth
	IDX11Device->setViewport(context, width, height);
	context->VSSetShader(mpVShaderDepth,  nullptr, 0);
	context->GSSetShader(nullptr,         nullptr, 0);
	context->PSSetShader(nullptr,         nullptr, 0);
	context->OMSetRenderTargets(1, pEmptyRTV, *mpShadowMapDepth.getDSV());
	context->ClearDepthStencilView(*mpShadowMapDepth.getDSV(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	for (auto obj : mpRenderObjectList) {
		if (obj->isShadowCasting()) {
			updateMaterial   (context, obj->getMaterial());
			updateWolrdMatrix(context, obj->getWorldMatrix());
			obj->render(context, false);
		}
	}

	//! render objects
	IDX11Device->setViewport(context, screenW, screenH);
	context->OMSetRenderTargets(1, mBackBuffer.getRTV(), *mDepthStencil.getDSV());
	context->PSSetShaderResources(2, 1, mpShadowMapDepth.getSRV());
	context->PSSetSamplers(1, 1, mpShadowMapDepth.getSample());
	for (auto obj : mpRenderObjectList) {
		updateMaterial   (context, obj->getMaterial());
		updateWolrdMatrix(context, obj->getWorldMatrix());
		obj->render(context);
	}

	//! render2D
	m2DOverlay.getTextureSize(width, height);
	IDX11Device->setViewport(context, width, height);
	context->VSSetShader(nullptr, nullptr, 0);
	context->GSSetShader(nullptr, nullptr, 0);
	context->PSSetShader(nullptr, nullptr, 0);
	context->ClearRenderTargetView(*m2DOverlay.getRTV(), overlay2DColor);
	context->OMSetRenderTargets(1, m2DOverlay.getRTV(), pEmptyDSV);
	for (auto obj : mpRenderObject2DList) {
		updateWolrdMatrix(context, obj->getWorldMatrix());
		obj->render(context);
	}

	//! render2D output to backBuffer
	context->OMSetRenderTargets(1, mBackBuffer.getRTV(), pEmptyDSV);
	updateWolrdMatrix(context, m2DOverlayRender.getWorldMatrix());
	m2DOverlayRender.render(context);
}

//! @brief 
void uDX11RenderScene_Demo00::updateWolrdMatrix(ID3D11DeviceContext* context, const nCBStruct::WorldMatrix* world) {
	context->UpdateSubresource(mpConstant_WorldMatrix, 0, NULL, world, 0, 0);
	context->VSSetConstantBuffers(nCBSlot::WORLD_MATRIX, 1, &mpConstant_WorldMatrix);
}
//! @brief 
void uDX11RenderScene_Demo00::updateMaterial(ID3D11DeviceContext* context, const nCBStruct::Material* material) {
	context->UpdateSubresource(mpConstant_Material, 0, NULL, material, 0, 0);
	context->PSSetConstantBuffers(nCBSlot::MATERIAL, 1, &mpConstant_Material);
}