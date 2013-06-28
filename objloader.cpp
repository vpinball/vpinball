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
vector<Vertex3D> tmpAllVerts;
vector<int> faces;

int isInList( int vi, int ti, int ni )
{
   for( unsigned int i=0;i<tmpAllVerts.size();i++ )
   {
      int result=0;
      if( tmpAllVerts[i].x==tmpVerts[vi].x && tmpAllVerts[i].y==tmpVerts[vi].y && tmpAllVerts[i].z==tmpVerts[vi].z )
      {
         result|=1;
      }
      if( tmpAllVerts[i].tu==tmpTexel[ti].x && tmpAllVerts[i].tv==tmpTexel[ti].y )
      {
         result|=2;
      }
      if( tmpAllVerts[i].nx==tmpNorms[ni].x && tmpAllVerts[i].ny==tmpNorms[ni].y && tmpAllVerts[i].nz==tmpNorms[ni].z )
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
   tmpAllVerts.clear();

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
         MyPoly tmpFace;
         tmpFace.fi0 = tmpFace.fi1 = tmpFace.fi2 = -1;
         int matches = fscanf_s(f, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &tmpFace.vi0, &tmpFace.ti0, &tmpFace.ni0,
                                                                   &tmpFace.vi1, &tmpFace.ti1, &tmpFace.ni1,
                                                                   &tmpFace.vi2, &tmpFace.ti2, &tmpFace.ni2);
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
         tmpAllVerts.push_back(tmp);
         faces.push_back( tmpAllVerts.size()-1 );
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
         tmpAllVerts.push_back(tmp);
         faces.push_back( tmpAllVerts.size()-1 );
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
         tmpAllVerts.push_back(tmp);
         faces.push_back( tmpAllVerts.size()-1 );
      }
      else
      {
         faces.push_back(idx);
      }
   }
   tmpVerts.clear();
   tmpTexel.clear();
   tmpNorms.clear();
   return true;
}

Vertex3D_NoTex2 *GetVertices( int &numVertices )
{
   Vertex3D_NoTex2 *objMesh = new Vertex3D_NoTex2[tmpAllVerts.size()];
   for( unsigned int i=0;i<tmpAllVerts.size();i++ )
   {
      objMesh[i].x = tmpAllVerts[i].x;
      objMesh[i].y = tmpAllVerts[i].y;
      objMesh[i].z = tmpAllVerts[i].z;
      objMesh[i].tu = tmpAllVerts[i].tu;
      objMesh[i].tv = tmpAllVerts[i].tv;
      objMesh[i].nx = tmpAllVerts[i].nx;
      objMesh[i].ny = tmpAllVerts[i].ny;
      objMesh[i].nz = tmpAllVerts[i].nz;
   }
   numVertices = tmpAllVerts.size();
   return objMesh;
}

WORD *GetIndexList( int &indexListSize )
{
   WORD *list = new WORD[faces.size()];
   for( unsigned int i=0;i<faces.size();i++ )
   {
      list[i] = faces[i];
   }
   indexListSize = faces.size();
   return list;
}