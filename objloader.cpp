#include "stdafx.h"

struct MyVector
{
   float x,y,z;
};
struct MyPoly
{
   int vi0,ti0,ni0, fi0;
   int vi1,ti1,ni1, fi1;
   int vi2,ti2,ni2, fi2;
};
vector<MyVector> tmpVerts;
vector<MyVector> tmpNorms;
vector<MyVector> tmpTexel;
vector<MyPoly> tmpFaces;
vector<Vertex3D> verts;
vector<int> faces;

int isInList( const int vi, const int ti, const int ni )
{
   for( unsigned int i=0;i<verts.size();i++ )
   {
      int result=0;
      if( verts[i].x==tmpVerts[vi].x && verts[i].y==tmpVerts[vi].y && verts[i].z==tmpVerts[vi].z )
      {
         result|=1;
      }
      if( verts[i].tu==tmpTexel[ti].x && verts[i].tv==tmpTexel[ti].y )
      {
         result|=2;
      }
      if( verts[i].nx==tmpNorms[ni].x && verts[i].ny==tmpNorms[ni].y && verts[i].nz==tmpNorms[ni].z )
      {
         result|=4;
      }
      if( result==7 )
      {
         return i;
      }
   }
   return -1;
}
void NormalizeNormals()
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

bool loadWavefrontObj( char *filename, bool flipTv, bool convertToLeftHanded )
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

   // need some small data type 
   while ( 1 )
   {
      char lineHeader[128];
      int res = fscanf_s(f,"%s", lineHeader );
      if( res==EOF ) 
      {
         fclose(f);
         break;
      }

      if( strcmp( lineHeader,"v") == 0 )
      {
         MyVector tmp;
         fscanf_s(f, "%f %f %f\n",&tmp.x, &tmp.y, &tmp.z );
         if ( convertToLeftHanded )
            tmp.z*=-1.0f;
         tmpVerts.push_back(tmp);
      }
      if( strcmp( lineHeader,"vt") == 0 )
      {
         MyVector tmp;
         fscanf_s(f, "%f %f\n",&tmp.x, &tmp.y );
         if ( flipTv || convertToLeftHanded )
         {
            tmp.y = 1.f-tmp.y;
         }
         tmpTexel.push_back(tmp);
      }
      if( strcmp( lineHeader,"vn") == 0 )
      {
         MyVector tmp;
         fscanf_s(f, "%f %f %f\n",&tmp.x, &tmp.y, &tmp.z );
         if ( convertToLeftHanded )
            tmp.z*=-1;
         tmpNorms.push_back(tmp);
      }
      if( strcmp( lineHeader,"f") == 0 )
      {
         if( tmpVerts.size()==0 )
         {
            ShowError("No vertices found in obj file, import is impossible!");
            tmpNorms.clear();
            tmpTexel.clear();
            fclose(f);
            return false;
         }
         if( tmpTexel.size()==0 )
         {
            ShowError("No texture coordinates (UVs) found in obj file, import is impossible!");
		    tmpVerts.clear();
            tmpNorms.clear();
            fclose(f);
            return false;
         }
         if( tmpNorms.size()==0 )
         {
            ShowError("No normals found in obj file, import is impossible!");
            tmpVerts.clear();
            tmpTexel.clear();
			fclose(f);
            return false;
         }
         MyPoly tmpFace;
         tmpFace.fi0 = tmpFace.fi1 = tmpFace.fi2 = -1;
         int matches=0;
         if ( convertToLeftHanded )
         {
            matches = fscanf_s(f, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &tmpFace.vi2, &tmpFace.ti2, &tmpFace.ni2,
                                                                  &tmpFace.vi1, &tmpFace.ti1, &tmpFace.ni1,
                                                                  &tmpFace.vi0, &tmpFace.ti0, &tmpFace.ni0);
         }
         else
         {
            matches = fscanf_s(f, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &tmpFace.vi0, &tmpFace.ti0, &tmpFace.ni0,
                                                                  &tmpFace.vi1, &tmpFace.ti1, &tmpFace.ni1,
                                                                  &tmpFace.vi2, &tmpFace.ti2, &tmpFace.ni2);
         }

         if( matches!=9 )
         {
            ShowError("Face information incorrect! Each face needs vertices, UVs and normals!");
		    tmpVerts.clear();
            tmpNorms.clear();
            tmpTexel.clear();
            fclose(f);
            return false;
         }
         tmpFace.vi0--; tmpFace.vi1--; tmpFace.vi2--;
         tmpFace.ti0--; tmpFace.ti1--; tmpFace.ti2--;
         tmpFace.ni0--; tmpFace.ni1--; tmpFace.ni2--;
         tmpFaces.push_back(tmpFace);
      }
   }
   for( unsigned int i=0;i<tmpFaces.size(); i++ )
   {
      int idx=isInList(tmpFaces[i].vi0, tmpFaces[i].ti0, tmpFaces[i].ni0 );
      if( idx==-1 )
      {
         Vertex3D tmp;
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
         Vertex3D tmp;
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
         Vertex3D tmp;
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
}

Vertex3D_NoTex2 *GetVertices( int &numVertices ) // clears temporary storage on the way
{
   Vertex3D_NoTex2 * const objMesh = new Vertex3D_NoTex2[verts.size()];
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
   numVertices = verts.size();
   verts.clear();
   return objMesh;
}

WORD *GetIndexList( int &indexListSize ) // clears temporary storage on the way
{
   bool showerror = true;
   WORD * const list = new WORD[faces.size()];
   for( unsigned int i=0;i<faces.size();i++ )
   {
	  if((faces[i] >= 65536) && showerror) { //!! DX7 limit, delete later-on
           ShowError("Too many vertex indices in obj file");
		   showerror = false;
	  }
      list[i] = faces[i];
   }
   indexListSize = faces.size();
   if(indexListSize >= 65536) //!! DX7 limit, delete later-on
	   ShowError("Too many polygons in obj file, this can lead to driver problems");
   faces.clear();
   return list;
}

// exporting a mesh to a Wavefront .OBJ file. The mesh is converted into right-handed coordinate system (VP uses left-handed)
void SaveOBJ( char *filename, Primitive *mesh )
{
   FILE *f;
   fopen_s(&f,filename,"wt");
   if( !f )
      return ;

   fprintf_s(f,"# Visual Pinball OBJ file\n");
   fprintf_s(f,"# numVerts: %i numFaces: %i\n", mesh->numVertices, mesh->indexListSize );
   fprintf_s(f,"o %s\n",mesh->m_d.meshFileName );
   for( int i=0; i<mesh->numVertices;i++ )
   {
      float z = mesh->objMeshOrg[i].z;
      z*=-1.0f;
      fprintf_s(f,"v %f %f %f\n", mesh->objMeshOrg[i].x, mesh->objMeshOrg[i].y, z );
   }
   for( int i=0; i<mesh->numVertices;i++ )
   {
      fprintf_s(f,"vn %f %f %f\n",mesh->objMeshOrg[i].nx, mesh->objMeshOrg[i].ny, mesh->objMeshOrg[i].nz );
   }
   for( int i=0; i<mesh->numVertices;i++ )
   {
      float tv = 1.f-mesh->objMeshOrg[i].tv;
      fprintf_s(f,"vt %f %f\n", mesh->objMeshOrg[i].tu, tv );
   }

   for( int i=0; i<mesh->indexListSize;i+=3 )
   {
      fprintf_s(f,"f %i/%i/%i %i/%i/%i %i/%i/%i\n", mesh->indexList[i+2]+1, mesh->indexList[i+2]+1, mesh->indexList[i+2]+1
                                                  , mesh->indexList[i+1]+1, mesh->indexList[i+1]+1, mesh->indexList[i+1]+1
                                                  , mesh->indexList[i  ]+1, mesh->indexList[i  ]+1, mesh->indexList[i  ]+1 );
   }
   fclose(f);
}