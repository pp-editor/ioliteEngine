#pragma once

namespace nFigureData {
	typedef void (*FigureFunc)(nCBStruct::Vertex*& vertices, UINT& verticesNum, UINT*& indices, UINT& indicesNum);

	//! 3D Figure
	void getQuad(nCBStruct::Vertex*& vertices, UINT& verticesNum, UINT*& indices, UINT& indicesNum);
	void getCube(nCBStruct::Vertex*& vertices, UINT& verticesNum, UINT*& indices, UINT& indicesNum);
	void getWiredCube(nCBStruct::Vertex*& vertices, UINT& verticesNum, UINT*& indices, UINT& indicesNum);

	//! 2DFigure
	void getRectangle(nCBStruct::Vertex*& vertices, UINT& verticesNum, UINT*& indices, UINT& indicesNum);
};