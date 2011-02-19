#ifndef PACKER_H
#define PACKER_H

#include <list>
#include <limits.h>
#include <vector>
#include <utility>

#include "error.h"
#include "printer.h"

/****************************************************************************************************/
/********************************************* Reader ***********************************************/
/****************************************************************************************************/

class Reader
{
protected:
	static const unsigned short BIT_IN_BYTE = 8;
	static const unsigned short MASK = 128;

	unsigned int bufSize;
	unsigned char *buf;
	unsigned int counter;
	unsigned int bufLen;

	Reader(unsigned int aBufSize);
	~Reader();
	
	inline bool SetNext(FILE *aFIn, unsigned char *aCh);
	inline void _SetNext(FILE *aFIn, unsigned char *aCh);
	unsigned int ReadInt(FILE *aFIn);

public:
	static const unsigned int DEF_BUF_SIZE = 1024;
};

/****************************************************************************************************/
/********************************************* Packer ***********************************************/
/****************************************************************************************************/

typedef std::list<std::string> LstFile;

class Packer: public Reader 
{
public:
	enum Alg
	{
		aRLE = 0,
		aHuffman
	};

private:
	Alg alg;

protected:
	Packer(unsigned int aBufSize, Alg aAlg);

	virtual void _Pack(FILE *aFIn, FILE *aFOut, unsigned int aFSize, const Printer &aPr) = 0;

public:
	void Pack(const LstFile &aLstFileIn, const std::string &aFileOut);
};

/****************************************************************************************************/
/***************************************** ProxyUnPacker  *******************************************/
/****************************************************************************************************/

class ProxyUnPacker: public Reader
{
public:
	ProxyUnPacker(unsigned int aBufSize = Reader::DEF_BUF_SIZE);

	virtual void _UnPack(FILE *aFIn, FILE *aFOut, const Printer &aPr) = 0;

	friend class UnPacker;
};


#endif