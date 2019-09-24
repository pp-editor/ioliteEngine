#pragma once
#include "tDX11ConstantBuffer.h"
#include "cDX11Texture.h"

//! @class 
class cDX11RenderObject {
public:
	cDX11RenderObject();
	~cDX11RenderObject();
	void Release();
	void render(ID3D11DeviceContext* context, bool updateShader = true);

public:
	void updateWorldMatrix(DirectX::XMMATRIX matrix);
	void updateMaterial(Material material);
	void createVertexBuffer(Vertex* data, UINT num);
	void createIndexBuffer (UINT* data, UINT num);
	void createTexture     (const std::string& filename);
	void setTexture        (const cDX11Texture& texture);
	void createRasterizerState(D3D11_CULL_MODE cull = D3D11_CULL_FRONT, D3D11_FILL_MODE fill = D3D11_FILL_SOLID);
	void createSamplerState();

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
	WorldMatrix                mWorldMatrix;
	Material                   mMaterial;
	ID3D11Buffer*              mpVertexBuffer;
	ID3D11Buffer*              mpIndexBuffer;
	UINT                       mIndicesNum;
	cDX11Texture               mTexture;
	ID3D11RasterizerState*     mpRasterizerState;

private:
	ID3D11VertexShader**       mpRef_VSShader;
	ID3D11GeometryShader**     mpRef_GSShader;
	ID3D11PixelShader**        mpRef_PSShader;
	ID3D11InputLayout**        mpRef_InputLayout;
};