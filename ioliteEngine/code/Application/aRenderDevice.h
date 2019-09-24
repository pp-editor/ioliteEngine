#pragma once

//! @class 描画インターフェース
class aRenderDevice {
public:
	aRenderDevice() = default;
	virtual ~aRenderDevice() = default;

public:
	virtual void setup() = 0;
	virtual void shutdown() = 0;
};