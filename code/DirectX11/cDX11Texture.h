#pragma once

//! @class 
class cDX11Texture {
public:
	cDX11Texture();
	~cDX11Texture();
	cDX11Texture(const cDX11Texture& src);
	void Release();

public:
	//! factory
	enum class eDescUse { Set, None };
	enum class eTexDescType : u32 { 
		Default       = 0,
		Render        = 1 << 1,
		DepthStencil  = 1 << 2,
		CPUdynamic    = 1 << 3,
		CubeMap       = 1 << 4,
		Mipmap        = 1 << 5,
		Depth         = 1 << 6,

		RenderDepth       = Depth  |Render,
		RenderCubeMap     = CubeMap|Render,
	};
	void assignTexture(ID3D11Texture2D* texture);
	void createTexture(const std::string& filename);
	void createTexture(UINT width, UINT height, eTexDescType type = eTexDescType::Default);
	void createRenderTargetView  (eDescUse descUse = eDescUse::Set);
	void createDepthStencilView  (eDescUse descUse = eDescUse::Set);
	void createShaderResourceView(eDescUse descUse = eDescUse::Set);

	enum class eSamplerType : u32 {
		Clamp   = 1 << 0,
		Wrap    = 1 << 1,
		Border  = 1 << 2,
		Depth   = 1 << 3,

		DepthClamp  = Depth|Clamp,
		DepthWrap   = Depth|Wrap,
		DepthBorder = Depth|Border,
		Default = Clamp,
	};
	void createSamplerState(eSamplerType type = eSamplerType::Default);

	//! control
	void map(D3D11_MAPPED_SUBRESOURCE& resource, UINT index = 0);
	void unmap(UINT index = 0);
	void copySubResource(ID3D11Texture2D* pTexture, UINT index = 0, UINT MipLevels = 1);

	//! reference
	bool isDepthTexutre();
	bool isCubeMap();
	void getTextureSize(UINT& width, UINT &height);
	UINT getTextureArraySize();
	bool isTextureCreated(){ return mpTexture != nullptr; }
	ID3D11Texture2D**          get()    { return &mpTexture; }
	ID3D11RenderTargetView**   getRTV() { return &mpRenderTargetView; }
	ID3D11DepthStencilView**   getDSV() { return &mpDepthStencilView; }
	ID3D11ShaderResourceView** getSRV() { return &mpShaderResourceView; }
	ID3D11SamplerState**       getSample(){ return &mpSamplerState; }

private:
	//! preparation resource desc
	void setDesc_TextureResource(UINT width, UINT height, eTexDescType type);
	void setDesc_RenderTargetView();
	void setDesc_DepthStencilView();
	void setDesc_ShaderResourceView();
	void setDesc_SamplerState(eSamplerType type = eSamplerType::Default);
	D3D11_TEXTURE2D_DESC&& getTextureDesc();

private:
	D3D11_TEXTURE2D_DESC             mDescTexture;
	D3D11_RENDER_TARGET_VIEW_DESC    mDescRenderTargetView;
	D3D11_DEPTH_STENCIL_VIEW_DESC	 mDescDepthStencilView;
	D3D11_SHADER_RESOURCE_VIEW_DESC	 mDescShaderResourceView;
	D3D11_SAMPLER_DESC               mDescSamplerState;

	ID3D11Texture2D*                 mpTexture;
	ID3D11RenderTargetView*	         mpRenderTargetView;
	ID3D11DepthStencilView*          mpDepthStencilView;
	ID3D11ShaderResourceView*        mpShaderResourceView;
	ID3D11SamplerState*              mpSamplerState;
};