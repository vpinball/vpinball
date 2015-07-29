/*
  Copyright (C) 2008 Martin Storsjo

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

//
// Regarding 2.) altered for VP: templated vertex type, const correctness, remember if already init'ed, change vertex cache size to 64, use structs
//

// Set these to adjust the performance and result quality
#define VERTEX_CACHE_SIZE 64
#define CACHE_FUNCTION_LENGTH 64

// The size of these data types affect the memory usage
typedef unsigned short ScoreType;
#define SCORE_SCALING 7281

typedef unsigned char AdjacencyType;
#define MAX_ADJACENCY 0xffui8

typedef unsigned int VertexIndexType;
typedef signed char  CachePosType;
typedef unsigned int TriangleIndexType;
typedef int          ArrayIndexType;

struct VertexData
{
    ArrayIndexType offset;
    ScoreType lastScore;
    AdjacencyType numActiveTri;
    CachePosType cacheTag;
};

// The size of the precalculated tables
#define CACHE_SCORE_TABLE_SIZE 64u
#define VALENCE_SCORE_TABLE_SIZE 64
#if CACHE_SCORE_TABLE_SIZE < VERTEX_CACHE_SIZE
 #error Vertex score table too small
#endif

// Precalculated tables
static ScoreType cachePositionScore[CACHE_SCORE_TABLE_SIZE];
static ScoreType valenceScore[VALENCE_SCORE_TABLE_SIZE];

// Score function constants
#define CACHE_DECAY_POWER	1.5
#define LAST_TRI_SCORE		0.75
#define VALENCE_BOOST_SCALE	2.0
#define VALENCE_BOOST_POWER	0.5

static bool initForsythDone = false;

// Precalculate the tables
inline void initForsyth() {
    if(initForsythDone)
	return;

	for (unsigned int i = 0; i < CACHE_SCORE_TABLE_SIZE; i++) {
		float score = 0.f;
		if (i < 3) {
			// This vertex was used in the last triangle,
			// so it has a fixed score, which ever of the three
			// it's in. Otherwise, you can get very different
			// answers depending on whether you add
			// the triangle 1,2,3 or 3,1,2 - which is silly
			score = LAST_TRI_SCORE;
		} else {
			// Points for being high in the cache.
			const float scaler = 1.0f / (float)(CACHE_FUNCTION_LENGTH - 3);
			score = powf(1.0f - (float)(i - 3) * scaler, CACHE_DECAY_POWER);
		}
		cachePositionScore[i] = (ScoreType) (SCORE_SCALING * score);
	}

	for (int i = 1; i < VALENCE_SCORE_TABLE_SIZE; i++) {
		// Bonus points for having a low number of tris still to
		// use the vert, so we get rid of lone verts quickly
		const float valenceBoost = powf((float)i, (float)(-VALENCE_BOOST_POWER));
		const float score = (float)VALENCE_BOOST_SCALE * valenceBoost;
		valenceScore[i] = (ScoreType) (SCORE_SCALING * score);
	}

    initForsythDone = true;
}

// Calculate the score for a vertex
inline ScoreType findVertexScore(const int numActiveTris,
                          const unsigned int cachePosition) {
	if (numActiveTris == 0)
		// No triangles need this vertex!
		return 0;

	ScoreType score = (cachePosition >= CACHE_SCORE_TABLE_SIZE) ? 0 // Vertex is not in LRU cache - no score
	                  : cachePositionScore[cachePosition];

	if (numActiveTris < VALENCE_SCORE_TABLE_SIZE)
		score += valenceScore[numActiveTris];
	else
	{
	    const float valenceBoost = powf((float)numActiveTris, (float)(-VALENCE_BOOST_POWER));
	    const float s = (float)VALENCE_BOOST_SCALE * valenceBoost;
	    score += (ScoreType) (SCORE_SCALING * s);
	}

	return score;
}

// The main reordering function
template <typename T>
T* reorderForsyth(const T* const indices,
                  const int nTriangles,
                  const int nVertices)
{
	// The tables need not be inited every time this function
	// is used. Either call initForsyth from the calling process,
	// or just replace the score tables with precalculated values.
	initForsyth();

	VertexData* const cVertex = new VertexData[nVertices];
	for (int i = 0; i < nVertices; ++i)
	    cVertex[i].numActiveTri = 0;

	// First scan over the vertex data, count the total number of
	// occurrances of each vertex
	for (int i = 0; i < 3*nTriangles; i++) {
		if (cVertex[indices[i]].numActiveTri == MAX_ADJACENCY) {
			// Unsupported mesh,
			// vertex shared by too many triangles
			delete [] cVertex;
			return NULL;
		}
		cVertex[indices[i]].numActiveTri++;
	}

	bool* const triangleAdded = new bool[nTriangles];
	ScoreType* const triangleScore = new ScoreType[nTriangles];
	T* const outTriangles = new T[nTriangles];
	T* const triangleIndices = new T[3*nTriangles];
	memset(triangleAdded, 0, sizeof(bool)*nTriangles);
	memset(triangleScore, 0, sizeof(ScoreType)*nTriangles);
	memset(triangleIndices, 0, sizeof(T)*3*nTriangles);

	// Count the triangle array offset for each vertex,
	// initialize the rest of the data.
	int sum = 0;
	for (int i = 0; i < nVertices; i++) {
		cVertex[i].offset = sum;
		sum += cVertex[i].numActiveTri;
		cVertex[i].numActiveTri = 0;
		cVertex[i].cacheTag = -1;
	}

	// Fill the vertex data structures with indices to the triangles
	// using each vertex
	for (int i = 0; i < nTriangles; i++)
		for (int j = 0; j < 3; j++) {
			const int v = indices[3*i + j];
			triangleIndices[cVertex[v].offset + cVertex[v].numActiveTri] = i;
			cVertex[v].numActiveTri++;
		}

	// Initialize the score for all vertices
	for (int i = 0; i < nVertices; i++) {
		cVertex[i].lastScore = findVertexScore(cVertex[i].numActiveTri, cVertex[i].cacheTag);
		const VertexData &c = cVertex[i];
		for (int j = 0; j < c.numActiveTri; j++)
			triangleScore[triangleIndices[c.offset + j]] += c.lastScore;
	}

	// Find the best triangle
	int bestTriangle = -1;
	int bestScore = -1;

	for (int i = 0; i < nTriangles; i++)
		if (triangleScore[i] > bestScore) {
			bestScore = triangleScore[i];
			bestTriangle = i;
		}

	// Initialize the cache
	int cache[VERTEX_CACHE_SIZE + 3];
	memset(cache, 0xFF, sizeof(int)*(VERTEX_CACHE_SIZE + 3));

	int scanPos = 0;
	int outPos = 0;

	// Output the currently best triangle, as long as there
	// are triangles left to output
	while (bestTriangle >= 0) {
		// Mark the triangle as added
		triangleAdded[bestTriangle] = true;
		// Output this triangle
		outTriangles[outPos++] = bestTriangle;
		for (int i = 0; i < 3; i++) {
			// Update this vertex
			const int v = indices[3*bestTriangle + i];

			// Check the current cache position, if it
			// is in the cache
			int endpos = cVertex[v].cacheTag;
			if (endpos < 0)
				endpos = VERTEX_CACHE_SIZE + i;
			// Move all cache entries from the previous position
			// in the cache to the new target position (i) one
			// step backwards
			assert(endpos < VERTEX_CACHE_SIZE+3);
			if(!(endpos < VERTEX_CACHE_SIZE+3))
			{
				delete [] cVertex;
				delete [] triangleAdded;
				delete [] triangleScore;
				delete [] outTriangles;
				delete [] triangleIndices;
				return NULL;
			}
			for (int j = endpos; j > i; j--) {
				cache[j] = cache[j-1];
				// If this cache slot contains a real
				// vertex, update its cache tag
				if (cache[j] >= 0)
				{
					cVertex[cache[j]].cacheTag++;
					if (cVertex[cache[j]].cacheTag >= VERTEX_CACHE_SIZE + 3)
					{
						delete[] cVertex;
						delete[] triangleAdded;
						delete[] triangleScore;
						delete[] outTriangles;
						delete[] triangleIndices;
						return NULL;
					}
				}
			}
			// Insert the current vertex into its new target slot
			cache[i] = v;
			cVertex[v].cacheTag = i;

			// Find the current triangle in the list of active
			// triangles and remove it (moving the last
			// triangle in the list to the slot of this triangle).
			const VertexData &c = cVertex[v];
			for (int j = 0; j < c.numActiveTri; j++) {
				if (triangleIndices[c.offset + j] == bestTriangle) {
					triangleIndices[c.offset + j] = triangleIndices[c.offset + c.numActiveTri - 1];
					break;
				}
			}
			// Shorten the list
			cVertex[v].numActiveTri--;
		}

		// Update the scores of all triangles in the cache
		for (int i = 0; i < VERTEX_CACHE_SIZE + 3; i++) {
			const int v = cache[i];
			if (v < 0)
				break;
			// This vertex has been pushed outside of the
			// actual cache
			if (i >= VERTEX_CACHE_SIZE) {
				cVertex[v].cacheTag = -1;
				cache[i] = -1;
			}
			const ScoreType newScore = findVertexScore(cVertex[v].numActiveTri, cVertex[v].cacheTag);
			const ScoreType diff = newScore - cVertex[v].lastScore;
			for (int j = 0; j < cVertex[v].numActiveTri; j++)
				triangleScore[triangleIndices[cVertex[v].offset + j]] += diff;
			cVertex[v].lastScore = newScore;
		}

		// Find the best triangle referenced by vertices in the cache
		bestTriangle = -1;
		bestScore = -1;
		for (int i = 0; i < VERTEX_CACHE_SIZE; i++) {
			if (cache[i] < 0)
				break;
			const int v = cache[i];
			for (int j = 0; j < cVertex[v].numActiveTri; j++) {
				const unsigned int t = triangleIndices[cVertex[v].offset + j];
				if (triangleScore[t] > bestScore) {
					bestTriangle = t;
					bestScore = triangleScore[t];
				}
			}
		}

		// If no active triangle was found at all, continue
		// scanning the whole list of triangles
		if (bestTriangle < 0) {
			for (; scanPos < nTriangles; scanPos++) {
				if (!triangleAdded[scanPos]) {
					bestTriangle = scanPos;
					break;
				}
			}
		}
	}

	// Clean up
	delete [] cVertex;
	delete [] triangleAdded;
	delete [] triangleScore;

	// Convert the triangle index array into a full triangle list
	outPos = 0;
	for (int i = 0; i < nTriangles; i++) {
		const unsigned int t = outTriangles[i]*3;
		for (int j = 0; j < 3; j++)
			triangleIndices[outPos++] = indices[t + j];
	}
	delete [] outTriangles;

	return triangleIndices;
}
