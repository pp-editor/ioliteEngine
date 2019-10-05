#pragma once
#include "tDX11ConstantBuffer.h"
#include "cDX11Texture.h"
#include "nFigureData.h"

//! @class 
class cDX11RenderObject {
public:
	cDX11RenderObject();
	virtual ~cDX11RenderObject();
	void Release();
	void render(ID3D11DeviceContext* context, bool updateShader = true, bool updateRasterizer = true);

public:
	void updateWorldMatrix (DirectX::XMMATRIX matrix);
	void updateMaterial    (nCBStruct::Material material);
	void createVertexBuffer(nCBStruct::Vertex* data, UINT num);
	void createIndexBuffer (UINT* data, UINT num);
	void loadModel         (const std::string& filename);
	void loadFigure        (nFigureData::FigureFunc func);
	void createTexture     (const std::string& filename);
	void setTexture        (const cDX11Texture& texture, bool isPassOwnership);
	void createRasterizerState(D3D11_CULL_MODE cull = D3D11_CULL_BACK, D3D11_FILL_MODE fill = D3D11_FILL_SOLID);
	void createSamplerState();
	void setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	void setShadowCasting(bool doShadowCasting);

private:
	D3D11_CULL_MODE getCullingMode();
public:
	nCBStruct::WorldMatrix* getWorldMatrix() { return &mWorldMatrix; }
	nCBStruct::Material*    getMaterial   () { return &mMaterial; }
	bool                    isCullingBack()  { return getCullingMode() == D3D11_CULL_BACK; }
	bool                    isCullingFront() { return getCullingMode() == D3D11_CULL_FRONT; }
	bool                    isCullingNone()  { return getCullingMode() == D3D11_CULL_NONE; }
	bool                    isShadowCasting(){ return mShadowCasting; }

public:
	void attach(ID3D11VertexShader**   vs)    { mpRef_VSShader    = vs; }
	void attach(ID3D11GeometryShader** gs)    { mpRef_GSShader    = gs; }
	void attach(ID3D11PixelShader**    ps)    { mpRef_PSShader    = ps; }
	void attach(ID3D11InputLayout**    layout){ mpRef_InputLayout = layout; }
	void attach(ID3D11VertexShader**   vs,
	            ID3D11GeometryShader** gs,
	            ID3D11PixelShader**    ps,
	            ID3D11InputLayout**    layout
	){
		attach(vs);
		attach(ps);
		attach(gs);
		attach(layout);
	}

private:
	nCBStruct::WorldMatrix     mWorldMatrix;
	nCBStruct::Material        mMaterial;
	ID3D11Buffer*              mpVertexBuffer;
	ID3D11Buffer*              mpIndexBuffer;
	UINT                       mIndicesNum;
	cDX11Texture               mTexture;
	ID3D11RasterizerState*     mpRasterizerState;
	D3D11_PRIMITIVE_TOPOLOGY   mPrimitiveTopology;
	bool                       mShadowCasting;
	bool                       mHasTextureOwnerShip;

private:
	ID3D11VertexShader**       mpRef_VSShader;
	ID3D11GeometryShader**     mpRef_GSShader;
	ID3D11PixelShader**        mpRef_PSShader;
	ID3D11InputLayout**        mpRef_InputLayout;
};