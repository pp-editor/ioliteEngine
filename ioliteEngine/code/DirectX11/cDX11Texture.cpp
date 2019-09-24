#include "Utility/nFile.h"
#include "Utility/nString.h"
#include "cDX11Texture.h"
#include "sDX11Device.h"

#define IDevice	 (IDX11Device->getDevice())
#define IContext (IDX11Device->getContext())

//! -------------------------------------------------------------------------------------
//! Base
//! -------------------------------------------------------------------------------------
//! @brief コンストラクタ
cDX11Texture::cDX11Texture() : 
						 mpTexture           (nullptr)
						,mpRenderTargetView  (nullptr)
						,mpDepthStencilView  (nullptr)
						,mpShaderResourceView(nullptr)
{

}
//! @brief デストラクタ
cDX11Texture::~cDX11Texture() {

}
//! @brief コピーコンストラクタ
cDX11Texture::cDX11Texture(const cDX11Texture& src) {
	mDescTexture            = mDescTexture;
	mDescRenderTargetView   = mDescRenderTargetView;
	mDescDepthStencilView   = mDescDepthStencilView;
	mDescShaderResourceView = mDescShaderResourceView;

	mpTexture            = src.mpTexture;
	mpRenderTargetView   = src.mpRenderTargetView;
	mpDepthStencilView   = src.mpDepthStencilView;
	mpShaderResourceView = src.mpShaderResourceView;
	mpSamplerState       = src.mpSamplerState;
}
//! @brief リソース解放
void cDX11Texture::Release() {
	SAFE_RELEASE(mpSamplerState);
	SAFE_RELEASE(mpShaderResourceView);
	SAFE_RELEASE(mpDepthStencilView);
	SAFE_RELEASE(mpRenderTargetView);
	SAFE_RELEASE(mpTexture);
}

//! -------------------------------------------------------------------------------------
//! Factory
//! -------------------------------------------------------------------------------------
//! @brief 既存テクスチャの割り当て
void cDX11Texture::assignTexture(ID3D11Texture2D* texture) {
	SAFE_RELEASE(mpTexture);
	mpTexture = texture;
}
//! @brief テクスチャファイルのロード
void cDX11Texture::createTexture(const std::string& filename) {
	char _buffer[MAX_PATH];
	strcpy_s(_buffer, filename.c_str());
	char* ext = strstr(_buffer, ".");
	nString::convertToLower(ext);

	enum eLoadType{ WIC, DDS } loadtype;
	if (strcmp(ext, ".dds") == 0) {
		loadtype = eLoadType::DDS;
	} else {
		loadtype = eLoadType::WIC;
	}
	wchar_t filenamew[MAX_PATH*2];
	nString::multiByteCharToWideChar(filename.c_str(), filenamew, filename.length()+1);
	auto pResource = reinterpret_cast<ID3D11Resource*>(mpTexture);
	if (loadtype == WIC) {
		if (FAILED(DirectX::CreateWICTextureFromFile(IDevice, filenamew, &pResource, &mpShaderResourceView))) {
			throw std::runtime_error("error: createWICTextureFromFile");
		}
	} else {
		if (FAILED(DirectX::CreateDDSTextureFromFile(IDevice, filenamew, &pResource, &mpShaderResourceView))) {
			throw std::runtime_error("error: createWICTextureFromFile");
		}
	}
	if (FAILED(pResource->QueryInterface(IID_ID3D11Texture2D, (void**)&mpTexture))) {
		throw std::runtime_error("error: up casting Texture2D");
	}
}
//! @brief 特殊なテクスチャの作成
void cDX11Texture::createTexture(UINT width, UINT height, eTexDescType type) {
	setDesc_TextureResource(width, height, type);
	SAFE_RELEASE(mpTexture);
	if (FAILED(IDevice->CreateTexture2D(&mDescTexture, NULL, &mpTexture))) {
		throw std::runtime_error("error: createTexture2D");
	}
}
//! @brief レンダーターゲットビューの作成
void cDX11Texture::createRenderTargetView(eDescUse descUse) {
	setDesc_RenderTargetView();
	SAFE_RELEASE(mpRenderTargetView);
	auto desc = (descUse == eDescUse::None ? nullptr : &mDescRenderTargetView);
	if (FAILED(IDevice->CreateRenderTargetView(mpTexture, desc, &mpRenderTargetView))) {
		throw std::runtime_error("error: create RenderTargetView");
	}
}
//! @brief デプスステンシルビューの作成
void cDX11Texture::createDepthStencilView(eDescUse descUse) {
	setDesc_DepthStencilView();
	SAFE_RELEASE(mpDepthStencilView);
	auto desc = (descUse == eDescUse::None ? nullptr : &mDescDepthStencilView);
	if (FAILED(IDevice->CreateDepthStencilView(mpTexture, desc, &mpDepthStencilView))) {
		throw std::runtime_error("error: create DepthStencilView");
	}
}
//! @brief シェーダーリソースビューの作成
void cDX11Texture::createShaderResourceView(eDescUse descUse) {
	setDesc_ShaderResourceView();
	SAFE_RELEASE(mpShaderResourceView);
	auto desc = (descUse == eDescUse::None ? nullptr : &mDescShaderResourceView);
	if (FAILED(IDevice->CreateShaderResourceView(mpTexture, desc, &mpShaderResourceView))) {
		throw std::runtime_error("error: create ShaderReosureView");
	}
}
//! @brief サンプラーステートの作成
void cDX11Texture::createSamplerState(eSamplerType type) {
	setDesc_SamplerState(type);
	SAFE_RELEASE(mpSamplerState);
	if (FAILED(IDevice->CreateSamplerState(&mDescSamplerState, &mpSamplerState))) {
		throw std::runtime_error("error: createSamplerState");
	}
}

//! -------------------------------------------------------------------------------------
//! Reference
//! -------------------------------------------------------------------------------------
//! @brief CPU書き込み可能なテクスチャのマップ
void cDX11Texture::map(D3D11_MAPPED_SUBRESOURCE& resource, UINT index) {
	if (FAILED(IContext->Map(mpTexture, D3D11CalcSubresource(0, index, 1), D3D11_MAP_WRITE_DISCARD, 0, &resource))) {
		throw std::runtime_error("error: cDX11Texture::map");
	}
}
//! @brief CPU書き込み可能なテクスチャのアンマップ
void cDX11Texture::unmap(UINT index) {
	IContext->Unmap(mpTexture, D3D11CalcSubresource(0, index, 1));
}
//! @brief テクスチャのコピー
void cDX11Texture::copySubResource(ID3D11Texture2D* pTexture, UINT index, UINT MipLevels) {
	IContext->CopySubresourceRegion(mpTexture, D3D11CalcSubresource(0, index, MipLevels), 0, 0, 0, pTexture, 0, nullptr);
}

//! -------------------------------------------------------------------------------------
//! Reference
//! -------------------------------------------------------------------------------------
//! @brief 深度テクスチャか
bool cDX11Texture::isDepthTexutre() {
	return (getTextureDesc().BindFlags & D3D11_BIND_DEPTH_STENCIL);
}
//! @brief テクスチャがキューブマップか
bool cDX11Texture::isCubeMap() {
	return (getTextureDesc().MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE);
}
//! @brief テクスチャサイズ取得
void cDX11Texture::getTextureSize(UINT& width, UINT &height) {
	D3D11_TEXTURE2D_DESC desc = getTextureDesc();
	width  = desc.Width;
	height = desc.Height;
}
// @brief テクスチャ配列サイズ取得
UINT cDX11Texture::getTextureArraySize() {
	return getTextureDesc().ArraySize;
}

//! -------------------------------------------------------------------------------------
//! Settings
//! -------------------------------------------------------------------------------------
//! @brief テクスチャリソースプロパティの指定
void cDX11Texture::setDesc_TextureResource(UINT width, UINT height, eTexDescType type){
	ZeroMemory(&mDescTexture, sizeof(mDescTexture));
	mDescTexture.Width              = width;
	mDescTexture.Height             = height;
	mDescTexture.MipLevels          = 1;
	mDescTexture.ArraySize          = 1;
	mDescTexture.SampleDesc.Count   = 1;
	mDescTexture.SampleDesc.Quality = 0;
	mDescTexture.Usage              = D3D11_USAGE_DEFAULT;
	mDescTexture.CPUAccessFlags     = 0;
	mDescTexture.MiscFlags          = 0;
	mDescTexture.Format             = DXGI_FORMAT_R32G32B32A32_FLOAT;
	mDescTexture.BindFlags          = D3D11_BIND_SHADER_RESOURCE;

	u32 utype = (u32)type;
	while(utype != 0){
		u32 _type = utype;
		for (UINT i = 0; i < 32; i++) {
			if (_type & (1 << i)) {
				utype  = (utype & (~(1 << i)));
				_type &= (1 << i);
				break;
			}
		}
		switch ((eTexDescType)_type) {
		case eTexDescType::DepthStencil:
			mDescTexture.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
			switch(IDX11Device->getDepthStencilFormat()){
			case DXGI_FORMAT_D16_UNORM:			   mDescTexture.Format = DXGI_FORMAT_R16_TYPELESS;      break;
			case DXGI_FORMAT_D24_UNORM_S8_UINT:	   mDescTexture.Format = DXGI_FORMAT_R24G8_TYPELESS;    break;
			case DXGI_FORMAT_D32_FLOAT:			   mDescTexture.Format = DXGI_FORMAT_R32_TYPELESS;      break;
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: mDescTexture.Format = DXGI_FORMAT_R32G8X24_TYPELESS; break;
			}
			break;
		case eTexDescType::Depth:
			mDescTexture.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
			mDescTexture.Format = DXGI_FORMAT_R32_TYPELESS;
			break;
		case eTexDescType::Render:
			mDescTexture.BindFlags |= D3D11_BIND_RENDER_TARGET;
			break;
		case eTexDescType::CPUdynamic:
			mDescTexture.Usage          = D3D11_USAGE_DYNAMIC;
			mDescTexture.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
			break;
		case eTexDescType::CubeMap:
			mDescTexture.ArraySize = 6;
			mDescTexture.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
			break;
		case eTexDescType::Mipmap:
			mDescTexture.MipLevels = 0;
			mDescTexture.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
			break;
		}
	}
}
//! @brief レンダーターゲットビューリソースプロパティの指定
void cDX11Texture::setDesc_RenderTargetView() {
	ZeroMemory(&mDescRenderTargetView, sizeof(mDescRenderTargetView));
	auto desc = getTextureDesc();
	mDescRenderTargetView.Format = desc.Format;
	mDescRenderTargetView.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; 
	if(isCubeMap()) {
		mDescRenderTargetView.Texture2DArray.FirstArraySlice = 0;
		mDescRenderTargetView.Texture2DArray.ArraySize       = desc.ArraySize;
		mDescRenderTargetView.Texture2DArray.MipSlice        = 0;
		mDescRenderTargetView.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	}
	if (IDX11Device->isMultiSampling()){
		switch (mDescRenderTargetView.ViewDimension) {
		case D3D11_RTV_DIMENSION_TEXTURE2D:      mDescRenderTargetView.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS; break;
		case D3D11_RTV_DIMENSION_TEXTURE2DARRAY: mDescRenderTargetView.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY; break;
		}
	}
}
//! @brief デプスステンシルビューリソースプロパティの指定
void cDX11Texture::setDesc_DepthStencilView() {
	ZeroMemory(&mDescDepthStencilView, sizeof(mDescDepthStencilView));
	switch(getTextureDesc().Format){
	case DXGI_FORMAT_R16_TYPELESS:        mDescDepthStencilView.Format = DXGI_FORMAT_D16_UNORM;			  break;
	case DXGI_FORMAT_R24G8_TYPELESS:      mDescDepthStencilView.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	  break;
	case DXGI_FORMAT_R32_TYPELESS:        mDescDepthStencilView.Format = DXGI_FORMAT_D32_FLOAT;			  break;
	case DXGI_FORMAT_R32G8X24_TYPELESS:   mDescDepthStencilView.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT; break;
	}
	if (IDX11Device->isMultiSampling()) mDescDepthStencilView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	else                                mDescDepthStencilView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
}
//! @brief シェーダーリソースビューリソースプロパティの指定
void cDX11Texture::setDesc_ShaderResourceView() {
	ZeroMemory(&mDescShaderResourceView, sizeof(mDescShaderResourceView));
	auto desc = getTextureDesc();
	mDescShaderResourceView.Format = desc.Format;
	mDescShaderResourceView.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	if (isCubeMap()) {
		mDescShaderResourceView.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		mDescShaderResourceView.TextureCube.MipLevels       = desc.MipLevels;
		mDescShaderResourceView.TextureCube.MostDetailedMip = 0;
	} else {
		mDescShaderResourceView.Texture2D.MipLevels       = desc.MipLevels;
		mDescShaderResourceView.Texture2D.MostDetailedMip = 0;
	}
	if (IDX11Device->isMultiSampling()) {
		switch (mDescShaderResourceView.ViewDimension) {
		case D3D11_SRV_DIMENSION_TEXTURE2D:      mDescShaderResourceView.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS; break;
		case D3D11_SRV_DIMENSION_TEXTURE2DARRAY: mDescShaderResourceView.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY; break;
		}
	}
	switch(mDescShaderResourceView.Format){
	case DXGI_FORMAT_R16_TYPELESS:      mDescShaderResourceView.Format = DXGI_FORMAT_R16_UNORM;                break;
	case DXGI_FORMAT_R24G8_TYPELESS:    mDescShaderResourceView.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;    break;
	case DXGI_FORMAT_R32_TYPELESS:      mDescShaderResourceView.Format = DXGI_FORMAT_R32_FLOAT;                break;
	case DXGI_FORMAT_R32G8X24_TYPELESS: mDescShaderResourceView.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS; break;
	}
}
//! @brief サンプラーステートのプロパティを指定
void cDX11Texture::setDesc_SamplerState(eSamplerType type) {
	ZeroMemory(&mDescSamplerState, sizeof(D3D11_SAMPLER_DESC));

	mDescSamplerState.Filter   = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

	u32 utype = (u32)type;
	while (utype != 0) {
		UINT _type = utype;
		for (UINT i = 0; i < 32; i++) {
			if (_type & (1 << i)) {
				utype = (utype & (~(1 << i)));
				_type &= (1 << i);
				break;
			}
		}
		switch ((eSamplerType)_type) {
		case eSamplerType::Clamp:
			mDescSamplerState.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			mDescSamplerState.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			mDescSamplerState.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			break;
		case eSamplerType::Wrap:
			mDescSamplerState.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			mDescSamplerState.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			mDescSamplerState.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			break;
		case eSamplerType::Border:
			mDescSamplerState.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			mDescSamplerState.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			mDescSamplerState.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			mDescSamplerState.BorderColor[0] = 1.0f;
			mDescSamplerState.BorderColor[1] = 1.0f;
			mDescSamplerState.BorderColor[2] = 1.0f;
			mDescSamplerState.BorderColor[3] = 1.0f;
			break;
		case eSamplerType::Depth:
			mDescSamplerState.MaxAnisotropy  = 1;
			mDescSamplerState.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			mDescSamplerState.MinLOD         = -FLT_MAX;
			mDescSamplerState.MaxLOD         = FLT_MAX;
			break;
		}
	}
}
//! @brief テクスチャのフォーマット取得
D3D11_TEXTURE2D_DESC&& cDX11Texture::getTextureDesc() {
	D3D11_TEXTURE2D_DESC desc;
	mpTexture->GetDesc(&desc);
	return std::move(desc);
}