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
					sscanf_s(line.c_str(), "v %f %f %f", &vt.position.x, &vt.position.y, &vt.position.z);
					vt.color  = { 1.f, 1.f, 1.f, 1.f };
					vt.normal = vt.position;
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
		ifs.close();
	}
};