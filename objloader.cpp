#include "stdafx.h"
#include "resource.h"

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

bool loadWavefrontObj( char *filename )
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
      int res = fscanf(f,"%s", lineHeader );
      if( res==EOF ) 
      {
         fclose(f);
         break;
      }

      if( strcmp( lineHeader,"v") == 0 )
      {
         MyVector tmp;
         fscanf_s(f, "%f %f %f\n",&tmp.x, &tmp.y, &tmp.z );
         tmpVerts.push_back(tmp);
      }
      if( strcmp( lineHeader,"vt") == 0 )
      {
         MyVector tmp;
         fscanf_s(f, "%f %f\n",&tmp.x, &tmp.y );
         tmp.y = 1-tmp.y;
         tmpTexel.push_back(tmp);
      }
      if( strcmp( lineHeader,"vn") == 0 )
      {
         MyVector tmp;
         fscanf_s(f, "%f %f %f\n",&tmp.x, &tmp.y, &tmp.z );
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
         int matches = fscanf_s(f, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &tmpFace.vi0, &tmpFace.ti0, &tmpFace.ni0,
                                                                   &tmpFace.vi1, &tmpFace.ti1, &tmpFace.ni1,
                                                                   &tmpFace.vi2, &tmpFace.ti2, &tmpFace.ni2);
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
   faces.clear();
   return list;
}
