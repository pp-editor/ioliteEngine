#pragma once
#pragma comment(lib, "DirectXTex.lib")

#include <windows.h>

#include <d3d12.h>
#include <d3d12shader.h>
#include <dxgi1_4.h>
#include <dxgidebug.h>

#include <d3d11.h>
#include <d3d11shader.h>

#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <DirectXTex/DirectXTex.h>
#include <DDSTextureLoader/DDSTextureLoader.h>
#include <WICTextureLoader/WICTextureLoader.h>

#include <iostream>
#include <string>
#include <list>
#include <chrono>

#define    SINGLETON_DEFINED(className)           \
private:                                          \
    static className* singleton_instance;         \
	className();                                  \
	~className();                                 \
public:                                           \
	static className* getInstance(){              \
		if (singleton_instance == nullptr) {      \
			singleton_instance = new className(); \
		}                                         \
		return singleton_instance;                \
	}                                             \
	static void destroy() {                       \
		delete singleton_instance;                \
	}

#define SINGLETON_INSTANCE(className) className* className::singleton_instance = nullptr
#define SINGLETON_ACCESSOR(className) className::getInstance()

#define SAFE_RELEASE(p) do{ if((p)){ (p)->Release(); (p) = nullptr; }  }while(0)


typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;