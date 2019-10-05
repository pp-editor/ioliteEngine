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
	void updateWolrdMatrix(ID3D11DeviceContext* context, const nCBStruct::WorldMatrix* world);
	void updateMaterial   (ID3D11DeviceContext* context, const nCBStruct::Material* material);

private:
	//! renderTarget
	cDX11Texture              mBackBuffer;
	cDX11Texture              mDepthStencil;
	ID3D11BlendState*         mpBlendState;

	//! shadowmap
	cDX11Texture              mpShadowMapDepth;
	ID3D11RasterizerState*    mpShadowRasterizer;

	//! 2D Overlay
	cDX11Texture              m2DOverlay;
	cDX11RenderObject         m2DOverlayRender;


	//! cbufffer resource
	ID3D11Buffer*             mpConstant_Perspective;
	ID3D11Buffer*             mpConstant_PerspectiveLight;
	ID3D11Buffer*             mpConstant_WorldMatrix;
	ID3D11Buffer*             mpConstant_Material;
	ID3D11Buffer*             mpConstant_LightProperty;
	ID3D11Buffer*             mpConstant_AmbientLight;
	ID3D11Buffer*             mpConstant_DirectionLight;
	ID3D11Buffer*             mpConstant_PointLight;
	ID3D11Buffer*             mpConstant_ScreenProj;

	//! cbuffer value
	nCBStruct::PerspectiveViewMatrix     mPerspective;
	nCBStruct::PerspectiveViewMatrix     mPerspectiveLight;
	nCBStruct::LightProperty             mLightProperty;
	nCBStruct::AmbientLight              mAmbientLight;
	nCBStruct::DirectionalLight          mDirectionLight;
	nCBStruct::PointLight                mPointLight;
	nCBStruct::ScreenProj                mScreenProj;

	//! layout
	ID3D11VertexShader*       mpVShader;
	ID3D11GeometryShader*     mpGShader;
	ID3D11PixelShader*        mpPShader;
	ID3D11PixelShader*        mpPShaderNoTexture;
	ID3D11PixelShader*        mpPShaderCube;
	ID3D11VertexShader*       mpVShaderDepth;
	ID3D11InputLayout*        mpInputLayout;

	//! layout 2D
	ID3D11VertexShader*       mpVShader2D;
	ID3D11PixelShader*        mpPShader2D;
	ID3D11InputLayout*        mpInputLayout2D;

	//! render objects
	std::list<cDX11RenderObject*> mpRenderObjectList;
	std::list<cDX11RenderObject*> mpRenderObject2DList;
};