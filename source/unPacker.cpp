#include "../Header/unPacker.h"

/****************************************************************************************************/
/********************************************* UnPacker *********************************************/
/****************************************************************************************************/

UnPacker::UnPacker(unsigned int aBufSize):
	bufSize(aBufSize)
{
	//empty
}

void UnPacker::UnPack(const std::string &aFileIn, const std::string &aDir)
{
	FILE *fOut, *fIn;
	if (fopen_s(&fIn, aFileIn.c_str(), "rb") || !fIn)
	{
		throw Error("Failed to open file " + aFileIn);
	}

	unsigned char cAlg;
	if (!fscanf_s(fIn, "%c", &cAlg))
	{
		throw Error("Error reading file");
	}

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
		fName = aDir + fName;
		//**************************
		//fName = "_" + fName;
		//**************************
		if (fopen_s(&fOut, fName.c_str(), "wb") || !fOut)
		{
			throw Error("Failed to create file " + fName);
		}

		Printer pr;
		pr.PrintText("Decompression " + fName + ": 0%");
		unP->_UnPack(fIn, fOut, pr);
		pr.PrintPercent(100);
		pr.PrintText("\n");

		fclose(fOut);
	}
	delete unP;
	fclose(fIn);
}