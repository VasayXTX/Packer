#ifndef PACKER_RLE
#define PACKER_RLE

#include "packer.h"

/****************************************************************************************************/
/***************************************** Packer RLE ***********************************************/
/****************************************************************************************************/
class Packer;

class PackerRLE: public Packer
{
private:
	void _Pack(FILE *aFIn, FILE *aFOut, unsigned int aFSize, const Printer &aPr);
	unsigned int Coding(FILE *aFIn, FILE *aFOut, unsigned int aFSize, const Printer &aPr);

public:
	PackerRLE(unsigned int aBufSize = Reader::DEF_BUF_SIZE);
};

/****************************************************************************************************/
/****************************************** UnPacker RLE ********************************************/
/****************************************************************************************************/

class UnPackerRLE: public ProxyUnPacker
{
public:
	UnPackerRLE(unsigned int aBufSize = Reader::DEF_BUF_SIZE);

	void _UnPack(FILE *aFIn, FILE *aFOut, const Printer &aPr);
};

#endif