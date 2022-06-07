
#include <vector>
#include <iostream> 
#include <string>
#include <fstream>
#include <sstream>

#include <math.h>

#include "objLoader.h"

using namespace std;




ObjLoader1::ObjLoader1(string& filename) {
#if 0
	//std::string testname = "test.obj.bak.mesh";
	name = filename;

	FILE* filefd = fopen((name + ".mesh").c_str(), "rb+");
	if (!filefd) {
		goto READ_DATA;
	}
	int size = v.size();
	fread(&size, sizeof(int), 1, filefd);
	for (int i = 0; i < size; i++)
	{
		struct vertices vtmp;

		fread(&vtmp, sizeof(struct vertices), 1, filefd);
		v.push_back(vtmp);
		if (i % 10000 == 0)
			std::cout << "i:" << i << std::endl;
	}
	fread(&size, sizeof(int), 1, filefd);

	size /= 3;



	for (int i = 0; i < size; i++)
	{
		std::vector<struct face> vftmp;
		for (int j = 0; j < 3; j++) {
			struct face ftmp;
			fread(&ftmp.u, sizeof(unsigned int), 1, filefd);
			vftmp.push_back(ftmp);
		}
		f.push_back(vftmp);
		if (i % 10000 == 0)
			std::cout << "i:" << i << std::endl;
	}
	return;
#endif
READ_DATA:
	ifstream file(filename);
	string line;
	int count = 0;
	while (getline(file, line))
	{

		if (line.substr(0, 2) == "v ")
		{
			struct vertices vtmp = { 0 };


			count++;


			istringstream s(line.substr(2));
			// v -1.038081 -0.8783139 -2.667491
			s >> vtmp.x >> vtmp.y >> vtmp.z;
			if (vtmp.x == 0 || vtmp.y == 0 || vtmp.z == 0) {
				cout << "0:index:" << count << "line:" << line.substr(2) << endl;;
			}

			v.push_back(vtmp);
		}
		else if (line.substr(0, 1) == "f")
		{
			vector<struct face> vface;

			count++;

			//vector<struct face> vface;
			istringstream s(line.substr(2));
			string tmpdata[3];
			// f 25/25/25     28/28/28           26/26/26
			s >> tmpdata[0] >> tmpdata[1] >> tmpdata[2];
			for (int i = 0; i < 3; i++)
			{
				struct face faceTmp;
				istringstream sdata(tmpdata[i]);
				string buffer;
				// sdata里面存储的   25/25/25
				int j = 0;
				while (getline(sdata, buffer, '/'))
				{
					// 得到的u 就是25
					faceTmp.u = atoi(buffer.c_str()) - 1;
					break;
				}
				vface.push_back(faceTmp);
			}
			f.push_back(vface);
		}
		if (count % 10000 == 0)
		{

			cout << "count : " << count << endl;
			count++;
		}


	}
	file.close();
	cout << "count : " << count << endl;
	cout << "v num: " << v.size() << endl;
	cout << "f num: " << f.size() << endl;
}

