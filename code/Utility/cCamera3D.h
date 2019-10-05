#pragma once

//! @class 
class cCamera3D {
public:
	cCamera3D();
	~cCamera3D();

	void init  ();
	void update();
	void rotateX(float angle);
	void rotateY(float angle);
	void setPos (float x, float y, float z);
	void movePos(float x, float y, float z);
	void movePosLocalAxis(float x, float y, float z);
	void makeLookAtViewLH(DirectX::XMMATRIX& matrix);

	const DirectX::XMVECTOR& getPos(){ return mPos; }
	const DirectX::XMVECTOR& getTar(){ return mTar; }
	const DirectX::XMVECTOR& getUp (){ return mUp;	}

private:
	void applyRotateTar();

private:
	float             mAngleX;
	float             mAngleY;
	DirectX::XMVECTOR mPos;
	DirectX::XMVECTOR mTar;
	DirectX::XMVECTOR mUp;
};