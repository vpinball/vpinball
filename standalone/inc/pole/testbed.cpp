// testbed.cpp: Take a directory and its contents, and create a Structured Storage File from it.
// Written using Windows - it should be portable to other environments with relatively modest changes.
// This was done mainly as a way to test POLE at various stages, but it has some utility on its own.
//

#include <iostream>
#include <fstream>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <list>
#include <string>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h>

#include "pole.h"

POLE::Storage* storage = 0;

bool DirectoryExists( const char* dirname ){

    if( _access( dirname, 0 ) == 0 ){

        struct stat status;
        stat( dirname, &status );

        return (status.st_mode & S_IFDIR) != 0;
    }
    return false;
}

void visit( int indent, POLE::Storage* storage, std::string path )
{
	std::list<std::string> entries;
	entries = storage->entries( path );

	std::list<std::string>::iterator it;
	for( it = entries.begin(); it != entries.end(); ++it )
	{
		std::string name = *it;
		std::string fullname = path + name;
		for( int j = 0; j < indent; j++ ) 
			std::cout << "    ";
		POLE::Stream* ss = new POLE::Stream( storage, fullname );
		std::cout << name;
		if( ss )
			if( !ss->fail() )std::cout << "  (" << ss->size() << ")";
		std::cout << std::endl;
		delete ss;

		if( storage->isDirectory( fullname ) )
			visit( indent+1, storage, fullname + "/" );
	}

}

bool extract( POLE::Storage* storage, const char* stream_name, const char* outfile )
{
  POLE::Stream* stream = new POLE::Stream( storage, stream_name );
  if( !stream )
  {
	printf("A stream could not be opened for %s.\n", stream_name);
	return false;
  }
  if( stream->fail() )
  {
	printf("A stream failed for %s.\n", stream_name);
	return false;
  }
  
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
  return true;
}

void extractDir(POLE::Storage* storage, std::string outDir, std::string path, int &nFolders, int &nFiles)
{
	const char *cOutDir = outDir.c_str();
	if (DirectoryExists(cOutDir))
	{
		std::cout << "You should not specify a directory for output that already exists." << std::endl;
		return;
	}

	_mkdir(cOutDir);
	nFolders++;
	std::list<std::string> entries;
	entries = storage->entries( path );

	std::list<std::string>::iterator it;
	for( it = entries.begin(); it != entries.end(); ++it )
	{
		std::string name = *it;
		std::string fullname = path + name;
		if (storage->isDirectory(fullname))
		{
			std::string outdirchild = outDir + name;
			extractDir(storage, outdirchild + "/", fullname + "/", nFolders, nFiles);
		}
		else
		{
			POLE::Stream* stream = new POLE::Stream(storage, fullname);
			if( !stream )
			{
				printf("A stream could not be opened for %s.\n", fullname);
				return;
			}
			if( stream->fail() )
			{
				printf("A stream failed for %s.\n", fullname);
				return;
			}
			std::ofstream file;
			std::string outfname = outDir + name;
			const char *coutfname = outfname.c_str();
			file.open(coutfname, std::ios::binary|std::ios::out);
			unsigned long bytesLeft = stream->size();
			unsigned char buffer[4096];
			while(bytesLeft > 0)
			{
				unsigned long bytesToRead = sizeof( buffer );
				if (bytesLeft < bytesToRead)
					bytesToRead = bytesLeft;
				unsigned read = stream->read( buffer, bytesToRead );
				file.write( (const char*)buffer, read  );
				bytesLeft -= read;
				if( read < bytesToRead )
					break; //shouldn't happen, probably
			}
			file.close();
			delete stream;
			nFiles++;
		}
	}
}

bool AddFile2Storage(POLE::Storage* storage, std::string inFile, std::string name)
{
	std::ifstream file;
	const char *cinfname = inFile.c_str();
	file.open(cinfname, std::ios::binary|std::ios::in);
	// find size of input file
	file.seekg( 0, std::ios::end );
	long filesize = file.tellg();
	file.seekg( 0 );
	POLE::Stream* ss = new POLE::Stream( storage, name, true, filesize );
	if( !ss )
	{
		printf("A stream could not be opened for %s.\n", name);
		file.close();
		return false;
	}
	if( ss->fail() )
	{
		printf("A stream failed for %s.\n", name);
		file.close();
		return false;
	}
	unsigned char buffer[4096];
	int bytesRead = 0;
	int bytesToRead;
	for (;;)
	{
		bytesToRead = filesize - bytesRead;
		if (bytesToRead <= 0)
			break;
		if (bytesToRead > 4096)
			bytesToRead = 4096;
		file.read((char *)buffer, bytesToRead);
		ss->write(buffer, bytesToRead);
		bytesRead += bytesToRead;
	}
	ss->flush();
	delete ss;
	file.close();
	return true;
}

void AddDir2Storage(POLE::Storage* storage, std::string inDir, std::string name, int &nFolders, int &nFiles)
{
	WIN32_FIND_DATA ffd;
	HANDLE h;


	std::string inDirLcl = inDir + "*";
	std::string outNameLcl = name;

	const char *cInDir = inDirLcl.c_str();
	h = FindFirstFile(cInDir, &ffd);
	if (h == INVALID_HANDLE_VALUE)
		return;
	nFolders++;
	do
	{
		if (ffd.cFileName[0] == '.')
		{
			if (ffd.cFileName[1] == 0 || (ffd.cFileName[1] == '.' && ffd.cFileName[2] == 0))
				continue;
		}
		outNameLcl = name + ffd.cFileName;
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			outNameLcl += "/";
			inDirLcl = inDir + ffd.cFileName;
			inDirLcl += "\\";
			AddDir2Storage(storage, inDirLcl, outNameLcl, nFolders, nFiles);
		}
		else
		{
			std::ifstream file;
			std::string infname = inDir + ffd.cFileName;
			const char *cinfname = infname.c_str();
			file.open(cinfname, std::ios::binary|std::ios::in);
			// find size of input file
			file.seekg( 0, std::ios::end );
			long filesize = file.tellg();
			file.seekg( 0 );
			POLE::Stream* ss = new POLE::Stream( storage, outNameLcl, true, filesize );
			unsigned char buffer[4096];
			int bytesRead = 0;
			int bytesToRead;
			for (;;)
			{
				bytesToRead = filesize - bytesRead;
				if (bytesToRead <= 0)
					break;
				if (bytesToRead > 4096)
					bytesToRead = 4096;
				file.read((char *)buffer, bytesToRead);
				ss->write(buffer, bytesToRead);
				bytesRead += bytesToRead;
			}
			ss->flush();
			delete ss;
			file.close();
			nFiles++;
		}
	}
	while (FindNextFile(h, &ffd) != 0);
}



void cmdOpen(std::list<std::string> &entries)
{
	if (entries.size() < 1)
		std::cout << "You must enter the name of an existing file formatted as structured storage." << std::endl;
	else
	{
		if (storage != 0)
			storage->close();
		std::string ssName = entries.front();
		entries.pop_front();
		storage = new POLE::Storage(ssName.c_str());
		bool bWrite = false;
		bool bCreate = false;
		if (entries.size() > 0)
		{
			std::string modifiers = entries.front();
			for (unsigned idx = 0; idx < modifiers.size(); idx++)
			{
				char c = modifiers[idx];
				if (c == 'c')
					bCreate = true;
				else if (c == 'w')
					bWrite = true;
			}
		}
		storage->open(bWrite, bCreate);
		int result = storage->result();
		if (result == POLE::Storage::Ok)
			std::cout << "Ok" << std::endl;
		else
		{
			delete storage;
			storage = 0;
			if (result == POLE::Storage::BadOLE)
				std::cout << "BadOLE" << std::endl;
			else if (result == POLE::Storage::NotOLE)
				std::cout << "NotOLE" << std::endl;
			else if (result == POLE::Storage::OpenFailed)
				std::cout << "OpenFailed" << std::endl;
			else
				printf("Unrecognizable result - %d\n", result);
		}
	}
}

void cmdClose(std::list<std::string> &entries)
{
	if (storage)
	{
		storage->close();
		delete storage;
		storage = 0;
	}
}

void cmdStats(std::list<std::string> &entries)
{
	if (storage)
	{
		unsigned int nDirs, nUUDirs, nBBs, nUUBBs, nSBs, nUUSBs;
		storage->GetStats(&nDirs, &nUUDirs, &nBBs, &nUUBBs, &nSBs, &nUUSBs);
		printf("%d Directory Entries, %d unused.\n", nDirs, nUUDirs);
		printf("%d Big Blocks, %d unused.\n", nBBs, nUUBBs);
		printf("%d Small Blocks, %d unused.\n", nSBs, nUUSBs);
	}
}

void cmdVisit(std::list<std::string> &entries)
{
	if (!storage)
		std::cout << "No storage is opened." << std::endl;
	else
	{
		if (entries.size() > 0)
			visit(0, storage, entries.front());
		else
			visit(0, storage, "/");
	}
}

void cmdExtract(std::list<std::string> &entries)
{
	if (!storage)
		std::cout << "No storage is opened." << std::endl;
	else if (entries.size() < 2)
		std::cout << "You must specify a path in the open storage to be extracted, and a destination file or folder." << std::endl;
	else
	{
		std::string ssPath = entries.front();
		entries.pop_front();
		std::string filePath = entries.front();
		if (storage->isDirectory(ssPath))
		{
			if (filePath[filePath.size()-1] != '/')
				filePath += '/';
			if (ssPath[0] != '/')
				ssPath = '/' + ssPath;
			if (ssPath[ssPath.size()-1] != '/')
				ssPath += '/';
			int nFiles = 0;
			int nFolders = 0;
			extractDir(storage, filePath, ssPath, nFolders, nFiles);
			printf("%d folders and %d files were extracted.\n", nFolders, nFiles);
		}
		else
		{
			if (extract(storage, ssPath.c_str(), filePath.c_str()))
				std::cout << "Ok" << std::endl;
		}
	}
}

void cmdAdd(std::list<std::string> &entries)
{
	if (!storage)
		std::cout << "No storage is opened." << std::endl;
	else if (entries.size() < 2)
		std::cout << "You must specify a path in the open storage to be created or modified, and a source file or folder." << std::endl;
	else if (!storage->isWriteable())
		std::cout << "The open storage cannot be modified." << std::endl;
	else if (entries.front() != "/" && storage->exists(entries.front()))
		std::cout << "The specified storage node already exists - to save it again, first delete it." << std::endl;
	else
	{
		std::string ssPath = entries.front();
		entries.pop_front();
		std::string filePath = entries.front();
		if (DirectoryExists(filePath.c_str()))
		{
			if (filePath[filePath.size()-1] != '/')
				filePath += '/';
			if (ssPath[0] != '/')
				ssPath = '/' + ssPath;
			if (ssPath[ssPath.size()-1] != '/')
				ssPath += '/';
			int nFiles = 0;
			int nFolders = 0;
			AddDir2Storage(storage, filePath, ssPath, nFolders, nFiles);
			printf("%d folders and %d files were added.\n", nFolders, nFiles);
		}
		else
		{
			if (AddFile2Storage(storage, filePath, ssPath))
				std::cout << "Ok" << std::endl;
		}
	}
}

void cmdDelete(std::list<std::string> &entries)
{
	if (!storage)
		std::cout << "No storage is opened." << std::endl;
	else if (entries.size() < 1)
		std::cout << "You must specify a path in the open storage to be deleted." << std::endl;
	else if (!storage->isWriteable())
		std::cout << "The open storage cannot be modified." << std::endl;
	else
	{
		if (!storage->deleteByName(entries.front()))
			std::cout << "The deletion failed." << std::endl;
		else
			std::cout << "Ok" << std::endl;
	}
}

bool cmdQuit(std::list<std::string> &entries)
{
	cmdClose(entries);
	return true;
}

void cmdHelp(std::list<std::string> &entries)
{
	std::cout << "You can enter any of the following commands. Note that they are case sensitive." << std::endl;
	std::cout << "open filePath [modifier] - modifiers can be any combination of r, w, or c (create)" << std::endl;
	std::cout << "visit [ssPath] - list contents of the structured storage, or of one leaf or node" << std::endl;
	std::cout << "extract ssPath filePath - Extract the contents of one leaf" << std::endl;
	std::cout << "add ssPath filePath - Add the contents of one leaf to the structured storage" << std::endl;
	std::cout << "delete ssPath - Delete the leaf or node, and whatever it contains" << std::endl;
	std::cout << "stats - show statistics for the open storage" << std::endl;
	std::cout << "close" << std::endl;
	std::cout << "quit" << std::endl;
}

std::list<std::string> clineParse(std::string inCmd)
{
	std::list<std::string> outWords;
	std::string oneWord;
	char c;
	int idx;
	int maxIdx = inCmd.size();
	bool bInQuote = false;
	for (idx = 0; idx < maxIdx; idx++)
	{
		c = inCmd[idx];
		if (bInQuote)
		{
			if (c == '\"')
			{
				outWords.push_back(oneWord);
				oneWord.clear();
				bInQuote = false;
			}
			else
				oneWord+= c;
		}
		else
		{
			if (c == '\"' && oneWord.size() == 0)
				bInQuote = true;
			else if (c == ' ')
			{
				if (oneWord.size() != 0)
				{
					outWords.push_back(oneWord);
					oneWord.clear();
				}
			}
			else
				oneWord+= c;
		}
	}
	if (oneWord.size() != 0)
		outWords.push_back(oneWord);
	return outWords;
}

int main(int argc, char* argv[])
{
	if( argc > 1 )
	{
		std::cout << "Usage:" << std::endl;
		std::cout << argv[0] << " This takes no arguments - type help in the console window for information. " << std::endl;
		return 0;
	}

	std::string command;
	while (true)
	{
		std::getline(std::cin, command);
		std::list<std::string> entries = clineParse(command);
		if (entries.size() == 0)
			continue;
		std::string cmdWord = entries.front();
		entries.pop_front();
		if (cmdWord.compare("open") == 0)
			cmdOpen(entries);
		else if (cmdWord.compare("visit") == 0)
			cmdVisit(entries);
		else if (cmdWord.compare("extract") == 0)
			cmdExtract(entries);
		else if (cmdWord.compare("add") == 0)
			cmdAdd(entries);
		else if (cmdWord.compare("delete") == 0)
			cmdDelete(entries);
		else if (cmdWord.compare("stats") == 0)
			cmdStats(entries);
		else if (cmdWord.compare("close") == 0)
			cmdClose(entries);
		else if (cmdWord.compare("quit") == 0)
		{
			if (cmdQuit(entries))
				break;
		}
		else
			cmdHelp(entries);
	}
#if 0
		std::list<std::string>::iterator it;
		for( it = entries.begin(); it != entries.end(); ++it )
			std::cout << *it << std::endl;
	open filename [modifier] (modifier can be r, w, c)
visit [sspath]
extract sspath fpath
add sspath fpath
delete sspath
close

	std::string indir = argv[1];
	char* outfile = argv[2];
	if (indir[indir.length()-1] != '\\')
		indir += "\\";
	POLE::Storage* storage = new POLE::Storage(outfile);
	storage->open(true, true);
	AddDir2Storage(storage, indir, "/");
	storage->close();
	storage->open();
	visit(0, storage, "/");
	storage->close();
#endif
}