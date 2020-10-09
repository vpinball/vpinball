#include "stdafx.h"
#include <hash.h>

// not thread safe!

struct MyPoly
{
   int vi0, ti0, ni0;
   int vi1, ti1, ni1;
   int vi2, ti2, ni2;
};

static vector<Vertex3Ds> tmpVerts;
static vector<Vertex3Ds> tmpNorms;
static vector<Vertex2D> tmpTexel;
static std::unordered_set<std::pair<const Vertex3D_NoTex2*,const unsigned int>, Vertex3D_NoTex2IdxHashFunctor, Vertex3D_NoTex2IdxComparator> tmpCombined; // only used to find duplicate vertices quickly
static vector<MyPoly> tmpFaces;
static vector<Vertex3D_NoTex2> verts;
static vector<unsigned int> indices;
static unsigned int faceIndexOffset = 0;
static FILE *matFile = NULL;

#if 0
static void NormalizeNormals()
{
   for (size_t i = 0; i < indices.size(); i += 3)
   {
      const unsigned int A = indices[i];
      const unsigned int B = indices[i + 1];
      const unsigned int C = indices[i + 2];
      const float x1 = verts[B].x - verts[A].x;
      const float y1 = verts[B].y - verts[A].y;
      const float z1 = verts[B].z - verts[A].z;
      const float x2 = verts[C].x - verts[A].x;
      const float y2 = verts[C].y - verts[A].y;
      const float z2 = verts[C].z - verts[A].z;
      float nx = y1*z2 - z1*y2;
      float ny = z1*x2 - x1*z1;
      float nz = x1*y2 - y1*x1;
      const float inv_len = 1.0f / sqrtf(nx*nx + ny*ny + nz*nz);
      nx *= inv_len;
      ny *= inv_len;
      nz *= inv_len;
      const int v[3] = { A, B, C };
      vector<int> seen;
      seen.resize(verts.size(), 0); //!!
      for (int t = 0; t < 3; t++)
      {
         const int c = v[t];
         seen[c]++;
         if (seen[c] == 1)
         {
            verts[c].nx = nx;
            verts[c].ny = ny;
            verts[c].nz = nz;
         }
         else
         {
            const float inv_seen = 1.0f / (float)seen[c];
            verts[c].nx = verts[c].nx * (1.0f - inv_seen) + nx*inv_seen;
            verts[c].ny = verts[c].ny * (1.0f - inv_seen) + ny*inv_seen;
            verts[c].nz = verts[c].nz * (1.0f - inv_seen) + nz*inv_seen;
            const float inv_len2 = 1.0f / sqrtf(verts[c].nx*verts[c].nx + verts[c].ny*verts[c].ny + verts[c].nz*verts[c].nz);
            verts[c].nx *= inv_len2;
            verts[c].ny *= inv_len2;
            verts[c].nz *= inv_len2;
         }
      }
      seen.clear();
   }
}
#endif

bool WaveFrontObj_LoadMaterial(const string& filename, Material * const mat)
{
   FILE *f;
   if ((fopen_s(&f, filename.c_str(), "r") != 0) || !f)
      return false;

   while (1)
   {
      char lineHeader[256];
      const int res = fscanf_s(f, "\n%s", lineHeader, 256);
      if (res == EOF)
      {
         fclose(f);
         return true;
      }
      if (strcmp(lineHeader, "newmtl") == 0)
      {
         char buf[MAXSTRING];
         fscanf_s(f, "%s\n", buf, MAXSTRING);
         mat->m_szName = buf;
      }
      else if (strcmp(lineHeader, "Ns") == 0)
      {
         float tmp;
         fscanf_s(f, "%f\n", &tmp);
         const int d = (int)(tmp*100.f);
         tmp = d / 100.0f;
         // normally a wavefront material specular exponent ranges from 0..1000.
         // but our shininess calculation differs from the way how e.g. Blender is calculating the specular exponent
         // starting from 0.5 and use only half of the exponent resolution to get a similar look
         mat->m_fRoughness = 0.5f + (tmp / 2000.0f);

         if (mat->m_fRoughness > 1.0f)
            mat->m_fRoughness = 1.0f;
         if (mat->m_fRoughness < 0.01f)
            mat->m_fRoughness = 0.01f;
      }
      else if (strcmp(lineHeader, "Ka") == 0)
      {
         Vertex3Ds tmp;
         fscanf_s(f, "%f %f %f\n", &tmp.x, &tmp.y, &tmp.z);
      }
      else if (strcmp(lineHeader, "Kd") == 0)
      {
         Vertex3Ds tmp;
         fscanf_s(f, "%f %f %f\n", &tmp.x, &tmp.y, &tmp.z);
         const DWORD r = (DWORD)(tmp.x * 255.f);
         const DWORD g = (DWORD)(tmp.y * 255.f);
         const DWORD b = (DWORD)(tmp.z * 255.f);
         mat->m_cBase = RGB(r, g, b);
      }
      else if (strcmp(lineHeader, "Ks") == 0)
      {
         Vertex3Ds tmp;
         fscanf_s(f, "%f %f %f\n", &tmp.x, &tmp.y, &tmp.z);
         const DWORD r = (DWORD)(tmp.x * 255.f);
         const DWORD g = (DWORD)(tmp.y * 255.f);
         const DWORD b = (DWORD)(tmp.z * 255.f);
         mat->m_cGlossy = RGB(r, g, b);
      }
      else if (strcmp(lineHeader, "Ni") == 0)
      {
         float tmp;
         fscanf_s(f, "%f\n", &tmp);
      }
      else if (strcmp(lineHeader, "d") == 0)
      {
         float tmp;
         fscanf_s(f, "%f\n", &tmp);
         if (tmp > 1.0f) tmp = 1.0f;
         mat->m_fOpacity = tmp;
         break;
      }
   }
   fclose(f);
   return true;
}

bool WaveFrontObj_Load(const string& filename, const bool flipTv, const bool convertToLeftHanded)
{
   FILE *f;
   if ((fopen_s(&f, filename.c_str(), "r") != 0) || !f)
      return false;

   tmpVerts.clear();
   tmpTexel.clear();
   tmpNorms.clear();
   tmpCombined.clear();
   tmpFaces.clear();
   verts.clear();
   indices.clear();

   struct VertInfo { int v; int t; int n; };
   std::vector<VertInfo> faceVerts;

   // need some small data type 
   while (1)
   {
      char lineHeader[256];
      const int res = fscanf_s(f, "\n%s", lineHeader, 256);
      if (res == EOF)
      {
         fclose(f);
         break;
      }

      if (strcmp(lineHeader, "v") == 0)
      {
         Vertex3Ds tmp;
         fscanf_s(f, "%f %f %f\n", &tmp.x, &tmp.y, &tmp.z);
         if (convertToLeftHanded)
            tmp.z = -tmp.z;
         tmpVerts.push_back(tmp);
      }
      else if (strcmp(lineHeader, "vt") == 0)
      {
         Vertex2D tmp;
         fscanf_s(f, "%f %f", &tmp.x, &tmp.y);
         if (flipTv || convertToLeftHanded)
            tmp.y = 1.f - tmp.y;
         tmpTexel.push_back(tmp);
      }
      else if (strcmp(lineHeader, "vn") == 0)
      {
         Vertex3Ds tmp;
         fscanf_s(f, "%f %f %f\n", &tmp.x, &tmp.y, &tmp.z);
         if (convertToLeftHanded)
            tmp.z = -tmp.z;
         tmpNorms.push_back(tmp);
      }
      else if (strcmp(lineHeader, "f") == 0)
      {
         if (tmpVerts.size() == 0)
         {
            ShowError("No vertices found in obj file, import is impossible!");
            goto Error;
         }
         if (tmpTexel.size() == 0)
         {
            ShowError("No texture coordinates (UVs) found in obj file, import is impossible!");
            goto Error;
         }
         if (tmpNorms.size() == 0)
         {
            ShowError("No normals found in obj file, import is impossible!");
            goto Error;
         }
         faceVerts.clear();
         int matches;
         do
         {
            VertInfo vi;
            matches = fscanf_s(f, "%d/%d/%d", &vi.v, &vi.t, &vi.n);
            if (matches > 0 && matches != 3)
            {
               ShowError("Face information incorrect! Each face needs vertices, UVs and normals!");
               goto Error;
            }
            if (matches == 3)
            {
               vi.v--; vi.t--; vi.n--;    // convert to 0-based indices
               faceVerts.push_back(vi);
            }
         } while (matches > 0);

         if (faceVerts.size() < 3)
         {
            ShowError("Invalid face -- less than 3 vertices!");
            goto Error;
         }

         if (convertToLeftHanded)
            std::reverse(faceVerts.begin(), faceVerts.end());

         // triangulate the face (assumes convex face)
         MyPoly tmpFace;
         tmpFace.vi0 = faceVerts[0].v;
         tmpFace.ti0 = faceVerts[0].t;
         tmpFace.ni0 = faceVerts[0].n;
         for (size_t i = 1; i < faceVerts.size() - 1; ++i)
         {
            tmpFace.vi1 = faceVerts[i].v;
            tmpFace.ti1 = faceVerts[i].t;
            tmpFace.ni1 = faceVerts[i].n;

            tmpFace.vi2 = faceVerts[i + 1].v;
            tmpFace.ti2 = faceVerts[i + 1].t;
            tmpFace.ni2 = faceVerts[i + 1].n;

            tmpFaces.push_back(tmpFace);
         }
      }
      else
      {
         // unknown line header, skip rest of line
         fgets(lineHeader, 256, f);
      }
   }

   verts.reserve(tmpFaces.size()*3); //!! reserve more than needed, but this is necessary to guarantee to not have any re-allocs below, so that all addresses to elements in the vector remain the same during the loop!
   indices.reserve(tmpFaces.size()*3); // reserve what is needed upfront

   for (size_t i = 0; i < tmpFaces.size(); i++)
   {
      Vertex3D_NoTex2 tmp;
      tmp.x  = tmpVerts[tmpFaces[i].vi0].x;
      tmp.y  = tmpVerts[tmpFaces[i].vi0].y;
      tmp.z  = tmpVerts[tmpFaces[i].vi0].z;
      tmp.tu = tmpTexel[tmpFaces[i].ti0].x;
      tmp.tv = tmpTexel[tmpFaces[i].ti0].y;
      tmp.nx = tmpNorms[tmpFaces[i].ni0].x;
      tmp.ny = tmpNorms[tmpFaces[i].ni0].y;
      tmp.nz = tmpNorms[tmpFaces[i].ni0].z;
      std::unordered_set<std::pair<const Vertex3D_NoTex2*,const unsigned int>, Vertex3D_NoTex2IdxHashFunctor, Vertex3D_NoTex2IdxComparator>::const_iterator idx = tmpCombined.find(std::pair<const Vertex3D_NoTex2*,const unsigned int>(&tmp,0)); // idx is ignored when searching via find
      if (idx == tmpCombined.end())
      {
         verts.push_back(tmp);
         tmpCombined.emplace(std::pair<const Vertex3D_NoTex2*,const unsigned int>(&verts.back(),(unsigned int)(verts.size() - 1)));
         indices.push_back((unsigned int)(verts.size() - 1));
      }
      else
         indices.push_back(idx->second);

      tmp.x  = tmpVerts[tmpFaces[i].vi1].x;
      tmp.y  = tmpVerts[tmpFaces[i].vi1].y;
      tmp.z  = tmpVerts[tmpFaces[i].vi1].z;
      tmp.tu = tmpTexel[tmpFaces[i].ti1].x;
      tmp.tv = tmpTexel[tmpFaces[i].ti1].y;
      tmp.nx = tmpNorms[tmpFaces[i].ni1].x;
      tmp.ny = tmpNorms[tmpFaces[i].ni1].y;
      tmp.nz = tmpNorms[tmpFaces[i].ni1].z;
      idx = tmpCombined.find(std::pair<const Vertex3D_NoTex2*,const unsigned int>(&tmp,0)); // idx is ignored when searching via find
      if (idx == tmpCombined.end())
      {
         verts.push_back(tmp);
         tmpCombined.emplace(std::pair<const Vertex3D_NoTex2*,const unsigned int>(&verts.back(),(unsigned int)(verts.size() - 1)));
         indices.push_back((unsigned int)(verts.size() - 1));
      }
      else
         indices.push_back(idx->second);

      tmp.x  = tmpVerts[tmpFaces[i].vi2].x;
      tmp.y  = tmpVerts[tmpFaces[i].vi2].y;
      tmp.z  = tmpVerts[tmpFaces[i].vi2].z;
      tmp.tu = tmpTexel[tmpFaces[i].ti2].x;
      tmp.tv = tmpTexel[tmpFaces[i].ti2].y;
      tmp.nx = tmpNorms[tmpFaces[i].ni2].x;
      tmp.ny = tmpNorms[tmpFaces[i].ni2].y;
      tmp.nz = tmpNorms[tmpFaces[i].ni2].z;
      idx = tmpCombined.find(std::pair<const Vertex3D_NoTex2*,const unsigned int>(&tmp,0)); // idx is ignored when searching via find
      if (idx == tmpCombined.end())
      {
         verts.push_back(tmp);
         tmpCombined.emplace(std::pair<const Vertex3D_NoTex2*,const unsigned int>(&verts.back(),(unsigned int)(verts.size() - 1)));
         indices.push_back((unsigned int)(verts.size() - 1));
      }
      else
         indices.push_back(idx->second);
   }
   // not used yet
   //   NormalizeNormals();

   tmpVerts.clear();
   tmpTexel.clear();
   tmpNorms.clear();
   tmpCombined.clear();
   tmpFaces.clear();
   return true;

Error:
   tmpVerts.clear();
   tmpTexel.clear();
   tmpNorms.clear();
   tmpCombined.clear();
   tmpFaces.clear();
   fclose(f);
   return false;
}

void WaveFrontObj_GetVertices(std::vector<Vertex3D_NoTex2>& vertices) // clears temporary storage on the way
{
   vertices = verts;
   verts.clear();
}

void WaveFrontObj_GetIndices(std::vector<unsigned int>& idx) // clears temporary storage on the way
{
   idx = indices;
   indices.clear();
}

FILE* WaveFrontObj_ExportStart(const string& filename)
{
   const int len = min((int)filename.length(), MAX_PATH-1);
   int i;
   for (i = len; i >= 0; i--)
   {
      if (filename[i] == '.')
      {
         i++;
         break;
      }
   }
   char matName[MAX_PATH];
   strncpy_s(matName, filename.c_str(), sizeof(matName)-1);
   if (i < len)
   {      
      memcpy(matName, filename.c_str(), i);
      matName[i] = 0;
      strncat_s(matName, "mtl", sizeof(matName)-strnlen_s(matName, sizeof(matName))-1);
   }

   for (i = len; i >= 0; i--)
   {
      if (matName[i] == '\\')
      {
         i++;
         break;
      }
   }
   char nameOnly[MAX_PATH] = { 0 };
   memcpy(nameOnly, matName + i, len - i);
   if ((fopen_s(&matFile, matName, "wt") != 0) || !matFile)
      return 0;
   fprintf_s(matFile, "# Visual Pinball table mat file\n");

   FILE *f;
   if ((fopen_s(&f, filename.c_str(), "wt") != 0) || !f)
      return 0;
   faceIndexOffset = 0;
   fprintf_s(f, "# Visual Pinball table OBJ file\n");
   fprintf_s(f, "mtllib %s\n", nameOnly);
   return f;
}

void WaveFrontObj_ExportEnd(FILE *f)
{
   fclose(f);
   fclose(matFile);
}

void WaveFrontObj_WriteMaterial(const string& texelName, const string& texelFilename, const Material * const mat)
{
   char texelNameCopy[MAX_PATH];
   strncpy_s(texelNameCopy, texelName.c_str(), sizeof(texelNameCopy)-1);
   RemoveSpaces(texelNameCopy);
   fprintf_s(matFile, "newmtl %s\n", texelNameCopy);
   fprintf_s(matFile, "Ns 7.843137\n");
   vec4 color = convertColor(mat->m_cBase);
   fprintf_s(matFile, "Ka 0.000000 0.000000 0.000000\n");
   fprintf_s(matFile, "Kd %f %f %f\n", color.x, color.y, color.z);
   color = convertColor(mat->m_cGlossy);
   fprintf_s(matFile, "Ks %f %f %f\n", color.x, color.y, color.z);
   fprintf_s(matFile, "Ni 1.500000\n");
   fprintf_s(matFile, "d %f\n", mat->m_fOpacity);
   fprintf_s(matFile, "illum 5\n");
   if (!texelFilename.empty())
   {
      strncpy_s(texelNameCopy, texelFilename.c_str(), sizeof(texelNameCopy)-1);
      RemoveSpaces(texelNameCopy);

      fprintf_s(matFile, "map_kd %s\n", texelNameCopy);
      fprintf_s(matFile, "map_ka %s\n\n", texelNameCopy);
   }
}

void WaveFrontObj_UseTexture(FILE *f, const string& texelName)
{
   fprintf_s(f, "usemtl %s\n", texelName.c_str());
}

void WaveFrontObj_UpdateFaceOffset(unsigned int numVertices)
{
   faceIndexOffset += numVertices;
}

void WaveFrontObj_WriteObjectName(FILE *f, const string& objname)
{
   fprintf_s(f, "o %s\n", objname.c_str());
}

void WaveFrontObj_WriteVertexInfo(FILE *f, const Vertex3D_NoTex2 *in_verts, unsigned int numVerts)
{
   for (unsigned i = 0; i < numVerts; i++)
   {
      fprintf_s(f, "v %f %f %f\n", in_verts[i].x, in_verts[i].y, -in_verts[i].z);
   }
   for (unsigned i = 0; i < numVerts; i++)
   {
      float tu = in_verts[i].tu;
      float tv = 1.f - in_verts[i].tv;
      if (tu != tu) tu = 0.0f;
      if (tv != tv) tv = 0.0f;
      fprintf_s(f, "vt %f %f\n", tu, tv);
   }
   for (unsigned i = 0; i < numVerts; i++)
   {
      float nx = in_verts[i].nx;
      float ny = in_verts[i].ny;
      float nz = in_verts[i].nz;
      if (nx != nx) nx = 0.0f;
      if (ny != ny) ny = 0.0f;
      if (nz != nz) nz = 0.0f;
      fprintf_s(f, "vn %f %f %f\n", nx, ny, -nz);
   }
}

void WaveFrontObj_WriteFaceInfo(FILE *f, const std::vector<WORD> &in_faces)
{
   for (size_t i = 0; i < in_faces.size(); i += 3)
   {
      fprintf_s(f, "f %u/%u/%u %u/%u/%u %u/%u/%u\n", in_faces[i + 2] + 1 + faceIndexOffset, in_faces[i + 2] + 1 + faceIndexOffset, in_faces[i + 2] + 1 + faceIndexOffset
         , in_faces[i + 1] + 1 + faceIndexOffset, in_faces[i + 1] + 1 + faceIndexOffset, in_faces[i + 1] + 1 + faceIndexOffset
         , in_faces[i] + 1 + faceIndexOffset, in_faces[i] + 1 + faceIndexOffset, in_faces[i] + 1 + faceIndexOffset);
   }
}

void WaveFrontObj_WriteFaceInfoList(FILE *f, const WORD *in_faces, const unsigned int numIndices)
{
   fprintf_s(f, "s 1\n");
   for (unsigned i = 0; i < numIndices; i += 3)
   {
      fprintf_s(f, "f %u/%u/%u %u/%u/%u %u/%u/%u\n", in_faces[i + 2] + 1 + faceIndexOffset, in_faces[i + 2] + 1 + faceIndexOffset, in_faces[i + 2] + 1 + faceIndexOffset
         , in_faces[i + 1] + 1 + faceIndexOffset, in_faces[i + 1] + 1 + faceIndexOffset, in_faces[i + 1] + 1 + faceIndexOffset
         , in_faces[i] + 1 + faceIndexOffset, in_faces[i] + 1 + faceIndexOffset, in_faces[i] + 1 + faceIndexOffset);
   }
}

void WaveFrontObj_WriteFaceInfoLong(FILE *f, const std::vector<unsigned int> &in_faces)
{
   for (size_t i = 0; i < in_faces.size(); i += 3)
   {
      fprintf_s(f, "f %u/%u/%u %u/%u/%u %u/%u/%u\n", in_faces[i + 2] + 1 + faceIndexOffset, in_faces[i + 2] + 1 + faceIndexOffset, in_faces[i + 2] + 1 + faceIndexOffset
         , in_faces[i + 1] + 1 + faceIndexOffset, in_faces[i + 1] + 1 + faceIndexOffset, in_faces[i + 1] + 1 + faceIndexOffset
         , in_faces[i] + 1 + faceIndexOffset, in_faces[i] + 1 + faceIndexOffset, in_faces[i] + 1 + faceIndexOffset);
   }
}

// exporting a mesh to a Wavefront .OBJ file. The mesh is converted into right-handed coordinate system (VP uses left-handed)
void WaveFrontObj_Save(const string& filename, const string& description, const Mesh& mesh)
{
   FILE *f;
   /*
   f = fopen(filename.c_str(), "wt");
   fprintf_s(f,"const unsigned int hitTargetT2Vertices=%i;\n",mesh.NumVertices());
   fprintf_s(f,"const unsigned int hitTargetT2NumFaces=%i;\n", mesh.NumIndices());
   fprintf_s(f,"Vertex3D_NoTex2 hitTargetT2Mesh[%i]=\n{\n",mesh.NumVertices());

   for (int i=0; i<mesh.NumVertices(); i++)
   {
   fprintf_s(f,"{ %ff,%ff,%ff, %ff,%ff,%ff, %ff,%ff},\n",mesh.m_vertices[i].x, mesh.m_vertices[i].y,mesh.m_vertices[i].z,
   mesh.m_vertices[i].nx, mesh.m_vertices[i].ny,mesh.m_vertices[i].nz,
   mesh.m_vertices[i].tu, mesh.m_vertices[i].tv);
   }

   fprintf_s(f,"};\nWORD hitTargetT2Indices[%i]=\n{\n   ",mesh.NumIndices());

   int ofs=0;
   for (int i=0; i<mesh.NumIndices(); i++)
   {
   fprintf_s(f,"%i,",mesh.m_indices[i]);
   ofs++;
   if (ofs==15)
   {
   ofs=0;
   fprintf_s(f,"\n   ");
   }
   }
   fprintf_s(f,"\n};");
   fclose(f);
   return;
   */
   if (mesh.m_animationFrames.size() == 0)
   {
      f = WaveFrontObj_ExportStart(filename);
      if (!f)
           return;
      fprintf_s(f, "# Visual Pinball OBJ file\n");
      fprintf_s(f, "# numVerts: %u numFaces: %u\n", (unsigned int)mesh.NumVertices(), (unsigned int)mesh.NumIndices());
      WaveFrontObj_WriteObjectName(f, description);
      WaveFrontObj_WriteVertexInfo(f, mesh.m_vertices.data(), (unsigned int)mesh.m_vertices.size());
      WaveFrontObj_WriteFaceInfoLong(f, mesh.m_indices);
      WaveFrontObj_ExportEnd(f);
   }
   else
   {
      const std::size_t pos = filename.find_last_of('.');
      assert(pos != string::npos);
      const string name = filename.substr(0, pos);
      char number[32] = { 0 };
      for (unsigned int i = 0; i < mesh.m_animationFrames.size(); i++)
      {
         std::vector<Vertex3D_NoTex2> vertsTmp = mesh.m_vertices;

         for (unsigned int t = 0; t < mesh.NumVertices(); t++)
         {
             const Mesh::VertData vi = mesh.m_animationFrames[i].m_frameVerts[t];
             vertsTmp[t].x  = vi.x;
             vertsTmp[t].y  = vi.y;
             vertsTmp[t].z  = vi.z;
             vertsTmp[t].nx = vi.nx;
             vertsTmp[t].ny = vi.ny;
             vertsTmp[t].nz = vi.nz;
         }
         sprintf_s(number, "%05u", i);
         const string fname = name + '_' + number + ".obj";
         f = WaveFrontObj_ExportStart(fname);
         if (!f)
               return;
         fprintf_s(f, "# Visual Pinball OBJ file\n");
         fprintf_s(f, "# numVerts: %u numFaces: %u\n", (unsigned int)mesh.NumVertices(), (unsigned int)mesh.NumIndices());
         WaveFrontObj_WriteObjectName(f, description);
         WaveFrontObj_WriteVertexInfo(f, vertsTmp.data(), (unsigned int)mesh.m_vertices.size());
         WaveFrontObj_WriteFaceInfoLong(f, mesh.m_indices);
         WaveFrontObj_ExportEnd(f);
       }
   }
}
