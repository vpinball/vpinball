/*
 *  Progressive Mesh type Polygon Reduction Algorithm
 *  by Stan Melax (c) 1998
 *  Permission to use any of this code wherever you want is granted..
 *  Although, please do acknowledge authorship if appropriate.
 *
 *  See the header file progmesh.h for a description of this module
 */

#include "stdafx.h"

#include <stdio.h>  
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#define NOMINMAX
#include <windows.h>
#include <assert.h>

#include "progmesh.h"

namespace ProgMesh {

template<class T> int   Contains(const std::vector<T> & c, const T & t){ return (int)std::count(begin(c), end(c), t); }
template<class T> int   IndexOf(const std::vector<T> & c, const T & v) { return (int)(std::find(begin(c), end(c), v) - begin(c)); }
//template<class T> T &   Add(std::vector<T> & c, T t)                   { c.push_back(t); return c.back(); }
template<class T> T     Pop(std::vector<T> & c)                        { const T val = std::move(c.back()); c.pop_back(); return val; }
template<class T> void  AddUnique(std::vector<T> & c, T t)             { if (!Contains(c, t)) c.push_back(t); }
//template<class T> void  Remove(std::vector<T> & c, T t)                { const std::vector<T>::const_iterator it = std::find(begin(c), end(c), t); assert(it != end(c)); c.erase(it); assert(!Contains(c, t)); }
template<class T> void  RemoveFillWithBack(std::vector<T> & c, T t)    { const int idxof = IndexOf(c, t); const T val = Pop(c); if (idxof == c.size()) return; c[idxof] = val; assert(!Contains(c, t)); }


/*
 *  For the polygon reduction algorithm we use data structures
 *  that contain a little bit more information than the usual
 *  indexed face set type of data structure.
 *  From a vertex we wish to be able to quickly get the
 *  neighboring faces and vertices.
 */
class Vertex;

class Triangle {
public:
	Vertex *			vertex[3]; // the 3 points that make this tri
	float3				normal;    // unit vector othogonal to this face

	__forceinline		Triangle(Vertex * const v0, Vertex * const v1, Vertex * const v2);
	__forceinline		~Triangle();
	__forceinline void	ComputeNormal();
	inline void			ReplaceVertex(Vertex * const vold, Vertex * const vnew);
	__forceinline int	HasVertex(const Vertex * const v);
};

class Vertex {
public:
	float3          position; // location of point in euclidean space
	unsigned int    id;       // place of vertex in original Array
	std::vector<Vertex *>   neighbor; // adjacent vertices
	std::vector<Triangle *> face;     // adjacent triangles
	float           objdist;  // cached cost of collapsing edge
	Vertex *        collapse; // candidate vertex for collapse

	__forceinline	Vertex(const float3 &v, const size_t _id);
	__forceinline	~Vertex();
	inline void		RemoveIfNonNeighbor(Vertex * const n);
};


static std::vector<Vertex *>   vertices;
static std::vector<Triangle *> triangles;


__forceinline Triangle::Triangle(Vertex * const v0, Vertex * const v1, Vertex * const v2)
{
	assert(v0 != v1 && v1 != v2 && v2 != v0);

	vertex[0] = v0;
	vertex[1] = v1;
	vertex[2] = v2;
	ComputeNormal();
	triangles.push_back(this);

	for (int i = 0; i < 3; i++) {
		vertex[i]->face.push_back(this);
		for (int j = 0; j < 3; j++)
			if (i != j)
				AddUnique(vertex[i]->neighbor, vertex[j]);
	}
}

__forceinline Triangle::~Triangle()
{
	RemoveFillWithBack(triangles, this);
	for (int i = 0; i < 3; i++)
		if (vertex[i])
			RemoveFillWithBack(vertex[i]->face, this);

	for (int i = 0; i < 3; i++) {
		const int i2 = (i + 1) % 3;
		if (vertex[i] && vertex[i2])
		{
			vertex[i]->RemoveIfNonNeighbor(vertex[i2]);
			vertex[i2]->RemoveIfNonNeighbor(vertex[i]);
		}
	}
}

__forceinline int Triangle::HasVertex(const Vertex * const v)
{
	return (v == vertex[0] || v == vertex[1] || v == vertex[2]);
}

__forceinline void Triangle::ComputeNormal()
{
	const float3 v0 = vertex[0]->position;
	const float3 v1 = vertex[1]->position;
	const float3 v2 = vertex[2]->position;
	normal = cross(v1 - v0, v2 - v1);
	if (magnitude(normal) > FLT_MIN)
		normal = normalize(normal);
}

inline void Triangle::ReplaceVertex(Vertex * const vold, Vertex * const vnew)
{
	assert(vold && vnew);
	assert(vold == vertex[0] || vold == vertex[1] || vold == vertex[2]);
	assert(vnew != vertex[0] && vnew != vertex[1] && vnew != vertex[2]);
	
	if (vold == vertex[0])
		vertex[0] = vnew;
	else if (vold == vertex[1])
		vertex[1] = vnew;
	else {
		assert(vold == vertex[2]);
		vertex[2] = vnew;
	}
	RemoveFillWithBack(vold->face, this);
	
	assert(!Contains(vnew->face, this));
	vnew->face.push_back(this);

	for (int i = 0; i < 3; i++) {
		vold->RemoveIfNonNeighbor(vertex[i]);
		vertex[i]->RemoveIfNonNeighbor(vold);
	}

	for (int i = 0; i < 3; i++) {
		assert(Contains(vertex[i]->face, this) == 1);
		for (int j = 0; j < 3; j++)
			if (i != j)
				AddUnique(vertex[i]->neighbor, vertex[j]);
	}

	ComputeNormal();
}

__forceinline Vertex::Vertex(const float3 &v, const size_t _id)
{
	position = v;
	id = (unsigned int)_id;
	vertices.push_back(this);
}

__forceinline Vertex::~Vertex()
{
	assert(face.size() == 0);

	while (neighbor.size()) {
		RemoveFillWithBack(neighbor[0]->neighbor, this);
		RemoveFillWithBack(neighbor, neighbor[0]);
	}
	RemoveFillWithBack(vertices, this);
}

inline void Vertex::RemoveIfNonNeighbor(Vertex * const n) 
{
	// removes n from neighbor Array if n isn't a neighbor.
	if (!Contains(neighbor, n))
		return;
	for (size_t i = 0; i < face.size(); i++)
		if (face[i]->HasVertex(n))
			return;

	RemoveFillWithBack(neighbor, n);
}


__forceinline float ComputeEdgeCollapseCost(const Vertex * const u, const Vertex * const v)
{
	// if we collapse edge uv by moving u to v then how 
	// much different will the model change, i.e. how much "error".
	// Texture, vertex normal, and border vertex code was removed
	// to keep this demo as simple as possible.
	// The method of determining cost was designed in order 
	// to exploit small and coplanar regions for
	// effective polygon reduction.
	// Is is possible to add some checks here to see if "folds"
	// would be generated.  i.e. normal of a remaining face gets
	// flipped.  I never seemed to run into this problem and
	// therefore never added code to detect this case.

	// find the "sides" triangles that are on the edge uv
	std::vector<Triangle *> sides;
	sides.reserve(u->face.size());
	for (size_t i = 0; i < u->face.size(); i++)
		if (u->face[i]->HasVertex(v))
			sides.push_back(u->face[i]);

	// use the triangle facing most away from the sides 
	// to determine our curvature term
	float curvature = 0.f;
	for (size_t i = 0; i < u->face.size(); i++) {
		float mincurv = 1.f; // curve for face i and closer side to it
		for (size_t j = 0; j < sides.size(); j++) {
			const float dotprod = dot(u->face[i]->normal, sides[j]->normal);	  // use dot product of face normals. 
			mincurv = std::min(mincurv, (1.f - dotprod)*0.5f);
		}
		curvature = std::max(curvature, mincurv);
	}

	// the more coplanar the lower the curvature term   
	const float edgelength = magnitude(v->position - u->position);
	return edgelength * curvature;
}

__forceinline void ComputeEdgeCostAtVertex(Vertex * const v)
{
	// compute the edge collapse cost for all edges that start
	// from vertex v.  Since we are only interested in reducing
	// the object by selecting the min cost edge at each step, we
	// only cache the cost of the least cost edge at this vertex
	// (in member variable collapse) as well as the value of the 
	// cost (in member variable objdist).
	if (v->neighbor.size() == 0) {
		// v doesn't have neighbors so it costs nothing to collapse
		v->collapse = NULL;
		v->objdist = -0.01f;
		return;
	}
	v->objdist = FLT_MAX;
	v->collapse = NULL;
	// search all neighboring edges for "least cost" edge
	for (size_t i = 0; i < v->neighbor.size(); i++) {
		const float dist = ComputeEdgeCollapseCost(v, v->neighbor[i]);
		if (dist < v->objdist) {
			v->collapse = v->neighbor[i];  // candidate for edge collapse
			v->objdist = dist;             // cost of the collapse
		}
	}
}

__forceinline void ComputeAllEdgeCollapseCosts()
{
	// For all the edges, compute the difference it would make
	// to the model if it was collapsed.  The least of these
	// per vertex is cached in each vertex object.
	for (size_t i = 0; i < vertices.size(); i++)
		ComputeEdgeCostAtVertex(vertices[i]);
}

__forceinline void Collapse(Vertex * const u, Vertex * const v)
{
	// Collapse the edge uv by moving vertex u onto v
	// Actually remove tris on uv, then update tris that
	// have u to have v, and then remove u.
	if (!v) {
		// u is a vertex all by itself so just delete it
		delete u;
		return;
	}
	std::vector<Vertex *> tmp(u->neighbor.size());
	// make tmp a Array of all the neighbors of u
	for (size_t i = 0; i < tmp.size(); i++)
		tmp[i] = u->neighbor[i];

	// delete triangles on edge uv:
	{
		size_t i = u->face.size();
		while (i--) {
			if (u->face[i]->HasVertex(v))
				delete u->face[i];
		}
	}
	// update remaining triangles to have v instead of u
	{
		size_t i = u->face.size();
		while (i--)
			u->face[i]->ReplaceVertex(u, v);
	}
	delete u;

	// recompute the edge collapse costs for neighboring vertices
	for (size_t i = 0; i < tmp.size(); i++)
		ComputeEdgeCostAtVertex(tmp[i]);
}

__forceinline void AddVertex(const std::vector<float3> &vert)
{
	for (size_t i = 0; i < vert.size(); i++)
		Vertex *v = new Vertex(vert[i], i); //!! braindead design, actually fills up "vertices"
}

__forceinline void AddFaces(const std::vector<tridata> &tri)
{
	for (size_t i = 0; i < tri.size(); i++)
		Triangle *t = new Triangle(vertices[tri[i].v[0]], //!! braindead design, actually fills up "triangles"
								   vertices[tri[i].v[1]],
								   vertices[tri[i].v[2]]);
}

__forceinline Vertex *MinimumCostEdge()
{
	// Find the edge that when collapsed will affect model the least.
	// This funtion actually returns a Vertex, the second vertex
	// of the edge (collapse candidate) is stored in the vertex data.
	// Serious optimization opportunity here: this function currently
	// does a sequential search through an unsorted Array :-(
	// Our algorithm could be O(n*lg(n)) instead of O(n*n)
	Vertex *mn = vertices[0];
	for (size_t i = 0; i < vertices.size(); i++)
		if (vertices[i]->objdist < mn->objdist)
			mn = vertices[i];
	return mn;
}

void ProgressiveMesh(const std::vector<float3> &vert, const std::vector<tridata> &tri,
					 std::vector<unsigned int> &map, std::vector<unsigned int> &permutation)
{
	if (vert.size() == 0 || tri.size() == 0)
		return;

	vertices.reserve(vert.size());
	triangles.reserve(tri.size());

	AddVertex(vert);  // put input data into our data structures
	AddFaces(tri);
	ComputeAllEdgeCollapseCosts(); // cache all edge collapse costs

	permutation.resize(vertices.size());  // allocate space
	map.resize(vertices.size());          // allocate space

	// reduce the object down to nothing:
	while (vertices.size() > 0) {
		// get the next vertex to collapse
		Vertex *mn = MinimumCostEdge();
		// keep track of this vertex, i.e. the collapse ordering
		permutation[mn->id] = (unsigned int)(vertices.size() - 1);
		// keep track of vertex to which we collapse to
		map[vertices.size() - 1] = mn->collapse ? mn->collapse->id : ~0u;
		// Collapse this edge
		Collapse(mn, mn->collapse);
	}

	// reorder the map Array based on the collapse ordering
	for (size_t i = 0; i < map.size(); i++)
		map[i] = (map[i] == ~0u) ? 0 : permutation[map[i]];

	// The caller of this function should reorder their vertices
	// according to the returned "permutation".

	assert(vertices.size() == 0);
	assert(triangles.size() == 0);
}

// Note that the use of the MapVertex() function and the map
// Array isn't part of the polygon reduction algorithm.
// We just set up this system here in this module
// so that we could retrieve the model at any desired vertex count.
// Therefore if this part of the program confuses you, then
// dont worry about it.
// When the model is rendered using a maximum of mx vertices
// then it is vertices 0 through mx-1 that are used.  
// We are able to do this because the vertex Array 
// gets sorted according to the collapse order.
// The MapVertex() routine takes a vertex number 'a' and the
// maximum number of vertices 'mx' and returns the 
// appropriate vertex in the range 0 to mx-1.
// When 'a' is greater than 'mx' the MapVertex() routine
// follows the chain of edge collapses until a vertex
// within the limit is reached.
//   An example to make this clear: assume there is
//   a triangle with vertices 1, 3 and 12.  But when
//   rendering the model we limit ourselves to 10 vertices.
//   In that case we find out how vertex 12 was removed
//   by the polygon reduction algorithm.  i.e. which
//   edge was collapsed.  Lets say that vertex 12 was collapsed
//   to vertex number 7.  This number would have been stored
//   in the collapse_map array (i.e. map[12]==7).
//   Since vertex 7 is in range (less than max of 10) we
//   will want to render the triangle 1,3,7.  
//   Pretend now that we want to limit ourselves to 5 vertices.
//   and vertex 7 was collapsed to vertex 3 
//   (i.e. map[7]==3).  Then triangle 1,3,12 would now be
//   triangle 1,3,3.  i.e. this polygon was removed by the
//   progressive mesh polygon reduction algorithm by the time
//   it had gotten down to 5 vertices.
//   No need to draw a one dimensional polygon. :-)
__forceinline unsigned int MapVertex(unsigned int a, const unsigned int mx, const std::vector<unsigned int> &map)
{
	while (a >= mx)
		a = map[a];
	return a;
}

void ReMapIndices(const unsigned int num_vertices, const std::vector<tridata> &tri, std::vector<tridata> &new_tri, const std::vector<unsigned int> &map)
{
	assert(new_tri.size() == 0);
	assert(map.size() != 0);
	assert(num_vertices != 0);

	for (size_t i = 0; i < tri.size(); i++) {
		tridata t;
		t.v[0] = MapVertex(tri[i].v[0], num_vertices, map);
		t.v[1] = MapVertex(tri[i].v[1], num_vertices, map);
		t.v[2] = MapVertex(tri[i].v[2], num_vertices, map);
		//!! note:  serious optimization opportunity here,
		//  by sorting the triangles the following "continue" 
		//  could have been made into a "break" statement.
		if (t.v[0] == t.v[1] || t.v[1] == t.v[2] || t.v[2] == t.v[0]) continue;
		new_tri.push_back(t);
	}
}

}
