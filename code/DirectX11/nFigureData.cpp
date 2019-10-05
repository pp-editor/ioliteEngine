#include "sDX11Device.h"
#include "nFigureData.h"

namespace nFigureData {

	//! SQUAD data
	static nCBStruct::Vertex gQuadVertices[] = {
		{ { -1.0f, +1.0f, +0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ { +1.0f, +1.0f, +0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
		{ { -1.0f, -1.0f, +0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { +1.0f, -1.0f, +0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
	};
	static UINT gQuadIndices[] = { 0, 1, 2, 1, 3, 2 };
	static UINT gRectangleIndices[] = { 0, 2, 1, 1, 2, 3 };

	//! CUBE data
	static nCBStruct::Vertex gCubeVertices[]{
		{ { -1.0f,  1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ {  1.0f,  1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { -1.0f, -1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
		{ {  1.0f, -1.0f, -1.0f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
		{ { -1.0f,  1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { -1.0f, -1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
		{ {  1.0f,  1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ {  1.0f, -1.0f,  1.0f }, {  0.0f,  0.0f,  1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { -1.0f,  1.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { -1.0f,  1.0f, -1.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ { -1.0f, -1.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
		{ { -1.0f, -1.0f, -1.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
		{ {  1.0f,  1.0f,  1.0f }, {  1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ {  1.0f, -1.0f,  1.0f }, {  1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
		{ {  1.0f,  1.0f, -1.0f }, {  1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ {  1.0f, -1.0f, -1.0f }, {  1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
		{ { -1.0f,  1.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
		{ {  1.0f,  1.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
		{ { -1.0f,  1.0f, -1.0f }, {  0.0f,  1.0f,  0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ {  1.0f,  1.0f, -1.0f }, {  0.0f,  1.0f,  0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { -1.0f, -1.0f,  1.0f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ { -1.0f, -1.0f, -1.0f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
		{ {  1.0f, -1.0f,  1.0f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ {  1.0f, -1.0f, -1.0f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
	};
	static UINT gCubeIndices[]{
		0,  1,  2,  3,  2,  1,
		4,  5,  6,  7,  6,  5,
		8,  9, 10, 11, 10,  9,
		12, 13, 14, 15, 14, 13,
		16, 17, 18, 19, 18, 17,
		20, 21, 22, 23, 22, 21,
	};
	static UINT gWiredCubeIndices[]{
		 0,  1,  1,  3,  3,  2,  2,  0,
		 4,  5,  5,  7,  7,  6,  6,  4,
		 8,  9,  9, 11, 11, 10, 10,  8,
		12, 13, 13, 15, 15, 14, 14, 12,
		16, 17, 17, 19, 19, 18, 18, 16,
		20, 21, 21, 23, 23, 22, 22, 20,
	};

	void getQuad(nCBStruct::Vertex*& vertices, UINT& verticesNum, UINT*& indices, UINT& indicesNum) {
		vertices = gQuadVertices;
		indices  = gQuadIndices;
		verticesNum = _countof(gQuadVertices);
		indicesNum  = _countof(gQuadIndices);
	}
	void getCube(nCBStruct::Vertex*& vertices, UINT& verticesNum, UINT*& indices, UINT& indicesNum) {
		vertices = gCubeVertices;
		indices  = gCubeIndices;
		verticesNum = _countof(gCubeVertices);
		indicesNum  = _countof(gCubeIndices);
	}
	void getWiredCube(nCBStruct::Vertex*& vertices, UINT& verticesNum, UINT*& indices, UINT& indicesNum) {
		vertices = gCubeVertices;
		indices  = gWiredCubeIndices;
		verticesNum = _countof(gCubeVertices);
		indicesNum  = _countof(gWiredCubeIndices);
	}
	void getRectangle(nCBStruct::Vertex*& vertices, UINT& verticesNum, UINT*& indices, UINT& indicesNum) {
		vertices = gQuadVertices;
		indices  = gRectangleIndices;
		verticesNum = _countof(gQuadVertices);
		indicesNum  = _countof(gRectangleIndices);
	}
};