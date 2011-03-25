#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <time.h>

#include "../Header/packer.h"
#include "../Header/unPacker.h"
#include "../Header/packerRLE.h"
#include "../Header/packerHuffman.h"
#include "../Header/error.h"
#include "../Header/printer.h"

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
	//argc = 5;
	//argv[1] = "/c";
	//argv[2] = "/r";
	//argv[3] = "D:\\Packer\\Tests\\RLE\\Output\\test00(zero).pck";
	//argv[4] = "D:\\Packer\\Tests\\RLE\\Input\\test00(zero).tst";

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
				lstFile.push_back(argv[i]);
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
	//system("pause");
	return 0;
}
