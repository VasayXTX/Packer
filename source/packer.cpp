#include "../Header/packer.h"

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

std::string GetShortFileName(const std::string &aStr)
{
	unsigned int i = i = aStr.size();
	while (i > 0 && aStr[i-1] != '\\')
	{
		--i;
	}
	std::string res;
	while (i < aStr.size())
	{
		res.push_back(aStr[i++]);
	}
	return res;
}

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
	FILE *fIn, *fOut;
	if (fopen_s(&fOut, aFileOut.c_str(), "wb") || !fOut)
	{
		throw Error("Failed to create file " + aFileOut);
	}

	fprintf(fOut, "%c", (unsigned char)alg);	//записываем в выходной файл вид алгоритма, на основании которого идет архивация
	WriteInt(fOut, aLstFileIn.size());			//записываем в выходной файл количество файлов для архивации

	for (LstFile::const_iterator it = aLstFileIn.begin(); it != aLstFileIn.end(); ++it)
	{
		time_t tBegin = time(0);
		if (fopen_s(&fIn, (*it).c_str(), "rb") || !fIn)
		{
			throw Error("Failed to open file " + *it);
		}

		std::string shortFName = GetShortFileName(*it);
		fprintf(fOut, "%c", (unsigned char)shortFName.size());	//записываем в выходной файл размер имени архивируемого файла
		for (unsigned int i = 0; i < shortFName.size(); ++i)	//записываем в выходной файл имя архивируемого файла
		{
			fprintf(fOut, "%c", shortFName[i]);
		}

		//Определяем размер файла
		fseek(fIn, 0, SEEK_END);
		unsigned int fSize = ftell(fIn);
		fseek(fIn, 0, SEEK_SET);

		Printer pr;
		pr.PrintText("Compression " + *it + ": 0%");
		_Pack(fIn, fOut, fSize, pr);
		pr.PrintPercent(100);
		pr.PrintText("\n");

		fclose(fIn);
	}
	fclose(fOut);
}

/****************************************************************************************************/
/***************************************** ProxyUnPacker  *******************************************/
/****************************************************************************************************/

ProxyUnPacker::ProxyUnPacker(unsigned int aBufSize):
	Reader(aBufSize)
{
	//empty
}