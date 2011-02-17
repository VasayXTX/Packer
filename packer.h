#ifndef PACKER_H
#define PACKER_H

#include <list>
#include <limits.h>
#include <vector>
#include <utility>

#include "error.h"

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

	virtual void _Pack(FILE *aFIn, FILE *aFOut) = 0;

public:
	void Pack(const LstFile &aLstFileIn, const std::string &aFileOut);
};

/****************************************************************************************************/
/***************************************** Packer RLE ***********************************************/
/****************************************************************************************************/

class PackerRLE: public Packer
{
private:
	void _Pack(FILE *aFIn, FILE *aFOut);
	unsigned int Coding(FILE *aFIn, FILE *aFOut);

public:
	PackerRLE(unsigned int aBufSize = Reader::DEF_BUF_SIZE);
};

/****************************************************************************************************/
/*********************************************** Huffman ********************************************/
/****************************************************************************************************/

namespace Huffman
{
	//--------------- Packing (префикс P) ---------------
	class PTNode					//Узел в дереве Хаффмана, использующееся при сжатии
	{
	private:
		PTNode *left, *right;
		unsigned int val;

	public:
		PTNode(unsigned int aVal, PTNode *aLeft = 0, PTNode *aRight = 0);
		PTNode *Left() const;
		PTNode *Right() const;
		unsigned int Value() const;
	};

	class PTLeaf: public PTNode		//Лист в дереве Хаффмана, использующееся при сжатии
	{
	private:
		unsigned char byte;
	public:
		PTLeaf(unsigned char aByte, unsigned int aVal);
		unsigned char Byte() const;
	};

	typedef std::list<PTNode *> LstPTNode;
	typedef std::list<unsigned char> PCode;
	typedef std::pair<PCode, unsigned int> PElem;
	typedef std::vector<PElem> PDict;
	typedef std::pair<PTNode *, unsigned int> PTPair;	//1 - указатель на корень дерева, 2 - количество листьев

	//--------------- UnPacking (префикс UP) ---------------
	class UPTNode					//Узел в дереве Хаффмана, использующееся при распаковке
	{
	private:
		UPTNode *left, *right;

	public:
		UPTNode(UPTNode *aLeft = 0, UPTNode *aRight = 0);

		void SetLeft(UPTNode *aNode);
		void SetRight(UPTNode *aNode);
		UPTNode *Left() const;
		UPTNode *Right() const;
	};

	class UPTLeaf: public UPTNode		//Лист в дереве Хаффмана, использующееся при распаковке
	{
	private:
		unsigned char byte;
		unsigned int codeSize;
	public:
		UPTLeaf(unsigned char aByte, unsigned int aCodeSize);

		unsigned char Byte() const;
		unsigned int CodeSize() const;
	};

	typedef std::list<UPTLeaf *> LstUPTLeaf;
}

/****************************************************************************************************/
/****************************************** Packer Huffman ******************************************/
/****************************************************************************************************/

class PackerHuffman: public Packer
{
private:
	std::vector<unsigned int> vecByte;
	Huffman::PDict dict;
	Huffman::PTNode *root;

	unsigned int InitVecByte(FILE *aFIn);
	unsigned int CreateTree();
	void CreateDictionary(Huffman::PTNode *aNode, Huffman::PCode aCur, unsigned char aMask, unsigned int aDepth);
	void WriteDictionary(FILE *aFOut, unsigned int aDictSize, unsigned char *aCurCh, unsigned int *aCurSize);
	inline void WriteCode(FILE *aFOut, const Huffman::PElem &aHPairCode, unsigned char *aCurCh, unsigned int *aCurSize);
	void Coding(FILE *aFIn, FILE *aFOut, unsigned char *aCurCh, unsigned int *aCurSize);

	void PrintReport() const;

	void _Pack(FILE *aFIn, FILE *aFOut);

public:
	PackerHuffman(unsigned int aBufSize = Reader::DEF_BUF_SIZE);
	~PackerHuffman();
};

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

class ProxyUnPacker: public Reader
{
public:
	ProxyUnPacker(unsigned int aBufSize = Reader::DEF_BUF_SIZE);

	virtual void UnPack(FILE *aFIn, FILE *aFOut) = 0;

	friend class UnPacker;
};

/****************************************************************************************************/
/****************************************** UnPacker RLE ********************************************/
/****************************************************************************************************/

class UnPackerRLE: public ProxyUnPacker
{
public:
	UnPackerRLE(unsigned int aBufSize = Reader::DEF_BUF_SIZE);

	void UnPack(FILE *aFIn, FILE *aFOut);
};

/****************************************************************************************************/
/***************************************** UnPacker Huffman *****************************************/
/****************************************************************************************************/

class UnPackerHuffman: public ProxyUnPacker
{
private:
	Huffman::LstUPTLeaf lstLeaf;
	Huffman::UPTNode *root;

	void ReadDictionary(FILE *aFIn);
	void CreateTree(FILE *aFIn, unsigned char *aCurCh, unsigned int *aCurSize);
	inline bool NextBit(FILE *aFIn, unsigned char *aCurCh, unsigned int *aCurSize);
	void Decoding(FILE *aFIn, FILE *aFOut, unsigned char *aCurCh, unsigned int *aCurSize, unsigned int aFLen);

public:
	UnPackerHuffman(unsigned int aBufSize = Reader::DEF_BUF_SIZE);
	~UnPackerHuffman();

	void UnPack(FILE *aFIn, FILE *aFOut);
};

#endif