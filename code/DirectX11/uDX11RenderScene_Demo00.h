#include "cDX11RenderScene.h"
#include "cDX11RenderObject.h"

class uDX11RenderScene_Demo00 : public cDX11RenderScene {

public:
	uDX11RenderScene_Demo00();
	~uDX11RenderScene_Demo00();

	void Release();
	void init();
	void update();
	void render(ID3D11DeviceContext* context);

private:
	void updateWolrdMatrix(ID3D11DeviceContext* context, const WorldMatrix* world);
	void updateMaterial(ID3D11DeviceContext* context, const Material* material);

private:
	//! renderTarget
	cDX11Texture              mBackBuffer;
	cDX11Texture              mDepthStencil;
	ID3D11BlendState*         mpBlendState;

	//! shadowmap
	cDX11Texture              mpShadowMapDepth;
	ID3D11RasterizerState*    mpShadowRasterizer;

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
	ID3D11InputLayout*        mpInputLayout;

	//! render objects
	std::list<cDX11RenderObject*> mpRenderObjectList;
};