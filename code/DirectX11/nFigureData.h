#pragma once

namespace nFigureData {
	void getQuad(Vertex*& vertices, UINT& verticesNum, UINT*& indices, UINT& indicesNum);
	void getCube(Vertex*& vertices, UINT& verticesNum, UINT*& indices, UINT& indicesNum);
	void getWiredCube(Vertex*& vertices, UINT& verticesNum, UINT*& indices, UINT& indicesNum);
};