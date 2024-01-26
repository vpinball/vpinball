/* POLE - Portable library to access OLE Storage 
   Copyright (C) 2002-2005 Ariya Hidayat <ariya@kde.org>

   Redistribution and use in source and binary forms, with or without 
   modification, are permitted provided that the following conditions 
   are met:
   * Redistributions of source code must retain the above copyright notice, 
     this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice, 
     this list of conditions and the following disclaimer in the documentation 
     and/or other materials provided with the distribution.
   * Neither the name of the authors nor the names of its contributors may be 
     used to endorse or promote products derived from this software without 
     specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
   THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <list>
#include <string>

#include "pole.h"

void visit( int indent, POLE::Storage* storage, std::string path )
{
  std::list<std::string> entries;
  entries = storage->entries( path );

  std::list<std::string>::iterator it;
  for( it = entries.begin(); it != entries.end(); ++it )
  {
    std::string name = *it;
    std::string fullname = path + name;
    for( int j = 0; j < indent; j++ ) std::cout << "    ";
    POLE::Stream* ss = new POLE::Stream( storage, fullname );
    std::cout << name;
    if( ss ) if( !ss->fail() )std::cout << "  (" << ss->size() << ")";
    std::cout << std::endl;
    delete ss;
    
    if( storage->isDirectory( fullname ) )
      visit( indent+1, storage, fullname + "/" );
  }
  
}

void dump( POLE::Storage* storage, char* stream_name )
{
  POLE::Stream* stream = new POLE::Stream( storage, stream_name );
  if( !stream ) return;
  if( stream->fail() ) return;
  
  // std::cout << "Size: " << stream->size() << " bytes" << std::endl;
  unsigned char buffer[16];
  for( ;; )
  {
      unsigned read = stream->read( buffer, sizeof( buffer ) );
      for( unsigned i = 0; i < read; i++ )
        printf( "%02x ", buffer[i] );
      std::cout << "    ";
      for( unsigned i = 0; i < read; i++ )
        printf( "%c", ((buffer[i]>=32)&&(buffer[i]<128)) ? buffer[i] : '.' );
      std::cout << std::endl;      
      if( read < sizeof( buffer ) ) break;
  }
  
  delete stream;
}

void extract( POLE::Storage* storage, char* stream_name, char* outfile )
{
  POLE::Stream* stream = new POLE::Stream( storage, stream_name );
  if( !stream ) return;
  if( stream->fail() ) return;
  
  std::ofstream file;
  file.open( outfile, std::ios::binary|std::ios::out );
  
  unsigned char buffer[16];
  for( ;; )
  {
      unsigned read = stream->read( buffer, sizeof( buffer ) );
      file.write( (const char*)buffer, read  );
      if( read < sizeof( buffer ) ) break;
  }
  file.close();
  
  delete stream;
}

int main(int argc, char *argv[])
{
  if( argc < 2 )
  {
    std::cout << "Usage:" << std::endl;
    std::cout << argv[0] << " filename [stream-name [output-file]]" << std::endl;
    return 0;
  }
  
  char* filename = argv[1];
  char* streamname = (argc<3) ? 0 : argv[2];
  char* outfile = (argc<4) ? 0 : argv[3];

  POLE::Storage* storage = new POLE::Storage( filename );
  storage->open();
  if( storage->result() != POLE::Storage::Ok )
  {
    std::cout << "Error on file " << filename << std::endl;
    return 1;
  }
  
  if( !streamname )
    visit( 0, storage, "/" );
  else if( !outfile )
     dump( storage, streamname );
  else
     extract( storage, streamname, outfile );
    
  delete storage;
  
  return 0;
}
