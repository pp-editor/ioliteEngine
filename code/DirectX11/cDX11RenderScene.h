#pragma once

class cDX11RenderScene {
public:
	cDX11RenderScene() = default;
	virtual ~cDX11RenderScene(){}

	virtual void Release()= 0;
	virtual void init()   = 0;
	virtual void update() = 0;
	virtual void render(ID3D11DeviceContext* context) = 0;
};