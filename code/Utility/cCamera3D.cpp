#include "cCamera3D.h"

//! @brief 
cCamera3D::cCamera3D() {

}
//! @brief 
cCamera3D::~cCamera3D() {

}
//! @brief 
void cCamera3D::init() {
	mPos = { 0, 0, 0, 1 };
	mTar = { 0, 0, 1, 1 };
	mUp  = { 0, 1, 0, 1 };
}
//! @brief 
void cCamera3D::update() {

}
//! @brief 
void cCamera3D::rotateX(float angle) {
//	mAngleX += angle;
	mAngleX = min(DirectX::XM_PIDIV2*0.99f, max(-DirectX::XM_PIDIV2*0.99f, mAngleX + angle));
	applyRotateTar();
}
//! @brief 
void cCamera3D::rotateY(float angle) {
	mAngleY += angle;
	applyRotateTar();
}
//! @brief 
void cCamera3D::setPos(float x, float y, float z) {
	mPos.m128_f32[0] = x;
	mPos.m128_f32[1] = y;
	mPos.m128_f32[2] = z;
}
//! @brief 
void cCamera3D::movePos(float x, float y, float z) {
	mPos.m128_f32[0] += x;
	mPos.m128_f32[1] += y;
	mPos.m128_f32[2] += z;
}
//! @brief 
void cCamera3D::movePosLocalAxis(float x, float y, float z) {
	auto qtX = DirectX::XMQuaternionRotationAxis({1, 0, 0, 1}, mAngleX);
	auto qtY = DirectX::XMQuaternionRotationAxis({0, 1, 0, 1}, mAngleY);
	auto qt  = DirectX::XMQuaternionMultiply(qtX, qtY);
	auto add = DirectX::XMVector3Rotate({ x, y, z, 0 }, qt);
	mPos = DirectX::XMVectorAdd(mPos, add);
}
//! @brief 
void cCamera3D::makeLookAtViewLH(DirectX::XMMATRIX& matrix) {
	matrix = DirectX::XMMatrixLookAtLH(mPos, DirectX::XMVectorAdd(mPos, mTar), mUp);
}
//! @brief 
void cCamera3D::applyRotateTar() {
	auto qtX = DirectX::XMQuaternionRotationAxis({1, 0, 0, 1}, mAngleX);
	auto qtY = DirectX::XMQuaternionRotationAxis({0, 1, 0, 1}, mAngleY);
	auto qt  = DirectX::XMQuaternionMultiply(qtX, qtY);
	mTar = DirectX::XMVector3Rotate({ 0, 0, 1, 0 }, qt);
//	std::cout << "tar (" << mTar.m128_f32[0] << ", " << mTar.m128_f32[1] << ", " << mTar.m128_f32[2] << ")" << std::endl;
//	std::cout << "X: " << mAngleX << ", Y: " << mAngleY << std::endl;
}