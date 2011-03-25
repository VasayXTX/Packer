#include "../Header/packerRLE.h"

/****************************************************************************************************/
/***************************************** Packer RLE ***********************************************/
/****************************************************************************************************/

extern void WriteInt(FILE *aFOut, unsigned int aI);

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

void PackerRLE::_Pack(FILE *aFIn, FILE *aFOut, unsigned int aFSize, const Printer &aPr)
{
	WriteInt(aFOut, 0);
	unsigned int fLen = Coding(aFIn, aFOut, aFSize, aPr);
	fseek(aFOut, -(int)fLen - sizeof(int), SEEK_CUR);
	WriteInt(aFOut, fLen);
	fseek(aFOut, fLen, SEEK_CUR);
}

unsigned int PackerRLE::Coding(FILE *aFIn, FILE *aFOut, unsigned int aFSize, const Printer &aPr)
{
	unsigned char counter = 0;
	bool isMatch = false, isNew = true;
	unsigned char cur, prev = 0;
	unsigned int fLen = 0;	//Длина получившегося кода
	unsigned int i = 0;
	while (SetNext(aFIn, &cur))
	{
		aPr.PrintPercent((unsigned int)((double)(i++) / aFSize * 100));
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
/****************************************** UnPacker RLE ********************************************/
/****************************************************************************************************/

UnPackerRLE::UnPackerRLE(unsigned int aBufSize):
	ProxyUnPacker(aBufSize)
{
	//empty
}

void UnPackerRLE::_UnPack(FILE *aFIn, FILE *aFOut, const Printer &aPr)
{
	unsigned char prev = 0;
	unsigned char cur;
	bool isNew = true, isMatch = false;

	unsigned int fSize = ReadInt(aFIn);
	unsigned int fLen = fSize;
	unsigned int i = 0;

	while (fLen--)
	{
		_SetNext(aFIn, &cur);
		aPr.PrintPercent((unsigned int)((double)(i++) / fSize * 100));
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
