/*
 *  Progressive Mesh type Polygon Reduction Algorithm
 *  by Stan Melax (c) 1998
 *
 *  The function ProgressiveMesh() takes a model in an "indexed face 
 *  set" sort of way.  i.e. Array of vertices and Array of triangles.
 *  The function then does the polygon reduction algorithm
 *  internally and reduces the model all the way down to 0
 *  vertices and then returns the order in which the
 *  vertices are collapsed and to which neighbor each vertex
 *  is collapsed to.  More specifically the returned "permutation"
 *  indicates how to reorder your vertices so you can render
 *  an object by using the first n vertices (for the n 
 *  vertex version).  After permuting your vertices, the
 *  map Array indicates to which vertex each vertex is collapsed to.
 */

#ifndef PROGRESSIVE_MESH_H
#define PROGRESSIVE_MESH_H

#include <vector>

namespace ProgMesh {

#include "progmesh_vecmatquat_minimal.h"

struct tridata {
	unsigned int v[3];
};

void ProgressiveMesh(const std::vector<float3> &vert, const std::vector<tridata> &tri,
                     std::vector<unsigned int> &map, std::vector<unsigned int> &permutation);

template<typename T>
void PermuteVertices(const std::vector<unsigned int> &permutation, std::vector<T> &vert, std::vector<tridata> &tri)
{
	assert(permutation.size() == vert.size());

	// rearrange the vertex Array 
	std::vector<T> temp_Array(vert.size());
	for (size_t i = 0; i<vert.size(); i++)
		temp_Array[i] = vert[i];
	for (size_t i = 0; i<vert.size(); i++)
		vert[permutation[i]] = temp_Array[i];

	// update the changes in the entries in the triangle Array
	for (size_t i = 0; i<tri.size(); i++)
		for (int j = 0; j<3; j++)
			tri[i].v[j] = permutation[tri[i].v[j]];
}

void ReMapIndices(const unsigned int num_vertices, const std::vector<tridata> &tri, std::vector<tridata> &new_tri, const std::vector<unsigned int> &map);

}

#endif
