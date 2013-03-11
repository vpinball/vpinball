#include "stdafx.h"
#pragma once

typedef struct  
{
   union {
      D3DCOLORVALUE   diffuse;        /* Diffuse color RGBA */
      D3DCOLORVALUE   dcvDiffuse;
	  __m128          d;
   };
   union {
      D3DCOLORVALUE   ambient;        /* Ambient color RGB */
      D3DCOLORVALUE   dcvAmbient;
	  __m128          a;
   };
   union {
      D3DCOLORVALUE   specular;       /* Specular 'shininess' */
      D3DCOLORVALUE   dcvSpecular;
	  __m128          s;
   };
   union {
      D3DCOLORVALUE   emissive;       /* Emissive color RGB */
      D3DCOLORVALUE   dcvEmissive;
   	  __m128          e;
   };
   union {
      D3DVALUE        power;          /* Sharpness if specular highlight */
      D3DVALUE        dvPower;
   };
}Material;

