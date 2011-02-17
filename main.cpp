#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <time.h>

#include "packer.h"

void InitFile(const std::string &aFName)
{
	FILE *fOut = fopen(aFName.c_str(), "wb");

	unsigned int fLen = 1;

	for (unsigned int c = 0; c <= UCHAR_MAX; ++c)
	{
		for (unsigned int i = 0; i < fLen; ++i)
		{
			fprintf(fOut, "%c", (unsigned char)c);
		}
		//++fLen;
	}

	fclose(fOut);
}

int main()
{
	//InitFile("input.in");
	InitFile("input2.in");
	InitFile("input3.in");

	//PackerHuffman packer;

	//Packed
	LstFile lstFile;
	lstFile.push_back("input2.in");
	lstFile.push_back("input.in");
	lstFile.push_back("input3.in");
	PackerHuffman p;
	time_t tBegin = time(0);
	p.Pack(lstFile, "output.smr");
	time_t tEnd = time(0);
	printf("Time packing: %i\n", tEnd - tBegin);

	////UnPacked
	UnPacker up;
	tBegin = time(0);
	up.UnPack("output.smr");
	tEnd = time(0);
	printf("Time unpacking: %i\n", tEnd - tBegin);

	char ch;
	scanf("%c", &ch);
	return 0;
}
