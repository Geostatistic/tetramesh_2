#pragma once

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>
#include <deque>
#include "Math.h"
#include"mesh_io.h"

#define TETLIBRARY
#include "tetgen.h"

class tetrahedral_mesh
{
public:
	uint32_t tetnum, nodenum, facenum, edgenum;
	std::deque<tetrahedra>tetrahedras;
	std::deque<node>nodes;
	std::deque<face>oldfaces;

	std::deque<uint32_t> adjfaces_num;
	std::deque<uint32_t> adjfaces_list;

	std::deque<face>faces;
	std::deque<edge>edges;
	uint32_t max = 1000000000;

	void loadobj(std::string filename);
};

void tetrahedral_mesh::loadobj(std::string filename)
{
	std::ifstream ifs(filename.c_str(), std::ifstream::in);
	if (!ifs.good())
	{
		std::cout << "Error loading obj:(" << filename << ") file not found!" << "\n";
		system("PAUSE");
		exit(0);
	}

	std::string line, key;
	int vertexid = 1, faceid = 0;
	while (!ifs.eof() && std::getline(ifs, line)) 
	{

	key = "";
	std::stringstream stringstream(line);
	stringstream >> key >> std::ws;

	if (key == "v") { // vertex	
		float x, y, z;
		while (!stringstream.eof())
		{
			stringstream >> x >> std::ws >> y >> std::ws >> z >> std::ws;
			nodes.push_back(node(vertexid, x, y, z)); vertexid++;
		}
	}

	else if (key == "f") { // face
		int x,y,z;
		while (!stringstream.eof()) {
			stringstream >> x >> std::ws >> y >> std::ws >> z >> std::ws;
			oldfaces.push_back(face(faceid, x, y, z)); faceid++;
		}
	}
	}

	tetgenio in, out;
	in.numberofpoints = vertexid - 1;
	in.pointlist = new REAL[in.numberofpoints * 3];
	for (int32_t i = 0; i < in.numberofpoints; i++)
	{
		in.pointlist[i * 3 + 0] = nodes.at(i).x;
		in.pointlist[i * 3 + 1] = nodes.at(i).y;
		in.pointlist[i * 3 + 2] = nodes.at(i).z;
	}
	tetrahedralize("fn-nn", &in, &out);

	// jetzt zuordnung zu tetgen
	// 1. nodes to tets - done
	// 2. neighbors to tets - done
	// 3. nodes to faces - done
	// 4. faces to tets - done
	// 5. orig_faces to nodes
	for (int i = 0; i < out.numberoftetrahedra; i++)
	{
		tetrahedra t;
		t.nindex1 = out.tetrahedronlist[i * 4 + 0];
		t.nindex2 = out.tetrahedronlist[i * 4 + 1];
		t.nindex3 = out.tetrahedronlist[i * 4 + 2];
		t.nindex4 = out.tetrahedronlist[i * 4 + 3];
		tetrahedras.push_back(t);
	}

	// assign neighbors to each tet
	for (int i = 0; i < out.numberoftetrahedra; i++)
	{
		tetrahedras.at(i).adjtet1 = out.neighborlist[i * 4 + 0];
		tetrahedras.at(i).adjtet2 = out.neighborlist[i * 4 + 1];
		tetrahedras.at(i).adjtet3 = out.neighborlist[i * 4 + 2];
		tetrahedras.at(i).adjtet4 = out.neighborlist[i * 4 + 3];
	}

	// assign faces to each tet
	for (int i = 0; i < out.numberoftetrahedra; i++)
	{
		tetrahedras.at(i).findex1 = out.tet2facelist[i * 4 + 0];
		tetrahedras.at(i).findex2 = out.tet2facelist[i * 4 + 1];
		tetrahedras.at(i).findex3 = out.tet2facelist[i * 4 + 2];
		tetrahedras.at(i).findex4 = out.tet2facelist[i * 4 + 3];
	}

	// assign nodes to faces
	for (int32_t i = 0; i < out.numberoftrifaces; i++)
	{
		face f;
		f.node_a = out.trifacelist[i * 3 + 0];
		f.node_b = out.trifacelist[i * 3 + 1];
		f.node_c = out.trifacelist[i * 3 + 2];
		faces.push_back(f);
	}

	// assign old faces to new nodes
	for (int i = 0; i < oldfaces.size();i++)
	{
		adjfaces_list.push_back(oldfaces.at(i).node_a); // now: find cuda equivalent for std::vector
		adjfaces_list.push_back(oldfaces.at(i).node_b);
		adjfaces_list.push_back(oldfaces.at(i).node_c);
	}


}