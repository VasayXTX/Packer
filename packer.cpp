#include "packer.h"

/****************************************************************************************************/
/******************************************** Huffman ***********************************************/
/****************************************************************************************************/

Huffman::PTNode::PTNode(unsigned int aVal, PTNode *aLeft, PTNode *aRight):
	left(aLeft),
	right(aRight),
	val(aVal)
{
	//empty
}

Huffman::PTNode *Huffman::PTNode::Left() const
{
	return left;
}

Huffman::PTNode *Huffman::PTNode::Right() const
{
	return right;
}

unsigned int Huffman::PTNode::Value() const
{
	return val;
}

Huffman::PTLeaf::PTLeaf(unsigned char aByte, unsigned int aVal):
	PTNode(aVal),
	byte(aByte)
{
	//empty
}

unsigned char Huffman::PTLeaf::Byte() const
{
	return byte;
}

//-------------------------------------------------------------------------------------------

Huffman::UPTNode::UPTNode(UPTNode *aLeft, UPTNode *aRight):
	left(aLeft),
	right(aRight)
{
	//empty
}

void Huffman::UPTNode::SetLeft(UPTNode *aNode)
{
	left = aNode;
}

void Huffman::UPTNode::SetRight(UPTNode *aNode)
{
	right = aNode;
}

Huffman::UPTNode *Huffman::UPTNode::Left() const
{
		return left;
}

Huffman::UPTNode *Huffman::UPTNode::Right() const
{
	return right;
}

Huffman::UPTLeaf::UPTLeaf(unsigned char aByte, unsigned int aCodeSize):
	byte(aByte),
	codeSize(aCodeSize)
{
	//empty
}

unsigned char Huffman::UPTLeaf::Byte() const
{
	return byte;
}

unsigned int Huffman::UPTLeaf::CodeSize() const
{
	return codeSize;
}

/****************************************************************************************************/
/********************************************* Reader ***********************************************/
/****************************************************************************************************/

Reader::Reader(unsigned int aBufSize):
	bufSize(aBufSize),
	buf(new unsigned char[bufSize]),
	counter(0),
	bufLen(0)
{
	//empty
}

Reader::~Reader()
{
	delete[] buf;
}

inline bool Reader::SetNext(FILE *aFIn, unsigned char *aCh)
{
	if (counter == bufLen || !bufLen)
	{
		if ((bufLen = fread(buf, sizeof(char), bufSize, aFIn)) <= 0)
		{
			return false;
		}
		counter = 0;
	}
	*aCh = buf[counter++];
	return true;
}

inline void Reader::_SetNext(FILE *aFIn, unsigned char *aCh)
{
	if (!SetNext(aFIn, aCh))
	{
		throw Error("Error in decompression. Perhaps the archive is corrupted");
	}
}

//Функция, реализующая чтение размера файла в несжатом виде
unsigned int Reader::ReadInt(FILE *aFIn)
{
	unsigned int res = 0;
	unsigned char *pC = (unsigned char *)(&res);
	for (unsigned int i = 0; i < sizeof(res); ++i)
	{
		unsigned char c;
		_SetNext(aFIn, &c);
		*pC++ = c;
	}
	return res;
}

/****************************************************************************************************/
/********************************************* Packer ***********************************************/
/****************************************************************************************************/

//Функция, реализующая запись размера файла в несжатом виде в выходной файл
void WriteInt(FILE *aFOut, unsigned int aI)
{
	unsigned char *c = (unsigned char *)&aI;
	for (unsigned int i = 0; i < sizeof(aI); ++i)
	{
		fprintf(aFOut, "%c", *c++);
	}
}

Packer::Packer(unsigned int aBufSize, Alg aAlg):
	Reader(aBufSize),
	alg(aAlg)
{
	//empty
}

void Packer::Pack(const LstFile &aLstFileIn, const std::string &aFileOut)
{
	FILE *fIn, *fOut = fopen(aFileOut.c_str(), "wb");
	if (!fOut)
	{
		throw Error("Failed to create file " + aFileOut);
	}

	fprintf(fOut, "%c", (unsigned char)alg);	//записываем в выходной файл вид алгоритма, на основании которого идет архивация
	WriteInt(fOut, aLstFileIn.size());			//записываем в выходной файл количество файлов для архивации

	for (LstFile::const_iterator it = aLstFileIn.begin(); it != aLstFileIn.end(); ++it)
	{
		fIn = fopen((*it).c_str(), "rb");
		if (!fIn)
		{
			throw Error("Failed to open file " + *it);
		}

		fprintf(fOut, "%c", (unsigned char)it->size());	//записываем в выходной файл размер имени архивируемого файла
		for (unsigned int i = 0; i < it->size(); ++i)	//записываем в выходной файл имя архивируемого файла
		{
			fprintf(fOut, "%c", (*it)[i]);
		}
		_Pack(fIn, fOut);

		fclose(fIn);
	}
	fclose(fOut);
}

/****************************************************************************************************/
/***************************************** Packer RLE ***********************************************/
/****************************************************************************************************/

inline void WriteRLECode(FILE *aFOut, unsigned char aCh, unsigned int *aFLen)
{
	fprintf(aFOut, "%c", aCh);
	++*aFLen;
}

PackerRLE::PackerRLE(unsigned int aBufSize):
	Packer(aBufSize, aRLE)
{
	//empty
}

void PackerRLE::_Pack(FILE *aFIn, FILE *aFOut)
{
	WriteInt(aFOut, 0);		//Записываем нули, чтобы потом вернуться и записать длину (в байтах) закодированного файла
	unsigned int fLen = Coding(aFIn, aFOut);
	fseek(aFOut, -fLen - sizeof(int), SEEK_CUR);
	WriteInt(aFOut, fLen);
	fseek(aFOut, fLen, SEEK_CUR);
}

unsigned int PackerRLE::Coding(FILE *aFIn, FILE *aFOut)
{
	unsigned char counter = 0;
	bool isMatch = false, isNew = true;
	unsigned char cur, prev = 0;
	unsigned int fLen = 0;	//Длина получившегося кода

	while (SetNext(aFIn, &cur))
	{
		if (isMatch)
		{
			if (cur == prev)
			{
				++counter;
				if (counter == UCHAR_MAX)
				{
					WriteRLECode(aFOut, counter, &fLen);
					counter = 0;
					isNew = true;
					isMatch = false;
				}
			}
			else
			{	
				fprintf(aFOut, "%c%c", counter, cur);
				fLen += 2;
				counter = 0;
				prev = cur;
				isMatch = false;
			}
			continue;
		}
		WriteRLECode(aFOut, cur, &fLen);
		if (!isNew && cur == prev)
		{
			isMatch = true;
		}
		else
		{
			prev = cur;
			isNew = false;
		}
	}
	if (isMatch)
	{
		WriteRLECode(aFOut, counter, &fLen);
	}
	return fLen;
}

/****************************************************************************************************/
/****************************************** Packer Huffman ******************************************/
/****************************************************************************************************/

void AddToListNode(Huffman::LstPTNode &aLstNode, Huffman::PTNode *aNode)
{
	Huffman::LstPTNode::reverse_iterator it = aLstNode.rbegin();
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

void PackerHuffman::_Pack(FILE *aFIn, FILE *aFOut)
{
	using namespace Huffman;

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

	Coding(aFIn, aFOut, &curCh, &curSize);

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
	using namespace Huffman;

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
void PackerHuffman::CreateDictionary(Huffman::PTNode *aNode, Huffman::PCode aCur, unsigned char aMask, unsigned int aDepth)
{
	using namespace Huffman;

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
inline void PackerHuffman::WriteCode(FILE *aFOut, const Huffman::PElem &aElem, unsigned char *aCurCh, unsigned int *aCurSize)
{
	using namespace Huffman;

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
void PackerHuffman::Coding(FILE *aFIn, FILE *aFOut, unsigned char *aCurCh, unsigned int *aCurSize)
{
	unsigned char cur;
	while (SetNext(aFIn, &cur))
	{
		WriteCode(aFOut, dict[cur], aCurCh, aCurSize);
	}
	if (*aCurSize)
	{
		fprintf(aFOut, "%c", *aCurCh);
	}
}

void PackerHuffman::PrintReport() const
{
	FILE *fRep = fopen("reportPack.rep", "wb");
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
/********************************************* UnPacker *********************************************/
/****************************************************************************************************/

UnPacker::UnPacker(unsigned int aBufSize):
	bufSize(aBufSize)
{
	//empty
}

void UnPacker::UnPack(const std::string &aFileIn)
{
	FILE *fOut, *fIn = fopen(aFileIn.c_str(), "rb");
	if (!fIn)
	{
		throw Error("Failed to open file " + aFileIn);
	}

	unsigned char cAlg;
	fscanf(fIn, "%c", &cAlg);

	ProxyUnPacker *unP;
	switch (cAlg)
	{
		case 0:
			unP = new UnPackerRLE(bufSize);
			break;
		case 1:
			unP = new UnPackerHuffman(bufSize);
			break;
		default:
			throw Error("Unknown type of compression");
	}
	unsigned int fileCount = unP->ReadInt(fIn);
	for (unsigned int i = 0; i < fileCount; ++i)
	{
		unsigned char fNameLen;
		unP->_SetNext(fIn, &fNameLen);
		std::string fName;
		unsigned char ch;
		for (unsigned int j = 0; j < fNameLen; ++j)
		{
			unP->_SetNext(fIn, &ch);
			fName.push_back(ch);
		}
		//**************************
		//fName = "_" + fName;
		//**************************
		fOut = fopen(fName.c_str(), "wb");
		if (!fOut)
		{
			throw Error("Failed to create file " + fName);
		}
		unP->UnPack(fIn, fOut);
		fclose(fOut);
	}
	delete unP;
	fclose(fIn);
}

ProxyUnPacker::ProxyUnPacker(unsigned int aBufSize):
	Reader(aBufSize)
{
	//empty
}

/****************************************************************************************************/
/****************************************** UnPacker RLE ********************************************/
/****************************************************************************************************/

UnPackerRLE::UnPackerRLE(unsigned int aBufSize):
	ProxyUnPacker(aBufSize)
{
	//empty
}

void UnPackerRLE::UnPack(FILE *aFIn, FILE *aFOut)
{
	unsigned char prev = 0;
	unsigned char cur;
	bool isNew = true, isMatch = false;

	unsigned int fLen = ReadInt(aFIn);

	while (fLen--)
	{
		if (isMatch)
		{
			for (unsigned int j = 0; j < cur; ++j)
			{
				fprintf(aFOut, "%c", prev);
			}
			isNew = true;
			isMatch = false;
		}
		else
		{
			fprintf(aFOut, "%c", cur);
			if (!isNew && prev == cur)
			{
				isMatch = true;
			}
			else
			{
				prev = cur;
				isNew = false;
			}
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

void UnPackerHuffman::UnPack(FILE *aFIn, FILE *aFOut)
{
	lstLeaf.clear();
	if (root)
	{
		DelNode(root);
	}
	root = new Huffman::UPTNode();

	unsigned int fLen = ReadInt(aFIn);
	ReadDictionary(aFIn);

	unsigned char curCh = 0;
	unsigned int curSize = 0;
	CreateTree(aFIn, &curCh, &curSize);

	Decoding(aFIn, aFOut, &curCh, &curSize, fLen);
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
		lstLeaf.push_back(new Huffman::UPTLeaf(ch1, ch2));
	}
}

//Функция, реализующая построение дерева при распаковке. Возвращает указатель на корень этого дерева
void UnPackerHuffman::CreateTree(FILE *aFIn, unsigned char *aCurCh, unsigned int *aCurSize)
{
	using namespace Huffman;

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
	unsigned int tmpCh;
	if (!*aCurSize)
	{
		_SetNext(aFIn, aCurCh);
		*aCurSize = BIT_IN_BYTE;
	}
	tmpCh = *aCurCh & MASK;
	*aCurCh <<= 1;
	--*aCurSize;
	return tmpCh;
}

//Функция реализующая декодирование
void UnPackerHuffman::Decoding(FILE *aFIn, FILE *aFOut, unsigned char *aCurCh, unsigned int *aCurSize, unsigned int aFLen)
{
	using namespace Huffman;

	UPTNode *node = root;
	while (aFLen)
	{
		node = NextBit(aFIn, aCurCh, aCurSize) ? node->Right() : node->Left();
		if (!node->Left())		//достаточно проверки одного  потомка
		{
			fprintf(aFOut, "%c", static_cast<Huffman::UPTLeaf *>(node)->Byte());
			node = root;
			--aFLen;
		}
	}
}