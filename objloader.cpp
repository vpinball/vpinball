#include "stdafx.h"

struct MyPoly
{
   int vi0,ti0,ni0;
   int vi1,ti1,ni1;
   int vi2,ti2,ni2;
};

static vector<Vertex3Ds> tmpVerts;
static vector<Vertex3Ds> tmpNorms;
static vector<Vertex2D> tmpTexel;
static vector<MyPoly> tmpFaces;
static vector<Vertex3D_NoTex2> verts;
static vector<int> faces;

static int isInList( const int vi, const int ti, const int ni )
{
   for( unsigned int i=0;i<verts.size();i++ )
      if( verts[i].x==tmpVerts[vi].x && verts[i].y==tmpVerts[vi].y && verts[i].z==tmpVerts[vi].z 
	  &&
	  verts[i].tu==tmpTexel[ti].x && verts[i].tv==tmpTexel[ti].y 
	  &&
	  verts[i].nx==tmpNorms[ni].x && verts[i].ny==tmpNorms[ni].y && verts[i].nz==tmpNorms[ni].z )
      {
         return i;
      }

   return -1;
}

#if 0
static void NormalizeNormals()
{
   for( unsigned int i=0;i<faces.size();i+=3 )
   {
      const int A=faces[i];
      const int B=faces[i+1];
      const int C=faces[i+2];
      const float x1 = verts[B].x - verts[A].x;
      const float y1 = verts[B].y - verts[A].y;
      const float z1 = verts[B].z - verts[A].z;
      const float x2 = verts[C].x - verts[A].x;
      const float y2 = verts[C].y - verts[A].y;
      const float z2 = verts[C].z - verts[A].z;
      float nx = y1*z2 - z1*y2;
      float ny = z1*x2 - x1*z1;
      float nz = x1*y2 - y1*x1;
      const float inv_len = 1.0f/sqrtf(nx*nx + ny*ny + nz*nz );
      nx *= inv_len;
      ny *= inv_len;
      nz *= inv_len;
	  const int v[3] = {A,B,C};
      vector<int> seen;
      seen.resize( verts.size(),0 ); //!!
      for( int t=0;t<3; t++ )
      {
         const int c=v[t];
         seen[c]++;
         if( seen[c]==1 )
         {
            verts[c].nx = nx;
            verts[c].ny = ny;
            verts[c].nz = nz;
         }
         else
         {
			const float inv_seen = 1.0f/(float)seen[c];
            verts[c].nx = verts[c].nx * (1.0f - inv_seen) + nx*inv_seen;
            verts[c].ny = verts[c].ny * (1.0f - inv_seen) + ny*inv_seen;
            verts[c].nz = verts[c].nz * (1.0f - inv_seen) + nz*inv_seen;
            const float inv_len2 = 1.0f/sqrtf(verts[c].nx*verts[c].nx + verts[c].ny*verts[c].ny + verts[c].nz*verts[c].nz);
            verts[c].nx *= inv_len2;
            verts[c].ny *= inv_len2;
            verts[c].nz *= inv_len2;
         }
      }
      seen.clear();
   }
}
#endif

bool WaveFrontObj_Load(const char *filename, const bool flipTv, const bool convertToLeftHanded )
{
   FILE *f;
   fopen_s(&f,filename,"r");
   if( !f )
      return false;

   tmpVerts.clear();
   tmpTexel.clear();
   tmpNorms.clear();
   tmpFaces.clear();
   verts.clear();
   faces.clear();

   struct VertInfo { int v; int t; int n; };
   std::vector<VertInfo> faceVerts;

   // need some small data type 
   while ( 1 )
   {
      char lineHeader[256];
      int res = fscanf_s(f,"\n%s", lineHeader, 256 );
      if( res==EOF ) 
      {
         fclose(f);
         break;
      }

      if( strcmp( lineHeader,"v") == 0 )
      {
         Vertex3Ds tmp;
         fscanf_s(f, "%f %f %f\n",&tmp.x, &tmp.y, &tmp.z );
         if ( convertToLeftHanded )
            tmp.z*=-1.0f;
         tmpVerts.push_back(tmp);
      }
      else if( strcmp( lineHeader,"vt") == 0 )
      {
         Vertex2D tmp;
         fscanf_s(f, "%f %f",&tmp.x, &tmp.y );
         if ( flipTv || convertToLeftHanded )
         {
            tmp.y = 1.f-tmp.y;
         }
         tmpTexel.push_back(tmp);
      }
      else if( strcmp( lineHeader,"vn") == 0 )
      {
         Vertex3Ds tmp;
         fscanf_s(f, "%f %f %f\n",&tmp.x, &tmp.y, &tmp.z );
         if ( convertToLeftHanded )
            tmp.z*=-1.f;
         tmpNorms.push_back(tmp);
      }
      else if( strcmp( lineHeader,"f") == 0 )
      {
         if( tmpVerts.size()==0 )
         {
            ShowError("No vertices found in obj file, import is impossible!");
            goto Error;
         }
         if( tmpTexel.size()==0 )
         {
            ShowError("No texture coordinates (UVs) found in obj file, import is impossible!");
            goto Error;
         }
         if( tmpNorms.size()==0 )
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
         for (unsigned i = 1; i < faceVerts.size() - 1; ++i)
         {
             tmpFace.vi1 = faceVerts[i].v;
             tmpFace.ti1 = faceVerts[i].t;
             tmpFace.ni1 = faceVerts[i].n;

             tmpFace.vi2 = faceVerts[i+1].v;
             tmpFace.ti2 = faceVerts[i+1].t;
             tmpFace.ni2 = faceVerts[i+1].n;

             tmpFaces.push_back(tmpFace);
         }
      }
      else
      {
          // unknown line header, skip rest of line
          fgets(lineHeader, 256, f);
      }
   }

   for( unsigned int i=0;i<tmpFaces.size(); i++ )
   {
      int idx=isInList(tmpFaces[i].vi0, tmpFaces[i].ti0, tmpFaces[i].ni0 );
      if( idx==-1 )
      {
         Vertex3D_NoTex2 tmp;
         tmp.x = tmpVerts[tmpFaces[i].vi0].x;
         tmp.y = tmpVerts[tmpFaces[i].vi0].y;
         tmp.z = tmpVerts[tmpFaces[i].vi0].z;
         tmp.tu = tmpTexel[tmpFaces[i].ti0].x;
         tmp.tv = tmpTexel[tmpFaces[i].ti0].y;
         tmp.nx = tmpNorms[tmpFaces[i].ni0].x;
         tmp.ny = tmpNorms[tmpFaces[i].ni0].y;
         tmp.nz = tmpNorms[tmpFaces[i].ni0].z;
         verts.push_back(tmp);
         faces.push_back( verts.size()-1 );
      }
      else
      {
         faces.push_back(idx);
      }

      idx=isInList(tmpFaces[i].vi1, tmpFaces[i].ti1, tmpFaces[i].ni1 );
      if( idx==-1 )
      {
         Vertex3D_NoTex2 tmp;
         tmp.x = tmpVerts[tmpFaces[i].vi1].x;
         tmp.y = tmpVerts[tmpFaces[i].vi1].y;
         tmp.z = tmpVerts[tmpFaces[i].vi1].z;
         tmp.tu = tmpTexel[tmpFaces[i].ti1].x;
         tmp.tv = tmpTexel[tmpFaces[i].ti1].y;
         tmp.nx = tmpNorms[tmpFaces[i].ni1].x;
         tmp.ny = tmpNorms[tmpFaces[i].ni1].y;
         tmp.nz = tmpNorms[tmpFaces[i].ni1].z;
         verts.push_back(tmp);
         faces.push_back( verts.size()-1 );
      }
      else
      {
         faces.push_back(idx);
      }

      idx=isInList(tmpFaces[i].vi2, tmpFaces[i].ti2, tmpFaces[i].ni2 );
      if( idx==-1 )
      {
         Vertex3D_NoTex2 tmp;
         tmp.x = tmpVerts[tmpFaces[i].vi2].x;
         tmp.y = tmpVerts[tmpFaces[i].vi2].y;
         tmp.z = tmpVerts[tmpFaces[i].vi2].z;
         tmp.tu = tmpTexel[tmpFaces[i].ti2].x;
         tmp.tv = tmpTexel[tmpFaces[i].ti2].y;
         tmp.nx = tmpNorms[tmpFaces[i].ni2].x;
         tmp.ny = tmpNorms[tmpFaces[i].ni2].y;
         tmp.nz = tmpNorms[tmpFaces[i].ni2].z;
         verts.push_back(tmp);
         faces.push_back( verts.size()-1 );
      }
      else
      {
         faces.push_back(idx);
      }
   }
   // not used yet
//   NormalizeNormals();

   tmpVerts.clear();
   tmpTexel.clear();
   tmpNorms.clear();
   tmpFaces.clear();
   return true;

Error:
   tmpVerts.clear();
   tmpTexel.clear();
   tmpNorms.clear();
   tmpFaces.clear();
   fclose(f);
   return false;
}

void WaveFrontObj_GetVertices( std::vector<Vertex3D_NoTex2>& objMesh ) // clears temporary storage on the way
{
   objMesh.resize( verts.size() );
   for( unsigned int i=0;i<verts.size();i++ )
   {
      objMesh[i].x = verts[i].x;
      objMesh[i].y = verts[i].y;
      objMesh[i].z = verts[i].z;
      objMesh[i].tu = verts[i].tu;
      objMesh[i].tv = verts[i].tv;
      objMesh[i].nx = verts[i].nx;
      objMesh[i].ny = verts[i].ny;
      objMesh[i].nz = verts[i].nz;
   }
   verts.clear();
}

void WaveFrontObj_GetIndices( std::vector<unsigned int>& list ) // clears temporary storage on the way
{
   list.resize( faces.size() );
   for( unsigned int i=0; i<faces.size(); i++ )
      list[i] = faces[i];
   faces.clear();
}

FILE* WaveFrontObj_ExportStart(const char *filename)
{
    FILE *f;
    fopen_s(&f, filename, "wt");
    if (!f)
        return 0;
    fprintf_s(f, "# Visual Pinball table OBJ file\n");
    return f;
}

void WaveFrontObj_WriteObjectName(FILE *f, const char *objname)
{
    fprintf_s(f, "o %s\n",objname);
}

void WaveFrontObj_WriteVertexInfo(FILE *f, const Vertex3D_NoTex2 *verts, unsigned int numVerts)
{
    for (unsigned i = 0; i < numVerts; i++)
    {
        fprintf_s(f, "v %f %f %f\n", verts[i].x, verts[i].y, -verts[i].z);
    }
    for (unsigned i = 0; i < numVerts; i++)
    {
        fprintf_s(f, "vn %f %f %f\n", verts[i].nx, verts[i].ny, verts[i].nz);
    }
    for (unsigned i = 0; i < numVerts; i++)
    {
        float tv = 1.f - verts[i].tv;
        fprintf_s(f, "vt %f %f\n", verts[i].tu, tv);
    }
}

void WaveFrontObj_WriteFaceInfo(FILE *f, const std::vector<WORD> &faces)
{
    for (unsigned i = 0; i < faces.size(); i += 3)
    {
        fprintf_s(f, "f %u/%u/%u %u/%u/%u %u/%u/%u\n", faces[i + 2] + 1, faces[i + 2] + 1, faces[i + 2] + 1
                                                     , faces[i + 1] + 1, faces[i + 1] + 1, faces[i + 1] + 1
                                                     , faces[i] + 1, faces[i] + 1, faces[i] + 1);
    }
}

void WaveFrontObj_WriteFaceInfoLong(FILE *f, const std::vector<unsigned int> &faces)
{
    for (unsigned i = 0; i < faces.size(); i += 3)
    {
        fprintf_s(f, "f %u/%u/%u %u/%u/%u %u/%u/%u\n", faces[i + 2] + 1, faces[i + 2] + 1, faces[i + 2] + 1
                                                     , faces[i + 1] + 1, faces[i + 1] + 1, faces[i + 1] + 1
                                                     , faces[i] + 1, faces[i] + 1, faces[i] + 1);
    }
}

// exporting a mesh to a Wavefront .OBJ file. The mesh is converted into right-handed coordinate system (VP uses left-handed)
void WaveFrontObj_Save(const char *filename, const char *description, const Mesh& mesh)
{
   FILE *f;
   fopen_s(&f,filename,"wt");
   if( !f )
      return ;
/*    
   fprintf_s(f,"const unsigned int bumperBaseNumVertices=%i;\n",mesh.NumVertices());
   fprintf_s(f,"const unsigned int bumperBaseNumFaces=%i;\n", mesh.NumIndices());
   fprintf_s(f,"Vertex3D_NoTex2 bumperBase[%i]=\n{\n",mesh.NumVertices());

   for( int i=0;i<mesh.NumVertices();i++ )
   {
       fprintf_s(f,"{ %ff,%ff,%ff, %ff,%ff,%ff, %ff,%ff},\n",mesh.m_vertices[i].x, mesh.m_vertices[i].y,mesh.m_vertices[i].z,
                                                             mesh.m_vertices[i].nx, mesh.m_vertices[i].ny,mesh.m_vertices[i].nz,
                                                             mesh.m_vertices[i].tu, mesh.m_vertices[i].tv);
   }

   fprintf_s(f,"};\nWORD bumperBaseIndices[%i]=\n{\n   ",mesh.NumIndices());

   int ofs=0;
   for( int i=0;i<mesh.NumIndices();i++ )
   {
        fprintf_s(f,"%i,",mesh.m_indices[i]);
        ofs++;
        if( ofs==15 )
        {
            ofs=0;
            fprintf_s(f,"\n   ");
        }
   }
   fprintf_s(f,"\n};");
   fclose(f);
   return;
*/
   fprintf_s(f,"# Visual Pinball OBJ file\n");
   fprintf_s(f,"# numVerts: %u numFaces: %u\n", mesh.NumVertices(), mesh.NumIndices() );
   WaveFrontObj_WriteObjectName(f, description);
   WaveFrontObj_WriteVertexInfo(f, mesh.m_vertices.data(), mesh.m_vertices.size());
   WaveFrontObj_WriteFaceInfoLong(f, mesh.m_indices);
   fclose(f);
}
