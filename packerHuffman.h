#ifndef PACKER_HUFFMAN
#define PACKER_HUFFMAN

#include "packer.h"

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
	void Coding(FILE *aFIn, FILE *aFOut, unsigned char *aCurCh, unsigned int *aCurSize, unsigned int aFSize, const Printer &aPr);

	void PrintReport() const;

	void _Pack(FILE *aFIn, FILE *aFOut, unsigned int aFSize, const Printer &aPr);

public:
	PackerHuffman(unsigned int aBufSize = Reader::DEF_BUF_SIZE);
	~PackerHuffman();
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
	void Decoding(FILE *aFIn, FILE *aFOut, unsigned char *aCurCh, unsigned int *aCurSize, unsigned int aFLen, const Printer &aPr);

public:
	UnPackerHuffman(unsigned int aBufSize = Reader::DEF_BUF_SIZE);
	~UnPackerHuffman();

	void _UnPack(FILE *aFIn, FILE *aFOut, const Printer &aPr);
};

#endif