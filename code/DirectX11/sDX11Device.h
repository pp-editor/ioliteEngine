#pragma once
#include "Application/aRenderDevice.h"
#include "tDX11ConstantBuffer.h"
#include "cDX11Texture.h"
#include "cDX11RenderObject.h"
#include "cDX11RenderScene.h"

//! @class 
class cDX11Device : public aRenderDevice{
	SINGLETON_DEFINED(cDX11Device)

//! -------------------------------------------------------------------------------------
//! called by Engine
//! -------------------------------------------------------------------------------------
public:
	void setup();
	void shutdown();
	void render();

//! -------------------------------------------------------------------------------------
//! Render Helper
//! -------------------------------------------------------------------------------------
public:
	ID3D11Device*        getDevice(){ return mpDevice; }
	ID3D11DeviceContext* getContext(){ return mpContext; }
	bool isMultiSampling() { return mDepthStencilSampleCount > 0; }
	DXGI_FORMAT getDepthStencilFormat(){ return mDepthStencilFormat; }

//! -------------------------------------------------------------------------------------
//! called by InnerFunction
//! -------------------------------------------------------------------------------------
private:
	void createDevice();

//! -------------------------------------------------------------------------------------
//! DirectX Resource Helper
//! -------------------------------------------------------------------------------------
public:
	enum eShaderType    { vs, gs, ps, layout };
	enum eBufferType    { vertex, index, constant };
	ID3DBlob*              createShaderBlob      (eShaderType type, const std::string& filename, const std::string& entrypoint);
	ID3D11VertexShader*    createVShader         (const std::string& filename, const std::string& entrypoint);
	ID3D11PixelShader*     createPShader         (const std::string& filename, const std::string& entrypoint);
	ID3D11GeometryShader*  createGShader         (const std::string& filename, const std::string& entrypoint);
	ID3D11InputLayout*     createInputLayout     (D3D11_INPUT_ELEMENT_DESC* layout, UINT num, const std::string& filename, const std::string& entrypoint);
	ID3D11Buffer*          createBuffer          (eBufferType type, void* data, UINT type_size, UINT num);
	ID3D11Buffer*          createVertexBuffer    (nCBStruct::Vertex* data, UINT num);
	ID3D11Buffer*          createIndexBuffer     (UINT* data, UINT num);
	ID3D11Buffer*          createConstantBuffer  (size_t size, UINT num);
	ID3D11RasterizerState* createRasterizerState (D3D11_CULL_MODE cull = D3D11_CULL_BACK, D3D11_FILL_MODE fill = D3D11_FILL_SOLID);
	void                   setViewport           (ID3D11DeviceContext* pContext, UINT width, UINT height);
	ID3D11Texture2D*       getBackBuffer         ();

//! -------------------------------------------------------------------------------------
//! Member Variables
//! -------------------------------------------------------------------------------------
private:
	DXGI_FORMAT               mDepthStencilFormat;
	UINT                      mDepthStencilSampleCount;

	D3D_FEATURE_LEVEL         mFeatureLevel;
	ID3D11Device*             mpDevice;
	IDXGISwapChain*           mpSwapchain;
	ID3D11DeviceContext*      mpContext;

	cDX11RenderScene*         mpRenderScene;
};

//! accessor
#define IDX11Device SINGLETON_ACCESSOR(cDX11Device)