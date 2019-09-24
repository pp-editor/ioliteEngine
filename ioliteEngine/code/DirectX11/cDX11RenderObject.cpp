#include "sDX11Device.h"
#include "cDX11RenderObject.h"

//! @brief コンストラクタ
cDX11RenderObject::cDX11RenderObject():
	 mWorldMatrix               ()
	,mpVertexBuffer             (nullptr)
	,mpIndexBuffer              (nullptr)
	,mIndicesNum                (0)
	,mTexture                   ()
	,mpRasterizerState          (nullptr)
	,mpRef_VSShader             (nullptr)
	,mpRef_GSShader             (nullptr)
	,mpRef_PSShader             (nullptr)
	,mpRef_InputLayout          (nullptr)
{

}
//! @brief デストラクタ
cDX11RenderObject::~cDX11RenderObject() {
	Release();
}
//! @brief 生成済みリソースの初期化
void cDX11RenderObject::Release() {
	SAFE_RELEASE(mpRasterizerState);
	mTexture.Release();
	SAFE_RELEASE(mpIndexBuffer);
	SAFE_RELEASE(mpVertexBuffer);

	mpRef_VSShader    = nullptr;
	mpRef_GSShader    = nullptr;
	mpRef_PSShader    = nullptr;
	mpRef_InputLayout = nullptr;
}
//! @brief オブジェクトの描画
void cDX11RenderObject::render(ID3D11DeviceContext* context, bool updateShader) {
	if (updateShader) {
		context->VSSetShader(mpRef_VSShader ? *mpRef_VSShader : nullptr, nullptr, 0);
		context->GSSetShader(mpRef_GSShader ? *mpRef_GSShader : nullptr, nullptr, 0);
		context->PSSetShader(mpRef_PSShader ? *mpRef_PSShader : nullptr, nullptr, 0);
//		IDX11Device->updateConstantBuffer();
	}

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetInputLayout(*mpRef_InputLayout);
	context->IASetVertexBuffers(0, 1, &mpVertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(mpIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->RSSetState(mpRasterizerState);
	IDX11Device->updateWolrdMatrix(context, &mWorldMatrix);
	IDX11Device->updateMaterial(context, &mMaterial);
	if (*mTexture.get() && mTexture.isCubeMap()) {
		context->PSSetShaderResources(1, 1, mTexture.getSRV());
	} else {
		context->PSSetShaderResources(0, 1, mTexture.getSRV());
	}
	context->PSSetSamplers(0, 1, mTexture.getSample());
	context->DrawIndexed(mIndicesNum, 0, 0);
}
//! @brief 座標(マトリクス)を更新
void cDX11RenderObject::updateWorldMatrix(DirectX::XMMATRIX matrix) {
	mWorldMatrix.world = matrix;
}
//! @brief マテリアルを更新
void cDX11RenderObject::updateMaterial(Material material) {
	mMaterial = material;
}
//! @brief 頂点バッファを作成
void cDX11RenderObject::createVertexBuffer(Vertex* data, UINT num) {
	mpVertexBuffer = IDX11Device->createVertexBuffer(data, num);
}
//! @brief インデックスバッファを作成
void cDX11RenderObject::createIndexBuffer(UINT* data, UINT num) {
	mpIndexBuffer = IDX11Device->createIndexBuffer(data, num);
	mIndicesNum   = num;
}
//! @brief テクスチャの作成
void cDX11RenderObject::createTexture(const std::string& filename) {
	mTexture.createTexture(filename);
}
//! @brief テクスチャのセット
void cDX11RenderObject::setTexture(const cDX11Texture& texture) {
	mTexture.Release();
	mTexture = texture;
}
//! @brief ラスタライザステートの作成
void cDX11RenderObject::createRasterizerState(D3D11_CULL_MODE cull, D3D11_FILL_MODE fill) {
	mpRasterizerState = IDX11Device->createRasterizerState(cull, fill);
}
//! @brief サンプラーステートの作成
void cDX11RenderObject::createSamplerState() {
	mTexture.createSamplerState();
}