#include "stdafx.h"

// this is the djb2 string hash algorithm, str is converted to lower case
unsigned long StringHash(const unsigned char *str)
{
   //MessageBox(0, str, 0, 0);
   unsigned long hash = 5381;
   int c;

   while (c = *str++)
      hash = ((hash << 5) + hash) + tolower(c); /* hash * 33 + c */

   return hash;
}
