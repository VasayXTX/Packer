#include "packerHuffman.h"

using namespace Huffman;

/****************************************************************************************************/
/******************************************** Huffman ***********************************************/
/****************************************************************************************************/

PTNode::PTNode(unsigned int aVal, PTNode *aLeft, PTNode *aRight):
	left(aLeft),
	right(aRight),
	val(aVal)
{
	//empty
}

PTNode *PTNode::Left() const
{
	return left;
}

PTNode *PTNode::Right() const
{
	return right;
}

unsigned int PTNode::Value() const
{
	return val;
}

PTLeaf::PTLeaf(unsigned char aByte, unsigned int aVal):
	PTNode(aVal),
	byte(aByte)
{
	//empty
}

unsigned char PTLeaf::Byte() const
{
	return byte;
}

//-------------------------------------------------------------------------------------------

UPTNode::UPTNode(UPTNode *aLeft, UPTNode *aRight):
	left(aLeft),
	right(aRight)
{
	//empty
}

void UPTNode::SetLeft(UPTNode *aNode)
{
	left = aNode;
}

void UPTNode::SetRight(UPTNode *aNode)
{
	right = aNode;
}

UPTNode *UPTNode::Left() const
{
		return left;
}

UPTNode *UPTNode::Right() const
{
	return right;
}

UPTLeaf::UPTLeaf(unsigned char aByte, unsigned int aCodeSize):
	byte(aByte),
	codeSize(aCodeSize)
{
	//empty
}

unsigned char UPTLeaf::Byte() const
{
	return byte;
}

unsigned int UPTLeaf::CodeSize() const
{
	return codeSize;
}

/****************************************************************************************************/
/****************************************** Packer Huffman ******************************************/
/****************************************************************************************************/

extern void WriteInt(FILE *aFOut, unsigned int aI);

void AddToListNode(LstPTNode &aLstNode, PTNode *aNode)
{
	LstPTNode::reverse_iterator it = aLstNode.rbegin();
	while (it != aLstNode.rend() && (*it)->Value() < aNode->Value())
	{
		++it;
	}
	aLstNode.insert(it.base(), aNode);
}

//Рекурсивная функция, реализующая удаление узлов в дереве
template<typename T>
void DelNode(T *aNode)
{
	if (aNode->Left())
	{
		DelNode(aNode->Left());
	}
	if (aNode->Right())
	{
		DelNode(aNode->Right());
	}
	delete aNode;
}

PackerHuffman::~PackerHuffman()
{
	DelNode(root);
}

PackerHuffman::PackerHuffman(unsigned int aBufSize):
	Packer(aBufSize, aHuffman),
	root(0)
{
	//empty
}

void PackerHuffman::_Pack(FILE *aFIn, FILE *aFOut, unsigned int aFSize, const Printer &aPr)
{
	vecByte.clear();
	dict.clear();
	if (root)
	{
		DelNode(root);
	}
	vecByte.resize(UCHAR_MAX + 1);
	dict.resize(UCHAR_MAX + 1);

	unsigned int fLen = InitVecByte(aFIn);
	WriteInt(aFOut, fLen);

	unsigned int leafCount = CreateTree();

	PCode curCode;
	CreateDictionary(root, curCode, MASK, 0);

	fseek(aFIn, 0, SEEK_SET);

	unsigned char curCh = 0;
	unsigned int curSize = 0;
	WriteDictionary(aFOut, leafCount, &curCh, &curSize);

	Coding(aFIn, aFOut, &curCh, &curSize, aFSize, aPr);

	//PrintReport();
}

//Функция, реализующая подсчеты вхождения каждого символа (символ есть байт) во входном файле. Первый проход
unsigned int PackerHuffman::InitVecByte(FILE *aFIn)
{
	unsigned int fLen = 0;
	unsigned char cur;
	while (SetNext(aFIn, &cur))
	{
		++vecByte[cur];
		++fLen;
	}
	return fLen;
}

//Функция, реализующая формирование дерева кодов Хаффмана. Возвращает количество листьев
unsigned int PackerHuffman::CreateTree()
{
	//Формирование основания (листья) дерева
	LstPTNode lstNode;
	for (unsigned int i = 0; i < vecByte.size(); ++i)
	{
		if (vecByte[i])
		{
			AddToListNode(lstNode, new PTLeaf((unsigned char)i, vecByte[i]));
		}
	}
	unsigned int leafCount = lstNode.size();

	//Построение дерева
	unsigned int n = lstNode.size();
	for (unsigned int i = 0; i < n - 1; ++i)
	{
		PTNode *r = lstNode.back();
		lstNode.pop_back();
		PTNode *l = lstNode.back();
		lstNode.pop_back();
		root = new PTNode(r->Value() + l->Value(), l, r);
		AddToListNode(lstNode, root);
	}
	return leafCount;
}

//Функция, реализующая построение словаря
void PackerHuffman::CreateDictionary(PTNode *aNode, PCode aCur, unsigned char aMask, unsigned int aDepth)
{
	if (!aNode->Left() && !aNode->Right())	//Если лист
	{
		PTLeaf *leaf = static_cast<PTLeaf *>(aNode);
		dict[leaf->Byte()].first = aCur;
		dict[leaf->Byte()].second = aDepth;
		return;
	}
	if (!(aDepth % BIT_IN_BYTE))
	{
		aCur.push_back(0);
		aMask = MASK;
	}
	CreateDictionary(aNode->Left(), aCur, aMask >> 1, aDepth + 1);
	aCur.back() |= aMask;
	CreateDictionary(aNode->Right(), aCur, aMask >> 1, aDepth + 1);
}

//Функция реализующая склеивание битов в байт и вывод его в выходной файл
inline void PackerHuffman::WriteCode(FILE *aFOut, const PElem &aElem, unsigned char *aCurCh, unsigned int *aCurSize)
{
	unsigned int tmpSize;
	unsigned int tmp;
	unsigned int step = 1;

	for (PCode::const_iterator it = aElem.first.begin(); it != aElem.first.end(); ++it)
	{
		*aCurCh |= *it >> *aCurSize;
		tmpSize = step == aElem.first.size() ? aElem.second - BIT_IN_BYTE * (step - 1) : BIT_IN_BYTE;
		tmp = *aCurSize + tmpSize;
		if (tmp >= BIT_IN_BYTE)
		{
			fprintf(aFOut, "%c", *aCurCh);
			*aCurCh = *it << (BIT_IN_BYTE - *aCurSize);
			*aCurSize = tmp - BIT_IN_BYTE;
		}
		else
		{
			*aCurSize = tmp;
		}
		++step;
	}
}

//Функция реализующая кодирование. Чтение данных из входного файла и, на основании словаря, вывод данных в выходной файл. Второй проход
void PackerHuffman::Coding(FILE *aFIn, FILE *aFOut, unsigned char *aCurCh, unsigned int *aCurSize, unsigned int aFSize, const Printer &aPr)
{
	unsigned char cur;
	unsigned int i = 0;
	while (SetNext(aFIn, &cur))
	{
		aPr.PrintPercent((double)(i++) / aFSize * 100);
		WriteCode(aFOut, dict[cur], aCurCh, aCurSize);
	}
	if (*aCurSize)
	{
		fprintf(aFOut, "%c", *aCurCh);
	}
	aPr.PrintPercent(100);
}

void PackerHuffman::PrintReport() const
{
	FILE *fRep;
	if (fopen_s(&fRep, "reportPack.rep", "wb"))
	{
		throw Error("Failed to create file reportPack.rep");
	}
	unsigned int sumLegnth = 0;
	for (unsigned int i = 0; i < dict.size(); ++i)
	{
		fprintf(fRep, "%c\tCounter : %i\t\t\tLength code : %i\n", (unsigned char)i, vecByte[i], dict[i].second);
		sumLegnth += dict[i].second * vecByte[i];
	}
	fprintf(fRep, "%i", sumLegnth / 8 + (!(sumLegnth % 8) ? 0 : 1));
	fclose(fRep);
}

void PackerHuffman::WriteDictionary(FILE *aFOut, unsigned int aDictSize, unsigned char *aCurCh, unsigned int *aCurSize)
{
	fprintf(aFOut, "%c", (unsigned char)aDictSize);		//Вывод в выходной файл количества элементов в словаре

	//Вывод в выходной файл элемента словаря, а за ним количество бит в его коде. Цикл для всех элементов в словаре
	for (unsigned int i = 0; i < dict.size(); ++i)
	{
		if (dict[i].second)
		{
			fprintf(aFOut, "%c%c", (unsigned char)i, (unsigned char)dict[i].second);
		}
	}
	//Вывод кодов элементов в словаре (непрерывная последовательность битов)
	for (unsigned int i = 0; i < dict.size(); ++i)
	{
		if (dict[i].second)
		{
			WriteCode(aFOut, dict[i], aCurCh, aCurSize);
		}
	}
}

/****************************************************************************************************/
/***************************************** UnPacker Huffman *****************************************/
/****************************************************************************************************/

UnPackerHuffman::UnPackerHuffman(unsigned int aBufSize):
	ProxyUnPacker(aBufSize),
	root(0)
{
	//empty
}

UnPackerHuffman::~UnPackerHuffman()
{
	DelNode(root);
}

void UnPackerHuffman::_UnPack(FILE *aFIn, FILE *aFOut, const Printer &aPr)
{
	lstLeaf.clear();
	if (root)
	{
		DelNode(root);
	}
	root = new UPTNode();

	unsigned int fLen = ReadInt(aFIn);
	ReadDictionary(aFIn);

	unsigned char curCh = 0;
	unsigned int curSize = 0;
	CreateTree(aFIn, &curCh, &curSize);

	Decoding(aFIn, aFOut, &curCh, &curSize, fLen, aPr);
}

//Функция, реализующая чтение элементов словаря и размер (в битах) кода каждого элемента из входного файла. 
void UnPackerHuffman::ReadDictionary(FILE *aFIn)
{
	unsigned char ch1, ch2;
	unsigned int bitCount = 0;

	_SetNext(aFIn, &ch1);
	unsigned int dictSize = !ch1 ? UCHAR_MAX + 1 : ch1;

	for (unsigned int i = 0; i < dictSize; ++i)
	{
		_SetNext(aFIn, &ch1);
		_SetNext(aFIn, &ch2);
		lstLeaf.push_back(new UPTLeaf(ch1, ch2));
	}
}

//Функция, реализующая построение дерева при распаковке. Возвращает указатель на корень этого дерева
void UnPackerHuffman::CreateTree(FILE *aFIn, unsigned char *aCurCh, unsigned int *aCurSize)
{
	//Разбиение на биты считанных байтов, построение дерева
	for (std::list<UPTLeaf *>::iterator it = lstLeaf.begin(); it != lstLeaf.end(); ++it)
	{
		UPTNode *node = root;
		for (unsigned int i = 0; i < (*it)->CodeSize() - 1; ++i)
		{
			if (NextBit(aFIn, aCurCh, aCurSize))
			{
				if (node->Right())
				{
					node = node->Right();
				}
				else
				{
					node->SetRight(new UPTNode());
					node = node->Right();
				}
			}
			else
			{
				if (node->Left())
				{
					node = node->Left();
				}
				else
				{
					node->SetLeft(new UPTNode());
					node = node->Left();
				}
			}
		}
		if (NextBit(aFIn, aCurCh, aCurSize))
		{
			node->SetRight(*it);
		}
		else
		{
			node->SetLeft(*it);
		}
	}
}

//Функция, реализующая сканирование байта, возвращая является ли старший бит 1. При этом проиходит сдвиг битов в байте
//Если байт просканирован целиком, то берется следующий байт из буфера
inline bool UnPackerHuffman::NextBit(FILE *aFIn, unsigned char *aCurCh, unsigned int *aCurSize)
{
	unsigned char tmpCh;
	if (!*aCurSize)
	{
		_SetNext(aFIn, aCurCh);
		*aCurSize = BIT_IN_BYTE;
	}
	tmpCh = *aCurCh & MASK;
	*aCurCh <<= 1;
	--*aCurSize;
	return tmpCh != 0;
}

//Функция реализующая декодирование
void UnPackerHuffman::Decoding(FILE *aFIn, FILE *aFOut, unsigned char *aCurCh, unsigned int *aCurSize, unsigned int aFLen, const Printer &aPr)
{
	UPTNode *node = root;
	unsigned int fSize = aFLen;
	unsigned int i = 0;
	while (aFLen)
	{
		node = NextBit(aFIn, aCurCh, aCurSize) ? node->Right() : node->Left();
		if (!node->Left())		//достаточно проверки одного  потомка
		{
			fprintf(aFOut, "%c", static_cast<UPTLeaf *>(node)->Byte());
			node = root;
			aPr.PrintPercent((double)(i++) / fSize * 100);
			--aFLen;
		}
	}
	aPr.PrintPercent(100);
}