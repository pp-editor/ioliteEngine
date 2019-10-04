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
	SAFE_RELEASE(mpVSShaderDepth);
	SAFE_RELEASE(mpPSShaderCube);
	SAFE_RELEASE(mpPSShaderNoTexture);
	SAFE_RELEASE(mpPSShader);
	SAFE_RELEASE(mpGSShader);
	SAFE_RELEASE(mpVSShader);

	//! objects
	for (auto it = mpRenderObjectList.begin(); it != mpRenderObjectList.end(); it++) {
		(*it)->Release();
		delete *it;
		*it = nullptr;
	}
	mpRenderObjectList.clear();

	//! unique resource
	SAFE_RELEASE(mpShadowRasterizer);
	SAFE_RELEASE(mpBlendState);
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

	//! create shader
	mpVSShader          = IDX11Device->createVSShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "vsMain");
	mpGSShader          = IDX11Device->createGSShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "gsMain");
	mpPSShader          = IDX11Device->createPSShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "psMain");
	mpPSShaderNoTexture = IDX11Device->createPSShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "psMainNoTexture");
	mpPSShaderCube      = IDX11Device->createPSShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "psMainCube");
	mpVSShaderDepth     = IDX11Device->createVSShader(nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "vsMainDepth");

	//! create inputLayout
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	mpInputLayout = IDX11Device->createInputLayout(layout, _countof(layout), nFile::getResourcePath("hlsl/DefaultPipeline.hlsl"), "vsMain");

	//! create PerspectiveView(ConstantBuffer)
	mpConstant_Perspective      = IDX11Device->createConstantBuffer(sizeof(PerspectiveViewMatrix), 1);
	mpConstant_PerspectiveLight = IDX11Device->createConstantBuffer(sizeof(PerspectiveViewMatrix), 1);
	mpConstant_WorldMatrix      = IDX11Device->createConstantBuffer(sizeof(WorldMatrix),           1);
	mpConstant_LightProperty    = IDX11Device->createConstantBuffer(sizeof(LightProperty),         1);
	mpConstant_Material         = IDX11Device->createConstantBuffer(sizeof(Material),              1);
	mpConstant_AmbientLight     = IDX11Device->createConstantBuffer(sizeof(AmbientLight),          1);
	mpConstant_DirectionLight   = IDX11Device->createConstantBuffer(sizeof(DirectionalLight),      1);
	mpConstant_PointLight       = IDX11Device->createConstantBuffer(sizeof(PointLight),            1);

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

	enum class eFigureType : int { Cube, WiredCube, Quad };
	auto loadCube = [](cDX11RenderObject* target, eFigureType type) {
		Vertex* vertices          = nullptr;
		UINT vNum, iNum, *indices = nullptr;
		switch (type) {
		case eFigureType::Cube:      nFigureData::getCube(vertices, vNum, indices, iNum);      break;
		case eFigureType::WiredCube: nFigureData::getWiredCube(vertices, vNum, indices, iNum); break;
		case eFigureType::Quad:      nFigureData::getQuad(vertices, vNum, indices, iNum);	   break;
		}
		target->createVertexBuffer(vertices, vNum);
		target->createIndexBuffer(indices, iNum);
	};
	auto loadBunny = [](cDX11RenderObject* target) {
		std::vector<Vertex> vertices;
		std::vector<UINT>   indices;
		nModelLoader::loadObj(nFile::getResourcePath("resource/mesh/bunny.obj"), vertices, indices);
		target->createVertexBuffer(vertices.data(), (UINT)vertices.size());
		target->createIndexBuffer(indices.data(), (UINT)indices.size());
	};

	//! create renderObject
	for (int i = 0; i < 8; i++) {
		cDX11RenderObject* pObj = new cDX11RenderObject();
		pObj->attach(&mpVSShader, nullptr, &mpPSShader, &mpInputLayout);
		switch(i){
		case 7:	//!< alpha cube
			loadCube(pObj, eFigureType::Cube);
			pObj->setTexture(dynamicTex);
			pObj->createRasterizerState(D3D11_CULL_NONE);
			pObj->setShadowCasting(false);
			break;
		case 6: //!< cube-mapping
			loadCube(pObj, eFigureType::Cube);
			pObj->setTexture(dynamicCube);
			pObj->attach(&mpPSShaderCube);
			break;
		case 5: //!< wireframe cube
			loadCube(pObj, eFigureType::Cube);
			pObj->createRasterizerState(D3D11_CULL_NONE, D3D11_FILL_WIREFRAME);
			pObj->attach(&mpPSShaderNoTexture);
			pObj->setShadowCasting(false);
			break;
		case 4: //!< stanford bunny
			loadBunny(pObj);
			pObj->attach(&mpPSShaderNoTexture);
			break;
		case 3: //!< built with line list cube
			loadCube(pObj, eFigureType::WiredCube);
			pObj->attach(&mpPSShaderNoTexture);
			pObj->setShadowCasting(false);
			pObj->setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			break;
		default:
			loadCube(pObj, eFigureType::Cube);
			pObj->createTexture(nFile::getResourcePath("resource/texture/mono.png"));
			pObj->createSamplerState();
		}
		DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
		pObj->updateWorldMatrix(matrix);
		Material material;
		material.ambient  = { 1.0f, 1.0f, 1.0f, 0.0f };
		material.diffuse  = { 1.0f, 1.0f, 1.0f, 0.0f };
		material.specular = { 1.0f, 1.0f, 1.0f,20.0f };
		pObj->updateMaterial(material);
		mpRenderObjectList.push_back(pObj);
	}
}
//! @brief 
void uDX11RenderScene_Demo00::update() {
	static float time = 0.0f;
	time += ITime->getDeltaTime();
	int i = 0;
	for (auto obj : mpRenderObjectList) {
		if (i == 0) {
			DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
			matrix *= DirectX::XMMatrixScaling(10.f, 0.3f, 10.f);
			matrix *= DirectX::XMMatrixTranslation(0.f, -1.f, 0.f);
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
	float depthClearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float backGroundColor[] = { 0.0f, 0.125f, 0.6f, 1.0f };
	context->ClearRenderTargetView(*mBackBuffer.getRTV(), backGroundColor);
	context->ClearDepthStencilView(*mDepthStencil.getDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//! update resource
	context->UpdateSubresource(mpConstant_Perspective,      0, NULL, &mPerspective,      0, 0);
	context->UpdateSubresource(mpConstant_PerspectiveLight, 0, NULL, &mPerspectiveLight, 0, 0);
	context->UpdateSubresource(mpConstant_LightProperty,    0, NULL, &mLightProperty,    0, 0);
	context->UpdateSubresource(mpConstant_AmbientLight,     0, NULL, &mAmbientLight,     0, 0);
	context->UpdateSubresource(mpConstant_DirectionLight,   0, NULL, &mDirectionLight,   0, 0);
	context->UpdateSubresource(mpConstant_PointLight,       0, NULL, &mPointLight,       0, 0);
	context->VSSetConstantBuffers(eCBufferSlot::PERSPECTIVE,     1, &mpConstant_Perspective);
	context->VSSetConstantBuffers(eCBufferSlot::LIGHT_PERSPECT,  1, &mpConstant_PerspectiveLight);
	context->PSSetConstantBuffers(eCBufferSlot::LIGHT_PROPERTY,  1, &mpConstant_LightProperty);
	context->PSSetConstantBuffers(eCBufferSlot::AMBIENT_LIGNT,   1, &mpConstant_AmbientLight);
	context->PSSetConstantBuffers(eCBufferSlot::DIRECTION_LIGHT, 1, &mpConstant_DirectionLight);
	context->PSSetConstantBuffers(eCBufferSlot::POINT_LIGHT,     1, &mpConstant_PointLight);

	//! set Blendstate
	float blendFactor[4] = {D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
	context->OMSetBlendState(mpBlendState, blendFactor, 0xFFFFFFFF);

	//! unbind resources
	ID3D11ShaderResourceView *const pSRV[1] = { NULL };
	context->PSSetShaderResources(2, 1, pSRV);

	//! prepare viewportRect
	UINT width, height, screenW, screenH;
	IWindow->getClientRect(screenW, screenH);
	mpShadowMapDepth.getTextureSize(width, height);

	//! pre-render depth
	IDX11Device->setViewport(context, width, height);
	context->VSSetShader(mpVSShaderDepth, nullptr, 0);
	context->GSSetShader(nullptr,         nullptr, 0);
	context->PSSetShader(nullptr,         nullptr, 0);
	ID3D11RenderTargetView *const pRTV[1] = { NULL };
	context->OMSetRenderTargets(1, pRTV, *mpShadowMapDepth.getDSV());
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
}

//! @brief 
void uDX11RenderScene_Demo00::updateWolrdMatrix(ID3D11DeviceContext* context, const WorldMatrix* world) {
	context->UpdateSubresource(mpConstant_WorldMatrix, 0, NULL, world, 0, 0);
	context->VSSetConstantBuffers(eCBufferSlot::WORLD_MATRIX, 1, &mpConstant_WorldMatrix);
}
//! @brief 
void uDX11RenderScene_Demo00::updateMaterial(ID3D11DeviceContext* context, const Material* material) {
	context->UpdateSubresource(mpConstant_Material, 0, NULL, material, 0, 0);
	context->PSSetConstantBuffers(eCBufferSlot::MATERIAL, 1, &mpConstant_Material);
}