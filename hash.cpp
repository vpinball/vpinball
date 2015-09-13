#include "stdafx.h"

// this is the djb2 string hash algorithm
unsigned long StringHash(const unsigned char *str)
{
   //MessageBox(0, (char*)str, 0, 0);
   unsigned long hash = 5381;
   int c;

   while (c = *str++)
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

   return hash;
}
