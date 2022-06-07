#pragma once
#include <vector>
#include <iostream> 
#include <string>
#include <fstream>
#include <sstream>
#include <math.h>
#include <list>

class ObjLoader1
{
public:
	// 几何体顶点v
	struct vertices {
		float x;
		float y;
		float z;
	};
	// 面f
	struct face {
		unsigned int u; //顶点标号
		unsigned int v; // 贴图顶点标号
		unsigned int w;	// 法向量标号
	};
	ObjLoader1(std::string& filename);//读取函数
	~ObjLoader1() {
#if 0
		std::string filename = "test.obj.mesh";
		name += ".mesh";
		FILE* filefd = fopen(name.c_str(), "wb+");
		if (!filefd)
			return;
		int size = v.size();
		fwrite(&size, sizeof(int), 1, filefd);
		fwrite(&v[0], sizeof(struct vertices), size, filefd);

		size = f.size() * 3;;
		fwrite(&size, sizeof(int), 1, filefd);
		auto faceit = f.begin();
		for (int i = 0; i < f.size(); i++)
		{
			for (int j = 0; j < 3; j++) {
				fwrite(&(*faceit)[j].u, sizeof(unsigned int), 1, filefd);
			}
			faceit++;
		}

#endif
		v.clear();
		f.clear();
	}
	//void obj_parse_face(const string& szface, struct face facesrc);

public:
	std::vector<struct vertices> v;//存放顶点(x,y,z)坐标
	std::list<std::vector<struct face>> f;//存放面的三个顶点索引
	std::string name;
};


