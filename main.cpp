#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <time.h>

#include "packer.h"
#include "unPacker.h"
#include "packerRLE.h"
#include "packerHuffman.h"
#include "error.h"
#include "printer.h"

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

int main(int argc, char *argv[])
{
	//argc = 4;
	//argv[1] = "/d";
	////argv[2] = "/h";	
	//argv[2] = "output.out";
	//argv[3] = "D:\\UnPacking";

	/*argv[4] = "input1.in";
	argv[5] = "input.in";
	argv[6] = "input3.in";*/

	Printer pr;

	if (argc == 1)
	{
		pr.PrintAbout();
		return 0;
	}
	time_t tBegin = time(0);
	try
	{
		if (argc == 2)
		{
			if (!strcmp(argv[1], "/h"))
			{
				pr.PrintHelp();
				return 0;
			}
			throw Error(std::string("Invalid key: ") + std::string(argv[1]));
		}
		if (!strcmp(argv[1], "/c"))
		{
			if (argc < 4)
			{
				throw Error("Not specified name of archived file");
			}
			if (argc < 5)
			{
				throw Error("Not specified files for archiving");
			}

			LstFile lstFile;
			for (int i = 4; i < argc; ++i)
			{
				lstFile.push_back(GetShortFileName(argv[i]));
			}

			if (!strcmp(argv[2], "/r"))
			{
				PackerRLE::PackerRLE().Pack(lstFile, argv[3]);
			}
			else if (!strcmp(argv[2], "/h"))
			{
				PackerHuffman::PackerHuffman().Pack(lstFile, argv[3]);
			}
			else
			{
				throw Error(std::string("Invalid key: ") + std::string(argv[2]));
			}
		}
		else if (!strcmp(argv[1], "/d"))
		{
			if (argc > 4)
			{
				throw Error(std::string("Invalid key: ") + std::string(argv[4]));
			}
			std::string dir;
			if (argc == 4)
			{
				dir = argv[3];
				if (dir[dir.size()-1] != '\\')
				{
					dir.push_back('\\');
				}
			}
			UnPacker::UnPacker().UnPack(argv[2], dir);
		}
		else
		{
			throw Error(std::string("Invalid key: ") + std::string(argv[1]));
		}
	}
	catch (Error &aErr)
	{
		pr.PrintError(aErr);
		return 0;
	}
	pr.PrintTime("Time at work: ", time(0) - tBegin);
//	system("pause");
	return 0;
}
