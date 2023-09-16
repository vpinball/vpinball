/* POLE - Portable C++ library to access OLE Storage 
   Copyright (C) 2002-2005 Ariya Hidayat <ariya@kde.org>

   Performance optimization: Dmitry Fedorov 
   Copyright 2009 <www.bioimage.ucsb.edu> <www.dimin.net> 

   Fix for more than 236 mbat block entries : Michel Boudinot
   Copyright 2010 <Michel.Boudinot@inaf.cnrs-gif.fr>

   Considerable rework to allow for creation and updating of structured storage : Stephen Baum
   Copyright 2013 <srbaum@gmail.com>

   Added GetAllStreams, reworked datatypes
   Copyright 2013 Felix Gorny from Bitplane
   
   More datatype changes to allow for 32 and 64 bit code, some fixes involving incremental updates, flushing
   Copyright 2013 <srbaum@gmail.com>
   
   Version: 0.5.2

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

#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <queue>
#include <limits>

#include <cstring>

#include "pole.h"

#ifdef POLE_USE_UTF16_FILENAMES
#include <codecvt>
#endif //POLE_USE_UTF16_FILENAMES

// enable to activate debugging output
// #define POLE_DEBUG
#define CACHEBUFSIZE 4096 //a presumably reasonable size for the read cache

namespace POLE
{

class Header
{
  public:
    unsigned char id[8];       // signature, or magic identifier
    uint64 b_shift;          // bbat->blockSize = 1 << b_shift
    uint64 s_shift;          // sbat->blockSize = 1 << s_shift
    uint64 num_bat;          // blocks allocated for big bat
    uint64 dirent_start;     // starting block for directory info
    uint64 threshold;        // switch from small to big file (usually 4K)
    uint64 sbat_start;       // starting block index to store small bat
    uint64 num_sbat;         // blocks allocated for small bat
    uint64 mbat_start;       // starting block to store meta bat
    uint64 num_mbat;         // blocks allocated for meta bat
    uint64 bb_blocks[109];
    bool dirty;                // Needs to be written
    
    Header();
    bool valid();
    void load( const unsigned char* buffer );
    void save( unsigned char* buffer );
    void debug();
};

class AllocTable
{
  public:
    static const uint64 Eof;
    static const uint64 Avail;
    static const uint64 Bat;
    static const uint64 MetaBat;
    uint64 blockSize;
    AllocTable();
    void clear();
    uint64 count();
    uint64 unusedCount();
    void resize( uint64 newsize );
    void preserve( uint64 n );
    void set( uint64 index, uint64 val );
    unsigned unused();
    void setChain( std::vector<uint64> );
    std::vector<uint64> follow( uint64 start );
    uint64 operator[](uint64 index );
    void load( const unsigned char* buffer, uint64 len );
    void save( unsigned char* buffer );
    uint64 size();
    void debug();
    bool isDirty();
    void markAsDirty(uint64 dataIndex, int64 bigBlockSize);
    void flush(std::vector<uint64> blocks, StorageIO *const io, int64 bigBlockSize);
  private:
    std::vector<uint64> data;
    std::vector<uint64> dirtyBlocks;
    bool bMaybeFragmented;
    AllocTable( const AllocTable& );
    AllocTable& operator=( const AllocTable& );
};

class DirEntry
{
  public:
    DirEntry(): valid(), name(), dir(), size(), start(), prev(), next(), child() {}
    bool valid;          // false if invalid (should be skipped)
    std::string name;    // the name, not in unicode anymore 
    bool dir;            // true if directory   
    uint64 size;         // size (not valid if directory)
    uint64 start;        // starting block
    uint64 prev;         // previous sibling
    uint64 next;         // next sibling
    uint64 child;        // first child
    int compare(const DirEntry& de);
    int compare(const std::string& name2);

};

class DirTree
{
  public:
    static const uint64 End;
    DirTree(int64 bigBlockSize);
    void clear(int64 bigBlockSize);
    inline uint64 entryCount();
    uint64 unusedEntryCount();
    DirEntry* entry( uint64 index );
    DirEntry* entry( const std::string& name, bool create = false, int64 bigBlockSize = 0, StorageIO *const io = 0, int64 streamSize = 0);
    int64 indexOf( DirEntry* e );
    int64 parent( uint64 index );
    std::string fullName( uint64 index );
    std::vector<uint64> children( uint64 index );
    uint64 find_child( uint64 index, const std::string& name, uint64 &closest );
    void load( unsigned char* buffer, uint64 len );
    void save( unsigned char* buffer );
    uint64 size();
    void debug();
    bool isDirty();
    void markAsDirty(uint64 dataIndex, int64 bigBlockSize);
    void flush(std::vector<uint64> blocks, StorageIO *const io, int64 bigBlockSize, uint64 sb_start, uint64 sb_size);
    uint64 unused();
    void findParentAndSib(uint64 inIdx, const std::string& inFullName, uint64 &parentIdx, uint64 &sibIdx);
    uint64 findSib(uint64 inIdx, uint64 sibIdx);
    void deleteEntry(DirEntry *entry, const std::string& inFullName, int64 bigBlockSize);
  private:
    std::vector<DirEntry> entries;
    std::vector<uint64> dirtyBlocks;
    DirTree( const DirTree& );
    DirTree& operator=( const DirTree& );
};

class StorageIO
{
  public:
    Storage* storage;         // owner
    std::string filename;     // filename
    std::fstream file;        // associated with above name
    int64 result;               // result of operation
    bool opened;              // true if file is opened
    uint64 filesize;   // size of the file
    bool writeable;           // true if the file can be modified
    
    Header* header;           // storage header 
    DirTree* dirtree;         // directory tree
    AllocTable* bbat;         // allocation table for big blocks
    AllocTable* sbat;         // allocation table for small blocks
    
    std::vector<uint64> sb_blocks; // blocks for "small" files
    std::vector<uint64> mbat_blocks; // blocks for doubly indirect indices to big blocks
    std::vector<uint64> mbat_data; // the additional indices to big blocks
    bool mbatDirty;           // If true, mbat_blocks need to be written
       
    std::list<Stream*> streams;

    StorageIO( Storage* storage, const char* filename );
    ~StorageIO();
    
    bool open(bool bWriteAccess = false, bool bCreate = false);
    void close();
    void flush();
    void load(bool bWriteAccess);
    void create();
    void init();
    bool deleteByName(const std::string& fullName);

    bool deleteNode(DirEntry *entry, const std::string& fullName);

    bool deleteLeaf(DirEntry *entry, const std::string& fullName);

    uint64 loadBigBlocks( std::vector<uint64> blocks, unsigned char* buffer, uint64 maxlen );

    uint64 loadBigBlock( uint64 block, unsigned char* buffer, uint64 maxlen );

    uint64 saveBigBlocks( std::vector<uint64> blocks, uint64 offset, unsigned char* buffer, uint64 len );

    uint64 saveBigBlock( uint64 block, uint64 offset, unsigned char*buffer, uint64 len );

    uint64 loadSmallBlocks( std::vector<uint64> blocks, unsigned char* buffer, uint64 maxlen );

    uint64 loadSmallBlock( uint64 block, unsigned char* buffer, uint64 maxlen );
    
    uint64 saveSmallBlocks( std::vector<uint64> blocks, uint64 offset, unsigned char* buffer, uint64 len, int64 startAtBlock = 0  );

    uint64 saveSmallBlock( uint64 block, uint64 offset, unsigned char* buffer, uint64 len );
    
    StreamIO* streamIO( const std::string& name, bool bCreate = false, int64 streamSize = 0 ); 

    void flushbbat();

    void flushsbat();

    std::vector<uint64> getbbatBlocks(bool bLoading);

    uint64 ExtendFile( std::vector<uint64> *chain );

    void addbbatBlock();

  private:  
    // no copy or assign
    StorageIO( const StorageIO& );
    StorageIO& operator=( const StorageIO& );

};

class StreamIO
{
  public:
    StorageIO* io;
    int64 entryIdx; //needed because a pointer to DirEntry will change whenever entries vector changes.
    std::string fullName;
    bool eof;
    bool fail;

    StreamIO( StorageIO* io, DirEntry* entry );
    ~StreamIO();
    uint64 size();
    void setSize(uint64 newSize);
    void seek( uint64 pos );
    uint64 tell();
    int64 getch();
    uint64 read( unsigned char* data, uint64 maxlen );
    uint64 read( uint64 pos, unsigned char* data, uint64 maxlen );
    uint64 write( unsigned char* data, uint64 len );
    uint64 write( uint64 pos, unsigned char* data, uint64 len );
    void flush();

  private:
    std::vector<uint64> blocks;

    // no copy or assign
    StreamIO( const StreamIO& );
    StreamIO& operator=( const StreamIO& );

    // pointer for read
    uint64 m_pos;

    // simple cache system to speed-up getch()
    unsigned char* cache_data;
    uint64 cache_size;
    uint64 cache_pos;
    void updateCache();
};

}; // namespace POLE

using namespace POLE;

#ifdef POLE_USE_UTF16_FILENAMES

std::string POLE::UTF16toUTF8(const std::wstring &utf16) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
    return converter.to_bytes(utf16);
}

std::wstring POLE::UTF8toUTF16(const std::string &utf8) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
    return converter.from_bytes(utf8);
}

#endif //POLE_USE_UTF16_FILENAMES

static void fileCheck(std::fstream &file)
{
    bool bGood, bFail, bEof, bBad;
    bool bNOTOK;
    bGood = file.good();
    bFail = file.fail();
    bEof = file.eof();
    bBad = file.bad();
    if (bFail || bEof || bBad)
        bNOTOK = true; //this doesn't really do anything, but it is a good place to set a breakpoint!
    file.clear();
}
    

static inline uint32 readU16( const unsigned char* ptr )
{
  return ptr[0]+(ptr[1]<<8);
}

static inline uint32 readU32( const unsigned char* ptr )
{
  return ptr[0]+(ptr[1]<<8)+(ptr[2]<<16)+(ptr[3]<<24);
}

static inline void writeU16( unsigned char* ptr, uint32 data )
{
  ptr[0] = (unsigned char)(data & 0xff);
  ptr[1] = (unsigned char)((data >> 8) & 0xff);
}

static inline void writeU32( unsigned char* ptr, uint32 data )
{
  ptr[0] = (unsigned char)(data & 0xff);
  ptr[1] = (unsigned char)((data >> 8) & 0xff);
  ptr[2] = (unsigned char)((data >> 16) & 0xff);
  ptr[3] = (unsigned char)((data >> 24) & 0xff);
}

static const unsigned char pole_magic[] = 
 { 0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1, 0x1a, 0xe1 };

// =========== Header ==========

Header::Header()
:   b_shift(9),                 // [1EH,02] size of sectors in power-of-two; typically 9 indicating 512-byte sectors
    s_shift(6),                 // [20H,02] size of mini-sectors in power-of-two; typically 6 indicating 64-byte mini-sectors
    num_bat(0),                 // [2CH,04] number of SECTs in the FAT chain
    dirent_start(0),            // [30H,04] first SECT in the directory chain
    threshold(4096),            // [38H,04] maximum size for a mini stream; typically 4096 bytes
    sbat_start(0),              // [3CH,04] first SECT in the MiniFAT chain
    num_sbat(0),                // [40H,04] number of SECTs in the MiniFAT chain
    mbat_start(AllocTable::Eof),// [44H,04] first SECT in the DIFAT chain
    num_mbat(0),                // [48H,04] number of SECTs in the DIFAT chain
    dirty(true)	

{
  for( unsigned int i = 0; i < 8; i++ )
    id[i] = pole_magic[i];  
  for( unsigned int i=0; i<109; i++ )
    bb_blocks[i] = AllocTable::Avail;
}

bool Header::valid()
{
  if( threshold != 4096 ) return false;
  if( num_bat == 0 ) return false;
  //if( (num_bat > 109) && (num_bat > (num_mbat * 127) + 109)) return false; // dima: incorrect check, number may be arbitrary larger
  if( (num_bat < 109) && (num_mbat != 0) ) return false;
  if( s_shift > b_shift ) return false;
  if( b_shift <= 6 ) return false;
  if( b_shift >=31 ) return false;
  
  return true;
}

void Header::load( const unsigned char* buffer ) {
  b_shift      = readU16( buffer + 0x1e ); // [1EH,02] size of sectors in power-of-two; typically 9 indicating 512-byte sectors and 12 for 4096
  s_shift      = readU16( buffer + 0x20 ); // [20H,02] size of mini-sectors in power-of-two; typically 6 indicating 64-byte mini-sectors
  num_bat      = readU32( buffer + 0x2c ); // [2CH,04] number of SECTs in the FAT chain
  dirent_start = readU32( buffer + 0x30 ); // [30H,04] first SECT in the directory chain
  threshold    = readU32( buffer + 0x38 ); // [38H,04] maximum size for a mini stream; typically 4096 bytes
  sbat_start   = readU32( buffer + 0x3c ); // [3CH,04] first SECT in the MiniFAT chain
  num_sbat     = readU32( buffer + 0x40 ); // [40H,04] number of SECTs in the MiniFAT chain
  mbat_start   = readU32( buffer + 0x44 ); // [44H,04] first SECT in the DIFAT chain
  num_mbat     = readU32( buffer + 0x48 ); // [48H,04] number of SECTs in the DIFAT chain
  
  for( unsigned int i = 0; i < 8; i++ )
    id[i] = buffer[i]; 

  // [4CH,436] the SECTs of first 109 FAT sectors
  for( unsigned int i=0; i<109; i++ )
    bb_blocks[i] = readU32( buffer + 0x4C+i*4 );
  dirty = false;
}

void Header::save( unsigned char* buffer )
{
  memset( buffer, 0, 0x4c );
  memcpy( buffer, pole_magic, 8 );        // ole signature
  writeU32( buffer + 8, 0 );              // unknown 
  writeU32( buffer + 12, 0 );             // unknown
  writeU32( buffer + 16, 0 );             // unknown
  writeU16( buffer + 24, 0x003e );        // revision ?
  writeU16( buffer + 26, 3 );             // version ?
  writeU16( buffer + 28, 0xfffe );        // unknown
  writeU16( buffer + 0x1e, (uint32) b_shift );
  writeU16( buffer + 0x20, (uint32) s_shift );
  writeU32( buffer + 0x2c, (uint32) num_bat );
  writeU32( buffer + 0x30, (uint32) dirent_start );
  writeU32( buffer + 0x38, (uint32) threshold );
  writeU32( buffer + 0x3c, (uint32) sbat_start );
  writeU32( buffer + 0x40, (uint32) num_sbat );
  writeU32( buffer + 0x44, (uint32) mbat_start );
  writeU32( buffer + 0x48, (uint32) num_mbat );
  
  for( unsigned int i=0; i<109; i++ )
    writeU32( buffer + 0x4C+i*4, (uint32) bb_blocks[i] );
  dirty = false;
}

void Header::debug()
{
  std::cout << std::endl;
  std::cout << "b_shift " << b_shift << std::endl;
  std::cout << "s_shift " << s_shift << std::endl;
  std::cout << "num_bat " << num_bat << std::endl;
  std::cout << "dirent_start " << dirent_start << std::endl;
  std::cout << "threshold " << threshold << std::endl;
  std::cout << "sbat_start " << sbat_start << std::endl;
  std::cout << "num_sbat " << num_sbat << std::endl;
  std::cout << "mbat_start " << mbat_start << std::endl;
  std::cout << "num_mbat " << num_mbat << std::endl;
  
  uint64 s = (num_bat<=109) ? num_bat : 109;
  std::cout << "bat blocks: ";
  for( uint64 i = 0; i < s; i++ )
    std::cout << bb_blocks[i] << " ";
  std::cout << std::endl;
}
 
// =========== AllocTable ==========

const uint64 AllocTable::Avail = 0xffffffff;
const uint64 AllocTable::Eof = 0xfffffffe;
const uint64 AllocTable::Bat = 0xfffffffd;
const uint64 AllocTable::MetaBat = 0xfffffffc;

AllocTable::AllocTable()
:   blockSize(4096),
    data(),
    dirtyBlocks(),
    bMaybeFragmented(true)
{
  // initial size
  resize( 128 );
}

uint64 AllocTable::count()
{
  return static_cast<uint64>(data.size());
}

uint64 AllocTable::unusedCount()
{
    uint64 maxIdx = count();
    uint64 nFound = 0;
    for (uint64 idx = 0; idx < maxIdx; idx++)
    {
        if( data[idx] == Avail )
            nFound++;
    }
    return nFound;
}

void AllocTable::resize( uint64 newsize )
{
  uint64 oldsize = static_cast<uint64>(data.size());
  data.resize( newsize );
  if( newsize > oldsize )
    for( uint64 i = oldsize; i<newsize; i++ )
      data[i] = Avail;
}

// make sure there're still free blocks
void AllocTable::preserve( uint64 n )
{
  std::vector<uint64> pre;
  for( unsigned i=0; i < n; i++ )
    pre.push_back( unused() );
}

uint64 AllocTable::operator[]( uint64 index )
{
  uint64 result;
  result = data[index];
  return result;
}

void AllocTable::set( uint64 index, uint64 value )
{
  if( index >= count() ) resize( index + 1);
  data[ index ] = value;
  if (value == Avail)
      bMaybeFragmented = true;
}

void AllocTable::setChain( std::vector<uint64> chain )
{
  if( chain.size() )
  {
    for( unsigned i=0; i<chain.size()-1; i++ )
      set( chain[i], chain[i+1] );
    set( chain[ chain.size()-1 ], AllocTable::Eof );
  }
}

// follow 
std::vector<uint64> AllocTable::follow( uint64 start )
{
  std::vector<uint64> chain;

  if( start >= count() ) return chain; 

  uint64 p = start;
  while( p < count() )
  {
    if( p == (uint64)Eof ) break;
    if( p == (uint64)Bat ) break;
    if( p == (uint64)MetaBat ) break;
    if( p >= count() ) break;
    chain.push_back( p );
    if( data[p] >= count() ) break;
    p = data[ p ];
  }

  return chain;
}

unsigned AllocTable::unused()
{
  // find first available block
  unsigned int maxIdx = (unsigned int) data.size();
  if (bMaybeFragmented)
  {
      for( unsigned i = 0; i < maxIdx; i++ )
        if( data[i] == Avail )
          return i;
  }
  
  // completely full, so enlarge the table
  unsigned int block = maxIdx;
  resize( maxIdx );
  bMaybeFragmented = false;
  return block;      
}

void AllocTable::load( const unsigned char* buffer, uint64 len )
{
  resize( len / 4 );
  for( unsigned i = 0; i < count(); i++ )
    set( i, readU32( buffer + i*4 ) );
}

// return space required to save this dirtree
uint64 AllocTable::size()
{
  return count() * 4;
}

void AllocTable::save( unsigned char* buffer )
{
  for( uint64 i = 0; i < count(); i++ )
    writeU32( buffer + i*4, (uint32) data[i] );
}

bool AllocTable::isDirty()
{
    return (dirtyBlocks.size() > 0);
}

void AllocTable::markAsDirty(uint64 dataIndex, int64 bigBlockSize)
{
    uint64 dbidx = dataIndex / (bigBlockSize / sizeof(uint32));
    for (uint64 idx = 0; idx < static_cast<uint64>(dirtyBlocks.size()); idx++)
    {
        if (dirtyBlocks[idx] == dbidx)
            return;
    }
    dirtyBlocks.push_back(dbidx);
}

void AllocTable::flush(std::vector<uint64> blocks, StorageIO *const io, int64 bigBlockSize)
{
    unsigned char *buffer = new unsigned char[bigBlockSize * blocks.size()];
    save(buffer);
    for (uint64 idx = 0; idx < static_cast<uint64>(blocks.size()); idx++)
    {
        bool bDirty = false;
        for (uint64 idx2 = 0; idx2 < static_cast<uint64>(dirtyBlocks.size()); idx2++)
        {
            if (dirtyBlocks[idx2] == idx)
            {
                bDirty = true;
                break;
            }
        }
        if (bDirty)
            io->saveBigBlock(blocks[idx], 0, &buffer[bigBlockSize*idx], bigBlockSize);
    }
    dirtyBlocks.clear();
    delete[] buffer;
}

void AllocTable::debug()
{
  std::cout << "block size " << data.size() << std::endl;
  for( unsigned i=0; i< data.size(); i++ )
  {
     if( data[i] == Avail ) continue;
     std::cout << i << ": ";
     if( data[i] == Eof ) std::cout << "[eof]";
     else if( data[i] == Bat ) std::cout << "[bat]";
     else if( data[i] == MetaBat ) std::cout << "[metabat]";
     else std::cout << data[i];
     std::cout << std::endl;
  }
}

// =========== DirEntry ==========
// "A node with a shorter name is less than a node with a inter name"
// "For nodes with the same length names, compare the two names." 
// --Windows Compound Binary File Format Specification, Section 2.5
int DirEntry::compare(const DirEntry& de)
{
    return compare(de.name);
}

int DirEntry::compare(const std::string& name2)
{
    if (name.length() < name2.length())
        return -1;
    else if (name.length() > name2.length())
        return 1;
    else
        return name.compare(name2);
}


// =========== DirTree ==========

const uint64 DirTree::End = 0xffffffff;

DirTree::DirTree(int64 bigBlockSize)
:   entries(),
    dirtyBlocks()
{
  clear(bigBlockSize);
}

void DirTree::clear(int64 bigBlockSize)
{
  // leave only root entry
  entries.resize( 1 );
  entries[0].valid = true;
  entries[0].name = "Root Entry";
  entries[0].dir = true;
  entries[0].size = 0;
  entries[0].start = End;
  entries[0].prev = End;
  entries[0].next = End;
  entries[0].child = End;
  markAsDirty(0, bigBlockSize);
}

inline uint64 DirTree::entryCount()
{
  return entries.size();
}

uint64 DirTree::unusedEntryCount()
{
    uint64 nFound = 0;
    for (uint64 idx = 0; idx < entryCount(); idx++)
    {
        if (!entries[idx].valid)
            nFound++;
    }
    return nFound;
}

DirEntry* DirTree::entry( uint64 index )
{
  if( index >= entryCount() ) return (DirEntry*) 0;
  return &entries[ index ];
}

int64 DirTree::indexOf( DirEntry* e )
{
  for( uint64 i = 0; i < entryCount(); i++ )
    if( entry( i ) == e ) return i;
    
  return -1;
}

int64 DirTree::parent( uint64 index )
{
  // brute-force, basically we iterate for each entries, find its children
  // and check if one of the children is 'index'
  for( uint64 j=0; j<entryCount(); j++ )
  {
    std::vector<uint64> chi = children( j );
    for( unsigned i=0; i<chi.size();i++ )
      if( chi[i] == index )
        return j;
  }
        
  return -1;
}

std::string DirTree::fullName( uint64 index )
{
  // don't use root name ("Root Entry"), just give "/"
  if( index == 0 ) return "/";

  std::string result = entry( index )->name;
  result.insert( 0,  "/" );
  uint64 p = parent( index );
  DirEntry * _entry = 0;
  while( p > 0 )
  {
    _entry = entry( p );
    if (_entry->dir && _entry->valid)
    {
      result.insert( 0,  _entry->name);
      result.insert( 0,  "/" );
    }
    --p;
    index = p;
    if( index <= 0 ) break;
  }
  return result;
}

// given a fullname (e.g "/ObjectPool/_1020961869"), find the entry
// if not found and create is false, return 0
// if create is true, a new entry is returned
DirEntry* DirTree::entry( const std::string& name, bool create, int64 bigBlockSize, StorageIO *const io, int64 streamSize)
{
   if( !name.length() ) return (DirEntry*)0;
 
   // quick check for "/" (that's root)
   if( name == "/" ) return entry( 0 );
   
   // split the names, e.g  "/ObjectPool/_1020961869" will become:
   // "ObjectPool" and "_1020961869" 
   std::list<std::string> names;
   std::string::size_type start = 0, end = 0;
   if( name[0] == '/' ) start++;
   int levelsLeft = 0;
   while( start < name.length() )
   {
     end = name.find_first_of( '/', start );
     if( end == std::string::npos ) end = name.length();
     names.push_back( name.substr( start, end-start ) );
     levelsLeft++;
     start = end+1;
   }
  
   // start from root 
   int64 index = 0 ;

   // trace one by one   
   std::list<std::string>::iterator it; 

   for( it = names.begin(); it != names.end(); ++it )
   {
     // find among the children of index
     levelsLeft--;
     uint64 child = 0;

     
     /*
     // dima: this block is really inefficient
     std::vector<unsigned> chi = children( index );
     for( unsigned i = 0; i < chi.size(); i++ )
     {
       DirEntry* ce = entry( chi[i] );
       if( ce ) 
       if( ce->valid && ( ce->name.length()>1 ) )
       if( ce->name == *it ) {
             child = chi[i];
             break;
       }
     }
     */
     // dima: performance optimisation of the previous
     uint64 closest = End;
     child = find_child( index, *it, closest );
     
     // traverse to the child
     if( child > 0 ) index = child;
     else
     {
       // not found among children
       if( !create || !io->writeable) return (DirEntry*)0;
       
       // create a new entry
       uint64 parent2 = index;
       index = unused();
       DirEntry* e = entry( index );
       e->valid = true;
       e->name = *it;
       e->dir = (levelsLeft > 0);
       if (!e->dir)
           e->size = streamSize;
       else
           e->size = 0;
       e->start = AllocTable::Eof;
       e->child = End;
       if (closest == End)
       {
           e->prev = End;
           e->next = entry(parent2)->child;
           entry(parent2)->child = index;
           markAsDirty(parent2, bigBlockSize);
       }
       else
       {
           DirEntry* closeE = entry( closest );
           if (closeE->compare(*e) < 0)
           {
               e->prev = closeE->next;
               e->next = End;
               closeE->next = index;
           }
           else
           {
               e->next = closeE->prev;
               e->prev = End;
               closeE->prev = index;
           }
           markAsDirty(closest, bigBlockSize);
       }
       markAsDirty(index, bigBlockSize);
       uint64 bbidx = index / (bigBlockSize / 128);
       std::vector <uint64> blocks = io->bbat->follow(io->header->dirent_start);
       while (blocks.size() <= bbidx)
       {
           uint64 nblock = io->bbat->unused();
           if (blocks.size() > 0)
           {
               io->bbat->set(blocks[static_cast<uint64>(blocks.size())-1], nblock);
               io->bbat->markAsDirty(blocks[static_cast<uint64>(blocks.size())-1], bigBlockSize);
           }
           io->bbat->set(nblock, AllocTable::Eof);
           io->bbat->markAsDirty(nblock, bigBlockSize);
           blocks.push_back(nblock);
           uint64 bbidx = nblock / (io->bbat->blockSize / sizeof(uint64));
           while (bbidx >= io->header->num_bat)
               io->addbbatBlock();
       }
     }
   }

   return entry( index );
}

// helper function: recursively find siblings of index
void dirtree_find_siblings( DirTree* dirtree, std::vector<uint64>& result, 
  uint64 index )
{
    DirEntry* e = dirtree->entry( index );
    if (!e) return;
    if (e->prev != DirTree::End)
        dirtree_find_siblings(dirtree, result, e->prev);
    result.push_back(index);
    if (e->next != DirTree::End)
        dirtree_find_siblings(dirtree, result, e->next);
}

std::vector<uint64> DirTree::children( uint64 index )
{
  std::vector<uint64> result;
  
  DirEntry* e = entry( index );
  if( e ) if( e->valid && e->child < entryCount() )
    dirtree_find_siblings( this, result, e->child );
    
  return result;
}

uint64 dirtree_find_sibling( DirTree* dirtree, uint64 index, const std::string& name, uint64& closest ) {

    uint64 count = dirtree->entryCount();
    DirEntry* e = dirtree->entry( index );
    if (!e || !e->valid) return 0;
    int cval = e->compare(name);
    if (cval == 0)
        return index;
    if (cval > 0)
    {
        if (e->prev > 0 && e->prev < count)
            return dirtree_find_sibling( dirtree, e->prev, name, closest );
    }
    else
    {
        if (e->next > 0 && e->next < count)
            return dirtree_find_sibling( dirtree, e->next, name, closest );
    }
    closest = index;
    return 0;
}

uint64 DirTree::find_child( uint64 index, const std::string& name, uint64& closest ) {

  uint64 count = entryCount();
  DirEntry* p = entry( index );
  if (p && p->valid && p->child < count )
    return dirtree_find_sibling( this, p->child, name, closest );
  
  return 0;
}

void DirTree::load( unsigned char* buffer, uint64 size )
{
  entries.clear();
  
  for( uint64 i = 0; i < size/128; i++ )
  {
    uint64 p = i * 128;
    
    // would be < 32 if first char in the name isn't printable
    unsigned prefix = 32;
    
    // parse name of this entry, which stored as Unicode 16-bit
    std::string name;
    int name_len = readU16( buffer + 0x40+p );
    if( name_len > 64 ) name_len = 64;
    for( int j=0; ( buffer[j+p]) && (j<name_len); j+= 2 )
      name.append( 1, buffer[j+p] );
      
    // first char isn't printable ? remove it...
    if( buffer[p] < 32 )
    { 
      prefix = buffer[0]; 
      name.erase( 0,1 ); 
    }
    
    // 2 = file (aka stream), 1 = directory (aka storage), 5 = root
    unsigned type = buffer[ 0x42 + p];
    
    DirEntry e;
    e.valid = ( type != 0 );
    e.name = name;
    e.start = readU32( buffer + 0x74+p );
    e.size = readU32( buffer + 0x78+p );
    e.prev = readU32( buffer + 0x44+p );
    e.next = readU32( buffer + 0x48+p );
    e.child = readU32( buffer + 0x4C+p );
    e.dir = ( type!=2 );
    
    // sanity checks
    if( (type != 2) && (type != 1 ) && (type != 5 ) ) e.valid = false;
    if( name_len < 1 ) e.valid = false;
    
    entries.push_back( e );
  }  
}

// return space required to save this dirtree
uint64 DirTree::size()
{
  return entryCount() * 128;
}

void DirTree::save( unsigned char* buffer )
{
  memset( buffer, 0, size() );
  
  // root is fixed as "Root Entry"
  DirEntry* root = entry( 0 );
  std::string name = "Root Entry";
  for( unsigned int j = 0; j < name.length(); j++ )
    buffer[ j*2 ] = name[j];
  writeU16( buffer + 0x40, static_cast<uint32>(name.length()*2 + 2) );
  writeU32( buffer + 0x74, 0xffffffff );
  writeU32( buffer + 0x78, 0 );
  writeU32( buffer + 0x44, 0xffffffff );
  writeU32( buffer + 0x48, 0xffffffff );
  writeU32( buffer + 0x4c, (uint32) root->child );
  buffer[ 0x42 ] = 5;
  //buffer[ 0x43 ] = 1; 

  for( unsigned int i = 1; i < entryCount(); i++ )
  {
    DirEntry* e = entry( i );
    if( !e ) continue;
    if( e->dir )
    {
      e->start = 0xffffffff;
      e->size = 0;
    }
    
    // max length for name is 32 chars
    std::string name = e->name;
    if( name.length() > 32 )
      name.erase( 32, name.length() );
      
    // write name as Unicode 16-bit
    for( unsigned j = 0; j < name.length(); j++ )
      buffer[ i*128 + j*2 ] = name[j];

    writeU16( buffer + i*128 + 0x40, static_cast<uint32>(name.length()*2 + 2) );
    writeU32( buffer + i*128 + 0x74, (uint32) e->start );
    writeU32( buffer + i*128 + 0x78, (uint32) e->size );
    writeU32( buffer + i*128 + 0x44, (uint32) e->prev );
    writeU32( buffer + i*128 + 0x48, (uint32) e->next );
    writeU32( buffer + i*128 + 0x4c, (uint32) e->child );
    if (!e->valid)
        buffer[ i*128 + 0x42 ] = 0; //STGTY_INVALID
    else
        buffer[ i*128 + 0x42 ] = e->dir ? 1 : 2; //STGTY_STREAM or STGTY_STORAGE
    buffer[ i*128 + 0x43 ] = 1; // always black
  }  
}

bool DirTree::isDirty()
{
    return (dirtyBlocks.size() > 0);
}


void DirTree::markAsDirty(uint64 dataIndex, int64 bigBlockSize)
{
    uint64 dbidx = dataIndex / (bigBlockSize / 128);
    for (uint64 idx = 0; idx < static_cast<uint64>(dirtyBlocks.size()); idx++)
    {
        if (dirtyBlocks[idx] == dbidx)
            return;
    }
    dirtyBlocks.push_back(dbidx);
}

void DirTree::flush(std::vector<uint64> blocks, StorageIO *const io, int64 bigBlockSize, uint64 sb_start, uint64 sb_size)
{
    uint64 bufLen = size();
    unsigned char *buffer = new unsigned char[bufLen];
    save(buffer);
    writeU32( buffer + 0x74, (uint32) sb_start );
    writeU32( buffer + 0x78, (uint32) sb_size );
    for (uint64 idx = 0; idx < static_cast<uint64>(blocks.size()); idx++)
    {
        bool bDirty = false;
        for (uint64 idx2 = 0; idx2 < static_cast<uint64>(dirtyBlocks.size()); idx2++)
        {
            if (dirtyBlocks[idx2] == idx)
            {
                bDirty = true;
                break;
            }
        }
        uint64 bytesToWrite = bigBlockSize;
        uint64 pos = bigBlockSize*idx;
        if ((bufLen - pos) < bytesToWrite)
            bytesToWrite = bufLen - pos;
        if (bDirty)
            io->saveBigBlock(blocks[idx], 0, &buffer[pos], bytesToWrite);
    }
    dirtyBlocks.clear();
    delete[] buffer;
}

uint64 DirTree::unused()
{
    for (uint64 idx = 0; idx < static_cast<uint64>(entryCount()); idx++)
    {
        if (!entries[idx].valid)
            return idx;
    }
    entries.push_back(DirEntry());
    return entryCount()-1;
}

// Utility function to get the index of the parent dirEntry, given that we already have a full name it is relatively fast.
// Then look for a sibling dirEntry that points to inIdx. In some circumstances, the dirEntry at inIdx will be the direct child
// of the parent, in which case sibIdx will be returned as 0. A failure is indicated if both parentIdx and sibIdx are returned as 0.

void DirTree::findParentAndSib(uint64 inIdx, const std::string& inFullName, uint64& parentIdx, uint64& sibIdx)
{
    sibIdx = 0;
    parentIdx = 0;
    if (inIdx == 0 || inIdx >= entryCount() || inFullName == "/" || inFullName == "")
        return;
    std::string localName = inFullName;
    if (localName[0] != '/')
        localName = '/' + localName;
    std::string parentName = localName;
    if (parentName[parentName.size()-1] == '/')
        parentName = parentName.substr(0, parentName.size()-1);
    std::string::size_type lastSlash;
    lastSlash = parentName.find_last_of('/');
    if (lastSlash == std::string::npos)
        return;
    if (lastSlash == 0)
        lastSlash = 1; //leave root
    parentName = parentName.substr(0, lastSlash);
    DirEntry *parent2 = entry(parentName);
    parentIdx = indexOf(parent2);
    if (parent2->child == inIdx)
        return; //successful return, no sibling points to inIdx
    sibIdx = findSib(inIdx, parent2->child);
}

// Utility function to get the index of the sibling dirEntry which points to inIdx. It is the responsibility of the original caller
// to start with the root sibling - i.e., sibIdx should be pointed to by the parent node's child.

uint64 DirTree::findSib(uint64 inIdx, uint64 sibIdx)
{
    DirEntry *sib = entry(sibIdx);
    if (!sib || !sib->valid)
        return 0;
    if (sib->next == inIdx || sib->prev == inIdx)
        return sibIdx;
    DirEntry *targetSib = entry(inIdx);
    int cval = sib->compare(*targetSib);
    if (cval > 0)
        return findSib(inIdx, sib->prev);
    else
        return findSib(inIdx, sib->next);
}

void DirTree::deleteEntry(DirEntry *dirToDel, const std::string& inFullName, int64 bigBlockSize)
{
    uint64 parentIdx;
    uint64 sibIdx;
    uint64 inIdx = indexOf(dirToDel);
    uint64 nEntries = entryCount();
    findParentAndSib(inIdx, inFullName, parentIdx, sibIdx);
    uint64 replIdx;
    if (!dirToDel->next || dirToDel->next > nEntries)
        replIdx = dirToDel->prev;
    else
    {
        DirEntry *sibNext = entry(dirToDel->next);
        if (!sibNext->prev || sibNext->prev > nEntries)
        {
            replIdx = dirToDel->next;
            sibNext->prev = dirToDel->prev;
            markAsDirty(replIdx, bigBlockSize);
        }
        else
        {
            DirEntry *smlSib = sibNext;
            int64 smlIdx = dirToDel->next;
            DirEntry *smlrSib;
            int64 smlrIdx = -1;
            for ( ; ; )
            {
                smlrIdx = smlSib->prev;
                smlrSib = entry(smlrIdx);
                if (!smlrSib->prev || smlrSib->prev > nEntries)
                    break;
                smlSib = smlrSib;
                smlIdx = smlrIdx;
            }
            replIdx = smlSib->prev;
            smlSib->prev = smlrSib->next;
            smlrSib->prev = dirToDel->prev;
            smlrSib->next = dirToDel->next;
            markAsDirty(smlIdx, bigBlockSize);
            markAsDirty(smlrIdx, bigBlockSize);
        }
    }
    if (sibIdx)
    {
        DirEntry *sib = entry(sibIdx);
        if (sib->next == inIdx)
            sib->next = replIdx;
        else
            sib->prev = replIdx;
        markAsDirty(sibIdx, bigBlockSize);
    }
    else
    {
        DirEntry *parNode = entry(parentIdx);
        parNode->child = replIdx;
        markAsDirty(parentIdx, bigBlockSize);
    }
    dirToDel->valid = false; //indicating that this entry is not in use
    markAsDirty(inIdx, bigBlockSize);
}


void DirTree::debug()
{
  for( unsigned i = 0; i < entryCount(); i++ )
  {
    DirEntry* e = entry( i );
    if( !e ) continue;
    std::cout << i << ": ";
    if( !e->valid ) std::cout << "INVALID ";
    std::cout << e->name << " ";
    if( e->dir ) std::cout << "(Dir) ";
    else std::cout << "(File) ";
    std::cout << e->size << " ";
    std::cout << "s:" << e->start << " ";
    std::cout << "(";
    if( e->child == End ) std::cout << "-"; else std::cout << e->child;
    std::cout << " ";
    if( e->prev == End ) std::cout << "-"; else std::cout << e->prev;
    std::cout << ":";
    if( e->next == End ) std::cout << "-"; else std::cout << e->next;
    std::cout << ")";    
    std::cout << std::endl;
  }
}

// =========== StorageIO ==========

StorageIO::StorageIO( Storage* st, const char* fname )
: storage(st),        
  filename(fname),
  file(), 
  result(Storage::Ok),        
  opened(false),        
  filesize(0),        
  writeable(false),        
  header(new Header()),        
  dirtree(new DirTree(1 << header->b_shift)),        
  bbat(new AllocTable()),        
  sbat(new AllocTable()),
  sb_blocks(),
  mbat_blocks(),
  mbat_data(),
  mbatDirty(),
  streams()
{
  bbat->blockSize = (uint64) 1 << header->b_shift;
  sbat->blockSize = (uint64) 1 << header->s_shift;
}

StorageIO::~StorageIO()
{
  if( opened ) close();
  delete sbat;
  delete bbat;
  delete dirtree;
  delete header;
}

bool StorageIO::open(bool bWriteAccess, bool bCreate)
{
  // already opened ? close first
  if (opened)
      close();
  if (bCreate)
  {
      create();
      init();
      writeable = true;
  }
  else
  {
      writeable = bWriteAccess;
      load(bWriteAccess);
  }
  
  return result == Storage::Ok;
}

void StorageIO::load(bool bWriteAccess)
{
  unsigned char* buffer = 0;
  uint64 buflen = 0;
  std::vector<uint64> blocks;
  
  // open the file, check for error
  result = Storage::OpenFailed;

#if defined(POLE_USE_UTF16_FILENAMES)
  if (bWriteAccess)
      file.open(UTF8toUTF16(filename).c_str(), std::ios::binary | std::ios::in | std::ios::out);
  else
      file.open(UTF8toUTF16(filename).c_str(), std::ios::binary | std::ios::in);
#else
  if (bWriteAccess)
      file.open(filename.c_str(), std::ios::binary | std::ios::in | std::ios::out);
  else
      file.open(filename.c_str(), std::ios::binary | std::ios::in);
#endif //defined(POLE_USE_UTF16_FILENAMES) && defined(POLE_WIN)

  if( !file.good() ) return;
  
  // find size of input file
  file.seekg(0, std::ios::end );
  filesize = static_cast<uint64>(file.tellg());

  // load header
  buffer = new unsigned char[512];
  file.seekg( 0 ); 
  file.read( (char*)buffer, 512 );
  fileCheck(file);
  header->load( buffer );
  delete[] buffer;

  // check OLE magic id
  result = Storage::NotOLE;
  for( unsigned i=0; i<8; i++ )
    if( header->id[i] != pole_magic[i] )
      return;
  
  // sanity checks
  result = Storage::BadOLE;
  if( !header->valid() ) return;
  if( header->threshold != 4096 ) return;

  // important block size
  bbat->blockSize = (uint64) 1 << header->b_shift;
  sbat->blockSize = (uint64) 1 << header->s_shift;
  
  blocks = getbbatBlocks(true);
  
  // load big bat
  buflen = static_cast<uint64>(blocks.size())*bbat->blockSize;
  if( buflen > 0 )
  {
    buffer = new unsigned char[ buflen ];  
    loadBigBlocks( blocks, buffer, buflen );
    bbat->load( buffer, buflen );
    delete[] buffer;
  }  

  // load small bat
  blocks.clear();
  blocks = bbat->follow( header->sbat_start );
  buflen = static_cast<uint64>(blocks.size())*bbat->blockSize;
  if( buflen > 0 )
  {
    buffer = new unsigned char[ buflen ];  
    loadBigBlocks( blocks, buffer, buflen );
    sbat->load( buffer, buflen );
    delete[] buffer;
  }  
  
  // load directory tree
  blocks.clear();
  blocks = bbat->follow( header->dirent_start );
  buflen = static_cast<uint64>(blocks.size())*bbat->blockSize;
  buffer = new unsigned char[ buflen ];  
  loadBigBlocks( blocks, buffer, buflen );
  dirtree->load( buffer, buflen );
  unsigned sb_start = readU32( buffer + 0x74 );
  delete[] buffer;
  
  // fetch block chain as data for small-files
  sb_blocks = bbat->follow( sb_start ); // small files
  
  // for troubleshooting, just enable this block
#if 0
  header->debug();
  sbat->debug();
  bbat->debug();
  dirtree->debug();
#endif
  
  // so far so good
  result = Storage::Ok;
  opened = true;
}

void StorageIO::create() {
  // std::cout << "Creating " << filename << std::endl; 
  
#if defined(POLE_USE_UTF16_FILENAMES)
  file.open(UTF8toUTF16(filename).c_str(), std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
#else
  file.open( filename.c_str(), std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
#endif
  if( !file.good() )
  {
    std::cerr << "Can't create " << filename << std::endl;
    result = Storage::OpenFailed;
    return;
  }
  
  // so far so good
  opened = true;
  result = Storage::Ok;
}

void StorageIO::init()
{
    // Initialize parts of the header, directory entries, and big and small allocation tables
    header->bb_blocks[0] = 0;
    header->dirent_start = 1;
    header->sbat_start = 2;
    header->num_bat = 1;
    header->num_sbat = 1;
    header->dirty = true;
    bbat->set(0, AllocTable::Eof);
    bbat->markAsDirty(0, bbat->blockSize);
    bbat->set(1, AllocTable::Eof);
    bbat->markAsDirty(1, bbat->blockSize);
    bbat->set(2, AllocTable::Eof);
    bbat->markAsDirty(2, bbat->blockSize);
    bbat->set(3, AllocTable::Eof);
    bbat->markAsDirty(3, bbat->blockSize);
    sb_blocks = bbat->follow( 3 );
    mbatDirty = false;  
}

void StorageIO::flush()
{
    if (header->dirty)
    {
        unsigned char *buffer = new unsigned char[512];
        header->save( buffer );
        file.seekp( 0 ); 
        file.write( (char*)buffer, 512 );
        fileCheck(file);
        delete[] buffer;
    }
    if (bbat->isDirty())
        flushbbat();
    if (sbat->isDirty())
        flushsbat();
    if (dirtree->isDirty())
    {
        std::vector<uint64> blocks;
        blocks = bbat->follow(header->dirent_start);
        uint64 sb_start = 0xffffffff;
        if (sb_blocks.size() > 0)
            sb_start = sb_blocks[0];
        dirtree->flush(blocks, this, bbat->blockSize, sb_start, static_cast<uint64>(sb_blocks.size())*bbat->blockSize);
    }
    if (mbatDirty && mbat_blocks.size() > 0)
    {
        uint64 nBytes = bbat->blockSize * static_cast<uint64>(mbat_blocks.size());
        unsigned char *buffer = new unsigned char[nBytes];
        uint64 sIdx = 0;
        uint64 dcount = 0;
        uint64 blockCapacity = bbat->blockSize / sizeof(uint64) - 1;
        uint64 blockIdx = 0;
        for (unsigned mdIdx = 0; mdIdx < mbat_data.size(); mdIdx++)
        {
            writeU32(buffer + sIdx, (uint32) mbat_data[mdIdx]);
            sIdx += 4;
            dcount++;
            if (dcount == blockCapacity)
            {
                blockIdx++;
                if (blockIdx == mbat_blocks.size())
                    writeU32(buffer + sIdx, AllocTable::Eof);
                else
                    writeU32(buffer + sIdx, (uint32) mbat_blocks[blockIdx]);
                sIdx += 4;
                dcount = 0;
            }
        }
        saveBigBlocks(mbat_blocks, 0, buffer, nBytes);
        delete[] buffer;
        mbatDirty = false;
    }
    file.flush();
    fileCheck(file);

  /* Note on Microsoft implementation:
     - directory entries are stored in the last block(s)
     - BATs are as second to the last
     - Meta BATs are third to the last  
  */
}

void StorageIO::close()
{
  if( !opened ) return;
  
  file.close(); 
  opened = false;
  
  std::list<Stream*>::iterator it;
  for( it = streams.begin(); it != streams.end(); ++it )
    delete *it;
}


StreamIO* StorageIO::streamIO( const std::string& name, bool bCreate, int64 streamSize )
{
  // sanity check
  if( !name.length() ) return (StreamIO*)0;

  // search in the entries
  DirEntry* entry = dirtree->entry( name, bCreate, bbat->blockSize, this, streamSize );
  //if( entry) std::cout << "FOUND\n";
  if( !entry ) return (StreamIO*)0;
  //if( !entry->dir ) std::cout << "  NOT DIR\n";
  if( entry->dir ) return (StreamIO*)0;

  StreamIO* result2 = new StreamIO( this, entry );
  result2->fullName = name;
  
  return result2;
}

bool StorageIO::deleteByName(const std::string& fullName)
{
    if (!fullName.length())
        return false;
    if (!writeable)
        return false;
    DirEntry* entry = dirtree->entry(fullName);
    if (!entry)
        return false;
    bool retVal;
    if (entry->dir)
        retVal = deleteNode(entry, fullName);
    else
        retVal = deleteLeaf(entry, fullName);
    if (retVal)
        flush();
    return retVal;
}

bool StorageIO::deleteNode(DirEntry *entry, const std::string& fullName)
{
    std::string lclName = fullName;
    if (lclName[lclName.size()-1] != '/')
        lclName += '/';
    bool retVal = true;
    while (entry->child && entry->child < dirtree->entryCount())
    {
        DirEntry* childEnt = dirtree->entry(entry->child);
        std::string childFullName = lclName + childEnt->name;
        if (childEnt->dir)
            retVal = deleteNode(childEnt, childFullName);
        else
            retVal = deleteLeaf(childEnt, childFullName);
        if (!retVal)
            return false;
    }
    dirtree->deleteEntry(entry, fullName, bbat->blockSize);
    return retVal;
}

bool StorageIO::deleteLeaf(DirEntry *entry, const std::string& fullName)
{
    std::vector<uint64> blocks;
    if (entry->size >= header->threshold)
    {
        blocks = bbat->follow(entry->start);
        for (unsigned idx = 0; idx < blocks.size(); idx++)
        {
            bbat->set(blocks[idx], AllocTable::Avail);
            bbat->markAsDirty(idx, bbat->blockSize);
        }
    }
    else
    {
        blocks = sbat->follow(entry->start);
        for (unsigned idx = 0; idx < blocks.size(); idx++)
        {
            sbat->set(blocks[idx], AllocTable::Avail);
            sbat->markAsDirty(idx, bbat->blockSize);
        }
    }
    dirtree->deleteEntry(entry, fullName, bbat->blockSize);
    return true;
}

uint64 StorageIO::loadBigBlocks( std::vector<uint64> blocks,
  unsigned char* data, uint64 maxlen )
{
  // sentinel
  if( !data ) return 0;
  fileCheck(file);
  if( !file.good() ) return 0;
  if( blocks.size() < 1 ) return 0;
  if( maxlen == 0 ) return 0;

  // read block one by one, seems fast enough
  uint64 bytes = 0;
  for( unsigned int i=0; (i < blocks.size() ) & ( bytes<maxlen ); i++ )
  {
    uint64 block = blocks[i];
    uint64 pos =  bbat->blockSize * ( block+1 );
    uint64 p = (bbat->blockSize < maxlen-bytes) ? bbat->blockSize : maxlen-bytes;
    if( pos + p > filesize )
        p = filesize - pos;
    file.seekg( pos );
    file.read( (char*)data + bytes, p );
    fileCheck(file);
    // should use gcount to see how many bytes were really returned - eof check...
    bytes += p;
  }

  return bytes;
}

uint64 StorageIO::loadBigBlock( uint64 block,
  unsigned char* data, uint64 maxlen )
{
  // sentinel
  if( !data ) return 0;
  fileCheck(file);
  if( !file.good() ) return 0;
  
  // wraps call for loadBigBlocks
  std::vector<uint64> blocks;
  blocks.resize( 1 );
  blocks[ 0 ] = block;
  
  return loadBigBlocks( blocks, data, maxlen );
}

uint64 StorageIO::saveBigBlocks( std::vector<uint64> blocks, uint64 offset, unsigned char* data, uint64 len )
{
  // sentinel
  if( !data ) return 0;
  fileCheck(file);
  if( !file.good() ) return 0;
  if( blocks.size() < 1 ) return 0;
  if( len == 0 ) return 0;

  // write block one by one, seems fast enough
  uint64 bytes = 0;
  for( unsigned int i=0; (i < blocks.size() ) & ( bytes<len ); i++ )
  {
    uint64 block = blocks[i];
    uint64 pos =  (bbat->blockSize * ( block+1 ) ) + offset;
    uint64 maxWrite = bbat->blockSize - offset;
    uint64 tobeWritten = len - bytes;
    if (tobeWritten > maxWrite)
        tobeWritten = maxWrite;
    file.seekp( pos );
    file.write( (char*)data + bytes, tobeWritten );
    fileCheck(file);

    bytes += tobeWritten;
    offset = 0;
    if (filesize < pos + tobeWritten)
        filesize = pos + tobeWritten;
  }

  return bytes;

}

uint64 StorageIO::saveBigBlock( uint64 block, uint64 offset, unsigned char* data, uint64 len )
{
    if ( !data ) return 0;
    fileCheck(file);
    if ( !file.good() ) return 0;
    //wrap call for saveBigBlocks
    std::vector<uint64> blocks;
    blocks.resize( 1 );
    blocks[ 0 ] = block;
    return saveBigBlocks(blocks, offset, data, len );
}

// return number of bytes which has been read
uint64 StorageIO::loadSmallBlocks( std::vector<uint64> blocks,
  unsigned char* data, uint64 maxlen )
{
  // sentinel
  if( !data ) return 0;
  fileCheck(file);
  if( !file.good() ) return 0;
  if( blocks.size() < 1 ) return 0;
  if( maxlen == 0 ) return 0;

  // our own local buffer
  unsigned char* buf = new unsigned char[ bbat->blockSize ];

  // read small block one by one
  uint64 bytes = 0;
  for( unsigned int i=0; ( i<blocks.size() ) & ( bytes<maxlen ); i++ )
  {
    uint64 block = blocks[i];

    // find where the small-block exactly is
    uint64 pos = block * sbat->blockSize;
    uint64 bbindex = pos / bbat->blockSize;
    if( bbindex >= sb_blocks.size() ) break;

    loadBigBlock( sb_blocks[ bbindex ], buf, bbat->blockSize );

    // copy the data
    uint64 offset = pos % bbat->blockSize;
    uint64 p = (maxlen-bytes < bbat->blockSize-offset ) ? maxlen-bytes :  bbat->blockSize-offset;
    p = (sbat->blockSize<p ) ? sbat->blockSize : p;
    memcpy( data + bytes, buf + offset, p );
    bytes += p;
  }
  
  delete[] buf;

  return bytes;
}

uint64 StorageIO::loadSmallBlock( uint64 block,
  unsigned char* data, uint64 maxlen )
{
  // sentinel
  if( !data ) return 0;
  fileCheck(file);
  if( !file.good() ) return 0;

  // wraps call for loadSmallBlocks
  std::vector<uint64> blocks;
  blocks.resize( 1 );
  blocks.assign( 1, block );

  return loadSmallBlocks( blocks, data, maxlen );
}


uint64 StorageIO::saveSmallBlocks( std::vector<uint64> blocks, uint64 offset, 
                                        unsigned char* data, uint64 len, int64 startAtBlock )
{
  // sentinel
  if( !data ) return 0;
  fileCheck(file);
  if( !file.good() ) return 0;
  if( blocks.size() < 1 ) return 0;
  if( len == 0 ) return 0;

  // write block one by one, seems fast enough
  uint64 bytes = 0;
  for( uint64 i = startAtBlock; (i < blocks.size() ) & ( bytes<len ); i++ )
  {
    uint64 block = blocks[i];
     // find where the small-block exactly is
    uint64 pos = block * sbat->blockSize;
    uint64 bbindex = pos / bbat->blockSize;
    if( bbindex >= sb_blocks.size() ) break;
    uint64 offset2 = pos % bbat->blockSize;
    uint64 maxWrite = sbat->blockSize - offset;
    uint64 tobeWritten = len - bytes;
    if (tobeWritten > maxWrite)
        tobeWritten = maxWrite;
    saveBigBlock( sb_blocks[ bbindex ], offset2 + offset, data + bytes, tobeWritten);
    bytes += tobeWritten;
    offset = 0;
    if (filesize < pos + tobeWritten)
        filesize = pos + tobeWritten;
  }
  return bytes;
}

uint64 StorageIO::saveSmallBlock( uint64 block, uint64 offset, unsigned char* data, uint64 len )
{
    if ( !data ) return 0;
    fileCheck(file);
    if ( !file.good() ) return 0;
    //wrap call for saveSmallBlocks
    std::vector<uint64> blocks;
    blocks.resize( 1 );
    blocks[ 0 ] = block;
    return saveSmallBlocks(blocks, offset, data, len );
}

void StorageIO::flushbbat()
{
    std::vector<uint64> blocks;
    blocks = getbbatBlocks(false);
    bbat->flush(blocks, this, bbat->blockSize);
}

void StorageIO::flushsbat()
{
    std::vector<uint64> blocks;
    blocks = bbat->follow(header->sbat_start);
    sbat->flush(blocks, this, bbat->blockSize);
}

std::vector<uint64> StorageIO::getbbatBlocks(bool bLoading)
{
    std::vector<uint64> blocks;
    // find blocks allocated to store big bat
    // the first 109 blocks are in header, the rest in meta bat
    blocks.clear();
    blocks.resize( header->num_bat );

    for( unsigned i = 0; i < 109; i++ )
    {
        if( i >= header->num_bat ) 
            break;
        else 
            blocks[i] = header->bb_blocks[i];
    }
    if (bLoading)
    {
        mbat_blocks.clear();
        mbat_data.clear();
        if( (header->num_bat > 109) && (header->num_mbat > 0) ) 
        {
            unsigned char* buffer2 = new unsigned char[ bbat->blockSize ];
            uint64 k = 109;
            uint64 sector;
            uint64 mdidx = 0;
            for( uint64 r = 0; r < header->num_mbat; r++ )
            {
                if(r == 0) // 1st meta bat location is in file header.
                    sector = header->mbat_start;
                else      // next meta bat location is the last current block value.
                {
                    sector = blocks[--k];
                    mdidx--;
                }
                mbat_blocks.push_back(sector);
                mbat_data.resize(mbat_blocks.size()*(bbat->blockSize/4));
                loadBigBlock( sector, buffer2, bbat->blockSize );
                for( uint64 s=0; s < bbat->blockSize; s+=4 )
                {
                    if( k >= header->num_bat )
                        break;
                    else
                    {
                        blocks[k] = readU32( buffer2 + s );
                        mbat_data[mdidx++] = blocks[k];
                        k++;
                    }
                }  
            }
            if (mbat_data.size() != mdidx) mbat_data.resize(mdidx);
            delete[] buffer2;
        }
    }
    else
    {
        unsigned i = 109;
        for (unsigned int idx = 0; idx < mbat_data.size(); idx++)
        {
            blocks[i++] = mbat_data[idx];
            if (i == header->num_bat)
                break;
        }
    }
    return blocks;
}

uint64 StorageIO::ExtendFile( std::vector<uint64> *chain )
{
    uint64 newblockIdx = bbat->unused();
    bbat->set(newblockIdx, AllocTable::Eof);
    uint64 bbidx = newblockIdx / (bbat->blockSize / sizeof(uint64));
    while (bbidx >= header->num_bat)
        addbbatBlock();
    bbat->markAsDirty(newblockIdx, bbat->blockSize);
    if (chain->size() > 0)
    {
        bbat->set((*chain)[chain->size()-1], newblockIdx);
        bbat->markAsDirty((*chain)[chain->size()-1], bbat->blockSize);
    }
    chain->push_back(newblockIdx);
    return newblockIdx;
}

void StorageIO::addbbatBlock()
{
    uint64 newblockIdx = bbat->unused();
    bbat->set(newblockIdx, AllocTable::MetaBat);

    if (header->num_bat < 109)
        header->bb_blocks[header->num_bat] = newblockIdx;
    else
    {
        mbatDirty = true;
        mbat_data.push_back(newblockIdx);
        uint64 metaIdx = header->num_bat - 109;
        uint64 idxPerBlock = bbat->blockSize / sizeof(uint64) - 1; //reserve room for index to next block
        uint64 idxBlock = metaIdx / idxPerBlock;
        if (idxBlock == mbat_blocks.size())
        {
            uint64 newmetaIdx = bbat->unused();
            bbat->set(newmetaIdx, AllocTable::MetaBat);
            mbat_blocks.push_back(newmetaIdx);
            if (header->num_mbat == 0)
                header->mbat_start = newmetaIdx;
            header->num_mbat++;
        }
    }
    header->num_bat++;
    header->dirty = true;
}


// =========== StreamIO ==========

StreamIO::StreamIO( StorageIO* s, DirEntry* e)
:   io(s),
    entryIdx(io->dirtree->indexOf(e)),
    fullName(),
    eof(false),
    fail(false),
    blocks(),
    m_pos(0),
    cache_data(new unsigned char[CACHEBUFSIZE]),        
    cache_size(0),         // indicating an empty cache
    cache_pos(0)
{
  if( e->size >= io->header->threshold ) 
    blocks = io->bbat->follow( e->start );
  else
    blocks = io->sbat->follow( e->start );
}

// FIXME tell parent we're gone
StreamIO::~StreamIO()
{
  delete[] cache_data;  
}

void StreamIO::setSize(uint64 newSize)
{
    bool bThresholdCrossed = false;
    bool bOver = false;

    if(!io->writeable )
        return;
    DirEntry *entry = io->dirtree->entry(entryIdx);
    if (newSize >= io->header->threshold && entry->size < io->header->threshold)
    {
        bThresholdCrossed = true;
        bOver = true;
    }
    else if (newSize < io->header->threshold && entry->size >= io->header->threshold)
    {
        bThresholdCrossed = true;
        bOver = false;
    }
    if (bThresholdCrossed)
    {
        // first, read what is already in the stream, limited by the requested new size. Note
        // that the read can work precisely because we have not yet reset the size.
        uint64 len = newSize;
        if (len > entry->size)
            len = entry->size;
        unsigned char *buffer = 0;
        uint64 savePos = tell();
        if (len)
        {
            buffer = new unsigned char[len];
            seek(0);
            read(buffer, len);
        }
        // Now get rid of the existing blocks
        if (bOver)
        {
            for (unsigned int idx = 0; idx < blocks.size(); idx++)
            {
                io->sbat->set(blocks[idx], AllocTable::Avail);
                io->sbat->markAsDirty(idx, io->bbat->blockSize);
            }
        }
        else
        {
            for (unsigned int idx = 0; idx < blocks.size(); idx++)
            {
                io->bbat->set(blocks[idx], AllocTable::Avail);
                io->bbat->markAsDirty(idx, io->bbat->blockSize);
            }
        }
        blocks.clear();
        entry->start = DirTree::End;
        // Now change the size, and write the old data back into the stream, if any
        entry->size = newSize;
        io->dirtree->markAsDirty(io->dirtree->indexOf(entry), io->bbat->blockSize);
        if (len)
        {
            write(0, buffer, len);
            delete buffer;
        }
        if (savePos <= entry->size)
            seek(savePos);
    }
    else if (entry->size != newSize) //simple case - no threshold was crossed, so just change the size
    {
        entry->size = newSize;
        io->dirtree->markAsDirty(io->dirtree->indexOf(entry), io->bbat->blockSize);
    }

}

void StreamIO::seek( uint64 pos )
{
  m_pos = pos;
}

uint64 StreamIO::tell()
{
  return m_pos;
}

int64 StreamIO::getch()
{
  // past end-of-file ?
  DirEntry *entry = io->dirtree->entry(entryIdx);
  if( m_pos >= entry->size ) return -1;

  // need to update cache ?
  if( !cache_size || ( m_pos < cache_pos ) ||
    ( m_pos >= cache_pos + cache_size ) )
      updateCache();

  // something bad if we don't get good cache
  if( !cache_size ) return -1;

  int64 data = cache_data[m_pos - cache_pos];
  m_pos++;

  return data;
}

uint64 StreamIO::read( uint64 pos, unsigned char* data, uint64 maxlen )
{
  // sanity checks
  if( !data ) return 0;
  if( maxlen == 0 ) return 0;

  uint64 totalbytes = 0;
  
  DirEntry *entry = io->dirtree->entry(entryIdx);
  if (pos + maxlen > entry->size)
      maxlen = entry->size - pos;
  if ( entry->size < io->header->threshold )
  {
    // small file
    uint64 index = pos / io->sbat->blockSize;

    if( index >= blocks.size() ) return 0;

    unsigned char* buf = new unsigned char[ io->sbat->blockSize ];
    uint64 offset = pos % io->sbat->blockSize;
    while( totalbytes < maxlen )
    {
      if( index >= blocks.size() ) break;
      io->loadSmallBlock( blocks[index], buf, io->bbat->blockSize );
      uint64 count = io->sbat->blockSize - offset;
      if( count > maxlen-totalbytes ) count = maxlen-totalbytes;
      memcpy( data+totalbytes, buf + offset, count );
      totalbytes += count;
      offset = 0;
      index++;
    }
    delete[] buf;

  }
  else
  {
    // big file
    uint64 index = pos / io->bbat->blockSize;
    
    if( index >= blocks.size() ) return 0;
    
    unsigned char* buf = new unsigned char[ io->bbat->blockSize ];
    uint64 offset = pos % io->bbat->blockSize;
    while( totalbytes < maxlen )
    {
      if( index >= blocks.size() ) break;
      io->loadBigBlock( blocks[index], buf, io->bbat->blockSize );
      uint64 count = io->bbat->blockSize - offset;
      if( count > maxlen-totalbytes ) count = maxlen-totalbytes;
      memcpy( data+totalbytes, buf + offset, count );
      totalbytes += count;
      index++;
      offset = 0;
    }
    delete[] buf;

  }

  return totalbytes;
}

uint64 StreamIO::read( unsigned char* data, uint64 maxlen )
{
  uint64 bytes = read( tell(), data, maxlen );
  m_pos += bytes;
  return bytes;
}

uint64 StreamIO::write( unsigned char* data, uint64 len )
{
  return write( tell(), data, len );
}

uint64 StreamIO::write( uint64 pos, unsigned char* data, uint64 len )
{
  // sanity checks
  if( !data ) return 0;
  if( len == 0 ) return 0;
  if( !io->writeable ) return 0;

  DirEntry *entry = io->dirtree->entry(entryIdx);
  if (pos + len > entry->size)
      setSize(pos + len); //reset size, possibly changing from small to large blocks
  uint64 totalbytes = 0;
  if ( entry->size < io->header->threshold )
  {
    // small file
    uint64 index = (pos + len - 1) / io->sbat->blockSize;
    while (index >= blocks.size())
    {
        uint64 nblock = io->sbat->unused();
        if (blocks.size() > 0)
        {
            io->sbat->set(blocks[blocks.size()-1], nblock);
            io->sbat->markAsDirty(blocks[blocks.size()-1], io->bbat->blockSize);
        }
        io->sbat->set(nblock, AllocTable::Eof);
        io->sbat->markAsDirty(nblock, io->bbat->blockSize);
        blocks.resize(blocks.size()+1);
        blocks[blocks.size()-1] = nblock;
        uint64 bbidx = nblock / (io->bbat->blockSize / sizeof(unsigned int));
        while (bbidx >= io->header->num_sbat)
        {
            std::vector<uint64> sbat_blocks = io->bbat->follow(io->header->sbat_start);
            io->ExtendFile(&sbat_blocks);
            io->header->num_sbat++;
            io->header->dirty = true; //Header will have to be rewritten
        }
        uint64 sidx = nblock * io->sbat->blockSize / io->bbat->blockSize;
        while (sidx >= io->sb_blocks.size()) 
        {
            io->ExtendFile(&io->sb_blocks);
            io->dirtree->markAsDirty(0, io->bbat->blockSize); //make sure to rewrite first directory block
        }
    }
    uint64 offset = pos % io->sbat->blockSize;
    index = pos / io->sbat->blockSize;
    //if (index == 0)
        totalbytes = io->saveSmallBlocks(blocks, offset, data, len, index);
  }
  else
  {
    uint64 index = (pos + len - 1) / io->bbat->blockSize;
    while (index >= blocks.size())
        io->ExtendFile(&blocks);
    uint64 offset = pos % io->bbat->blockSize;
    uint64 remainder = len;
    index = pos / io->bbat->blockSize;
    while( remainder > 0 )
    {
      if( index >= blocks.size() ) break;
      uint64 count = io->bbat->blockSize - offset;
      if ( remainder < count )
          count = remainder;
      io->saveBigBlock( blocks[index], offset, data + totalbytes, count );
      totalbytes += count;
      remainder -= count;
      index++;
      offset = 0;
    }
  }
  if (blocks.size() > 0 && entry->start != blocks[0])
  {
      entry->start = blocks[0];
      io->dirtree->markAsDirty(io->dirtree->indexOf(entry), io->bbat->blockSize);
  }
  m_pos += len;
  return totalbytes;
}

void StreamIO::flush()
{
    io->flush();
}

void StreamIO::updateCache()
{
  // sanity check
  if( !cache_data ) return;

  DirEntry *entry = io->dirtree->entry(entryIdx);
  cache_pos = m_pos - (m_pos % CACHEBUFSIZE);
  uint64 bytes = CACHEBUFSIZE;
  if( cache_pos + bytes > entry->size ) bytes = entry->size - cache_pos;
  cache_size = read( cache_pos, cache_data, bytes );
}


// =========== Storage ==========

Storage::Storage( const char* filename )
{
  io = new StorageIO( this, filename );
}

Storage::~Storage()
{
  delete io;
}

int Storage::result()
{
  return (int) io->result;
}

bool Storage::open(bool bWriteAccess, bool bCreate)
{
  return io->open(bWriteAccess, bCreate);
}

void Storage::close()
{
  io->close();
}

std::list<std::string> Storage::entries( const std::string& path )
{
  std::list<std::string> localResult;
  DirTree* dt = io->dirtree;
  DirEntry* e = dt->entry( path, false );
  if( e  && e->dir )
  {
    uint64 parent = dt->indexOf( e );
    std::vector<uint64> children = dt->children( parent );
    for( uint64 i = 0; i < children.size(); i++ )
      localResult.push_back( dt->entry( children[i] )->name );
  }
  
  return localResult;
}

bool Storage::isDirectory( const std::string& name )
{
  DirEntry* e = io->dirtree->entry( name, false );
  return e ? e->dir : false;
}

bool Storage::exists( const std::string& name )
{
    DirEntry* e = io->dirtree->entry( name, false );
    return (e != 0);
}

bool Storage::isWriteable()
{
    return io->writeable;
}

bool Storage::deleteByName( const std::string& name )
{
  return io->deleteByName(name);
}

void Storage::GetStats(uint64 *pEntries, uint64 *pUnusedEntries,
      uint64 *pBigBlocks, uint64 *pUnusedBigBlocks,
      uint64 *pSmallBlocks, uint64 *pUnusedSmallBlocks)
{
    *pEntries = io->dirtree->entryCount();
    *pUnusedEntries = io->dirtree->unusedEntryCount();
    *pBigBlocks = io->bbat->count();
    *pUnusedBigBlocks = io->bbat->unusedCount();
    *pSmallBlocks = io->sbat->count();
    *pUnusedSmallBlocks = io->sbat->unusedCount();
}

// recursively collect stream names
void CollectStreams( std::list<std::string>& result, DirTree* tree, DirEntry* parent, const std::string& path )
{
  DirEntry* c = tree->entry( parent->child );
  std::queue<DirEntry*> queue;
  if ( c ) queue.push( c );
  while ( !queue.empty() ) {
    DirEntry* e = queue.front();
    queue.pop();
    if ( e->dir )
      CollectStreams( result, tree, e, path + e->name + "/" );
    else
      result.push_back( path + e->name );
    DirEntry* p = tree->entry( e->prev );
    if ( p ) queue.push( p );
    DirEntry* n = tree->entry( e->next );
    if ( n ) queue.push( n );
    // not testing if p or n have already been processed; potential infinite loop in case of closed Entry chain
    // it seems not to happen, though
  }
}

std::list<std::string> Storage::GetAllStreams( const std::string& storageName )
{
  std::list<std::string> vresult;
  DirEntry* e = io->dirtree->entry( storageName, false );
  if ( e && e->dir ) CollectStreams( vresult, io->dirtree, e, storageName );
  return vresult;
}

// =========== Stream ==========

Stream::Stream( Storage* storage, const std::string& name, bool bCreate, int64 streamSize )
:   io(storage->io->streamIO( name, bCreate, (int) streamSize ))
{
}

// FIXME tell parent we're gone
Stream::~Stream()
{
  delete io;
}

std::string Stream::fullName()
{
  return io ? io->fullName : std::string();
}

uint64 Stream::tell()
{
  return io ? io->tell() : 0;
}

void Stream::seek( uint64 newpos )
{
  if( io )
      io->seek(newpos);
}

uint64 Stream::size()
{
    if (!io)
        return 0;
    DirEntry *entry = io->io->dirtree->entry(io->entryIdx);
    return entry->size;
}

void Stream::setSize(int64 newSize)
{
    if (!io)
        return;
    if (newSize < 0)
        return;
    if (newSize > std::numeric_limits<int64>::max())
        return;
    io->setSize(newSize);
}

int64 Stream::getch()
{
  return io ? io->getch() : 0;
}

uint64 Stream::read( unsigned char* data, uint64 maxlen )
{
  return io ? io->read( data, maxlen ) : 0;
}

uint64 Stream::write( unsigned char* data, uint64 len )
{
    return io ? io->write( data, len ) : 0;
}

void Stream::flush()
{
    if (io)
        io->flush();
}

bool Stream::eof()
{
  return io ? io->eof : false;
}

bool Stream::fail()
{
  return io ? io->fail : true;
}
