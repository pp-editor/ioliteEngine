#pragma once

enum eCBufferSlot {
	PERSPECTIVE    ,
	WORLD_MATRIX   ,
	MATERIAL       ,
	LIGHT_PERSPECT ,
	LIGHT_PROPERTY ,
	AMBIENT_LIGNT  ,
	DIRECTION_LIGHT,
	POINT_LIGHT    ,
};

struct Vertex {
	DirectX::XMVECTOR position;
	DirectX::XMVECTOR normal;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT2 texcoord;
};
struct PerspectiveViewMatrix {
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX proj;
	PerspectiveViewMatrix() : view(DirectX::XMMatrixIdentity()), proj(DirectX::XMMatrixIdentity()) {}
};
struct WorldMatrix {
	DirectX::XMMATRIX world;
	WorldMatrix() : world(DirectX::XMMatrixIdentity()) {}
};
struct Material {
	DirectX::XMFLOAT4 ambient;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;
	Material() : ambient(1,1,1,1), diffuse(1,1,1,1), specular(1,1,1,1){}
};
struct LightProperty {
	DirectX::XMFLOAT4 eyePosition;
	LightProperty() : eyePosition(0,0,0,1) {}
};
struct AmbientLight {
	DirectX::XMFLOAT4 color;
	AmbientLight() : color(1,1,1,1) {}
};
struct DirectionalLight {
	DirectX::XMFLOAT4 position;
	DirectX::XMFLOAT4 direction;
	DirectX::XMFLOAT4 color;
	DirectionalLight() : position(0,0,0,1), direction(0,0,0,1), color(1,1,1,1) {}
};
struct PointLight {
	DirectX::XMFLOAT4 position;
	DirectX::XMFLOAT4 diffuse;
	DirectX::XMFLOAT4 specular;
	DirectX::XMFLOAT4 shininess;
	PointLight() : position(0,0,0,1), diffuse(1,1,1,1), specular(1,1,1,1), shininess(1,1,1,1) {}
};