#pragma once
#include "Application/aRenderDevice.h"
#include "tDX11ConstantBuffer.h"
#include "cDX11Texture.h"
#include "cDX11RenderObject.h"

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
	void updateWolrdMatrix(ID3D11DeviceContext* context, const WorldMatrix* world);
	void updateMaterial   (ID3D11DeviceContext* context, const Material* material);
	void updateConstantBuffer();

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
	ID3DBlob*              createShaderBlob       (eShaderType type, const std::string& filename, const std::string& entrypoint);
	ID3D11VertexShader*    createVSShader         (const std::string& filename, const std::string& entrypoint);
	ID3D11PixelShader*     createPSShader         (const std::string& filename, const std::string& entrypoint);
	ID3D11GeometryShader*  createGSShader         (const std::string& filename, const std::string& entrypoint);
	ID3D11InputLayout*     createInputLayout      (D3D11_INPUT_ELEMENT_DESC* layout, UINT num, const std::string& filename, const std::string& entrypoint);
	ID3D11Buffer*          createBuffer           (eBufferType type, void* data, UINT type_size, UINT num);
	ID3D11Buffer*          createVertexBuffer     (Vertex* data, UINT num);
	ID3D11Buffer*          createIndexBuffer      (UINT* data, UINT num);
	ID3D11Buffer*          createConstantBuffer   (size_t size, UINT num);
	ID3D11RasterizerState* createRasterizerState  (D3D11_CULL_MODE cull = D3D11_CULL_FRONT, D3D11_FILL_MODE fill = D3D11_FILL_SOLID);
	void                   setViewport            (ID3D11DeviceContext* pContext, UINT width, UINT height);

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
	cDX11Texture              mBackBuffer;
	cDX11Texture              mDepthStencil;
	ID3D11BlendState*         mpBlendState;

	//! shadowmap
	cDX11Texture              mpShadowMapTexture;
	cDX11Texture              mpShadowMapDepth;

	//! cbufffer resource
	ID3D11Buffer*             mpConstant_Perspective;
	ID3D11Buffer*             mpConstant_PerspectiveLight;
	ID3D11Buffer*             mpConstant_WorldMatrix;
	ID3D11Buffer*             mpConstant_Material;
	ID3D11Buffer*             mpConstant_LightProperty;
	ID3D11Buffer*             mpConstant_AmbientLight;
	ID3D11Buffer*             mpConstant_DirectionLight;
	ID3D11Buffer*             mpConstant_PointLight;

	//! cbuffer value
	PerspectiveViewMatrix     mPerspective;
	PerspectiveViewMatrix     mPerspectiveLight;
	LightProperty             mLightProperty;
	AmbientLight              mAmbientLight;
	DirectionalLight          mDirectionLight;
	PointLight                mPointLight;

	//! layout
	ID3D11VertexShader*       mpVSShader;
	ID3D11GeometryShader*     mpGSShader;
	ID3D11PixelShader*        mpPSShader;
	ID3D11PixelShader*        mpPSShaderNoTexture;
	ID3D11PixelShader*        mpPSShaderCube;
	ID3D11VertexShader*       mpVSShaderDepth;
	ID3D11PixelShader*        mpPSShaderDepth;
	ID3D11InputLayout*        mpInputLayout;

	//! objects
	std::list<cDX11RenderObject*> mpRenderObjectList;
};

//! accessor
#define IDX11Device SINGLETON_ACCESSOR(cDX11Device)