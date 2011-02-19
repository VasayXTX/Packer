#ifndef UNPACKER
#define UNPACKER

#include "packerRLE.h"
#include "packerHuffman.h"

/****************************************************************************************************/
/********************************************* UnPacker *********************************************/
/****************************************************************************************************/

class UnPacker
{
private:
	unsigned int bufSize;

public:
	UnPacker(unsigned int aBufSize = Reader::DEF_BUF_SIZE);

	void UnPack(const std::string &aFileIn);
};

#endif