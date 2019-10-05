#include "sDX11Device.h"
#include "cDX11RenderObject.h"
#include "Utility/nModelLoader.h"

//! @brief コンストラクタ
cDX11RenderObject::cDX11RenderObject():
	 mWorldMatrix               ()
	,mpVertexBuffer             (nullptr)
	,mpIndexBuffer              (nullptr)
	,mIndicesNum                (0)
	,mTexture                   ()
	,mpRasterizerState          (nullptr)
	,mPrimitiveTopology         (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
	,mShadowCasting             (true)
	,mHasTextureOwnerShip       (true)
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
	if (mHasTextureOwnerShip) {
		mTexture.Release();
	} else {
		mTexture = {};
	}
	SAFE_RELEASE(mpIndexBuffer);
	SAFE_RELEASE(mpVertexBuffer);

	mpRef_VSShader    = nullptr;
	mpRef_GSShader    = nullptr;
	mpRef_PSShader    = nullptr;
	mpRef_InputLayout = nullptr;
}
//! @brief オブジェクトの描画
void cDX11RenderObject::render(ID3D11DeviceContext* context, bool updateShader, bool updateRasterizer) {
	if (updateShader) {
		context->VSSetShader(mpRef_VSShader ? *mpRef_VSShader : nullptr, nullptr, 0);
		context->GSSetShader(mpRef_GSShader ? *mpRef_GSShader : nullptr, nullptr, 0);
		context->PSSetShader(mpRef_PSShader ? *mpRef_PSShader : nullptr, nullptr, 0);
	}

	UINT stride = sizeof(nCBStruct::Vertex);
	UINT offset = 0;
	context->IASetInputLayout(*mpRef_InputLayout);
	context->IASetPrimitiveTopology(mPrimitiveTopology);
	context->IASetVertexBuffers(0, 1, &mpVertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(mpIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	if (updateRasterizer) {
		context->RSSetState(mpRasterizerState);
	}
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
void cDX11RenderObject::updateMaterial(nCBStruct::Material material) {
	mMaterial = material;
}
//! @brief 頂点バッファを作成
void cDX11RenderObject::createVertexBuffer(nCBStruct::Vertex* data, UINT num) {
	mpVertexBuffer = IDX11Device->createVertexBuffer(data, num);
}
//! @brief インデックスバッファを作成
void cDX11RenderObject::createIndexBuffer(UINT* data, UINT num) {
	mpIndexBuffer = IDX11Device->createIndexBuffer(data, num);
	mIndicesNum   = num;
}
//! @brief モデルデータのロード
void cDX11RenderObject::loadModel(const std::string& filename) {
	std::vector<nCBStruct::Vertex> vertices;
	std::vector<UINT>   indices;
	if (strstr(filename.c_str(), ".obj") != NULL) {
		nModelLoader::loadObj(filename, vertices, indices);
	} else {
		throw std::runtime_error("error: cDX11RenderObject::loadModel(not supported format)");
	}
	createVertexBuffer(vertices.data(), (UINT)vertices.size());
	createIndexBuffer(indices.data(), (UINT)indices.size());
}
//! @brief 図形モデルのロード
void cDX11RenderObject::loadFigure(nFigureData::FigureFunc func) {
	nCBStruct::Vertex* vertices = nullptr;
	UINT vNum = 0, iNum = 0, *indices = nullptr;
	func(vertices, vNum, indices, iNum);
	createVertexBuffer(vertices, vNum);
	createIndexBuffer(indices, iNum);
}
//! @brief テクスチャの作成
void cDX11RenderObject::createTexture(const std::string& filename) {
	mTexture.createTexture(filename);
	mHasTextureOwnerShip = true;
}
//! @brief テクスチャのセット
void cDX11RenderObject::setTexture(const cDX11Texture& texture, bool isPassTextureOwnership) {
	if (mHasTextureOwnerShip) {
		mTexture.Release();
	}
	mTexture = texture;
	mHasTextureOwnerShip = isPassTextureOwnership;
}
//! @brief ラスタライザステートの作成
void cDX11RenderObject::createRasterizerState(D3D11_CULL_MODE cull, D3D11_FILL_MODE fill) {
	mpRasterizerState = IDX11Device->createRasterizerState(cull, fill);
}
//! @brief サンプラーステートの作成
void cDX11RenderObject::createSamplerState() {
	mTexture.createSamplerState();
}
//! @brief 描画方法の指定
void cDX11RenderObject::setPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology) {
	mPrimitiveTopology = topology;
}
//! @brief 影を落とすかどうか
void cDX11RenderObject::setShadowCasting(bool doShadowCasting) {
	mShadowCasting = doShadowCasting;
}

//! @brief カリング方向の取得
D3D11_CULL_MODE cDX11RenderObject::getCullingMode() {
	if (mpRasterizerState == nullptr) {
		return D3D11_CULL_BACK;
	}
	D3D11_RASTERIZER_DESC desc;
	mpRasterizerState->GetDesc(&desc);
	return desc.CullMode;
}