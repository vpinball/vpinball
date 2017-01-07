#pragma once
#ifndef _OBJLOADER_H
#define _OBJLOADER_H

FILE* WaveFrontObj_ExportStart(const char *filename);
void WaveFrontObj_ExportEnd(FILE *f);
void WaveFrontObj_UpdateFaceOffset(unsigned int numVertices);
void WaveFrontObj_WriteObjectName(FILE *f, const char *objname);
void WaveFrontObj_WriteVertexInfo(FILE *f, const Vertex3D_NoTex2 *verts, unsigned int numVerts);
void WaveFrontObj_WriteFaceInfo(FILE *f, const std::vector<WORD> &faces);
void WaveFrontObj_WriteFaceInfoLong(FILE *f, const std::vector<unsigned int> &faces);
void WaveFrontObj_Save(const char *filename, const char *description, const Mesh& mesh);
void WaveFrontObj_GetVertices(std::vector<Vertex3D_NoTex2>& objMesh);
void WaveFrontObj_GetIndices(std::vector<unsigned int>& list);
bool WaveFrontObj_Load(const char *filename, const bool flipTv, const bool convertToLeftHanded);
void WaveFrontObj_WriteFaceInfoList(FILE *f, const WORD *faces, const unsigned int numIndices);
void WaveFrontObj_WriteMaterial(const char *texelName, const char *texelFilename, const Material * const mat);
void WaveFrontObj_UseTexture(FILE *f, const char *texelName);
bool WaveFrontObjLoadMaterial(const char *filename, Material *mat);


#endif // !_OBJLOADER_H
