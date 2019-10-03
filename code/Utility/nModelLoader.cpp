#include <fstream>
#include "DirectX11/tDX11ConstantBuffer.h"
#include "nModelLoader.h"

namespace nModelLoader {
	void loadObj(std::string filename, std::vector<Vertex>& vertices, std::vector<UINT>& indices) {
		std::ifstream ifs;
		ifs.open(filename, std::ios::in);

		std::string line;
		enum class eMode : int { none, v, f };
		eMode mode;

		int countV = 0;
		int countF = 0;
		while (!ifs.eof()) {
			mode = eMode::none;
			std::getline(ifs, line);
			if (line[0] == '#') continue;
			if (line[0] == 'v' && line[1] == ' ') mode = eMode::v;
			if (line[0] == 'f' && line[1] == ' ') mode = eMode::f;

			switch (mode) {
			case eMode::v:
				{
					Vertex vt = {};
					float x, y, z;
					sscanf_s(line.c_str(), "v %f %f %f", &x, &y, &z);
					vt.position = { x, y, z, 1.0f };
					vt.color    = { 1.f, 1.f, 1.f, 1.f };
					vt.normal   = { 0.f, 0.f, 0.f };
					vertices.push_back(std::move(vt));
					countV++;
				}
				break;
			case eMode::f:
				{
					UINT i[3];
					sscanf_s(line.c_str(), "f %d %d %d", i, i+1, i+2);
					indices.push_back(i[0]-1);
					indices.push_back(i[1]-1);
					indices.push_back(i[2]-1);
					countF+=3;
				}
				break;
			}
		}
		{
			using namespace DirectX;
			for (int i = 0; i < indices.size(); i += 3) {
				auto vec0 = XMVectorSubtract(vertices[indices[i+1]].position, vertices[indices[i+0]].position);
				auto vec1 = XMVectorSubtract(vertices[indices[i+2]].position, vertices[indices[i+0]].position);
				auto normal = XMVector3Normalize(XMVector3Cross(vec0, vec1));
				vertices[indices[i+0]].normal = XMVector3Normalize(XMVectorAdd(vertices[indices[i+0]].normal, normal));
				vertices[indices[i+1]].normal = XMVector3Normalize(XMVectorAdd(vertices[indices[i+1]].normal, normal));
				vertices[indices[i+2]].normal = XMVector3Normalize(XMVectorAdd(vertices[indices[i+2]].normal, normal));
			}
		}
		
		ifs.close();
	}
};