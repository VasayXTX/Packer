#include "printer.h"

Printer::Printer()
{
	//empty
}

void Printer::PrintText(const std::string &aText) const
{
	std::cout << aText << '\n';
}

void Printer::PrintAbout() const
{
	std::cout << "Packer v1.0.\nCopyright(c) Vasiliy Tsubenko\n";
	std::cout << "For more information, run program with key \"/h\"\n";
}

void Printer::PrintHelp() const
{
	std::cout << "Packer v1.0.\nCopyright(c) Vasiliy Tsubenko\n";
	std::cout << "Program for compression data\n\n";
	std::cout << "How to use program for compression:\n";
	std::cout << "> Packer.exe /c ALG ARCHIVE_NAME FILE_LIST\n";
	std::cout << "ALG:\n";
	std::cout << "\t/r\tcompression using RLE\n";
	std::cout << "\t/h\tcompression using Huffman\n";
	std::cout << "ARCHIVE_NAME:\n\tName of the archived file\n";
	std::cout << "LIST_FILE\n\tList of files for compressing (FILE_NAME1, FILE_NAME2, ..)\n\n";
	std::cout << "How to use program for decompression:\n";
	std::cout << "> Packer.exe /d ARCHIVE_NAME\n";
	std::cout << "ARCHIVE_NAME:\n\tName of the archived file\n";
}

void Printer::PrintError(const Error &aErr) const
{
	std::cout << "Error: " << aErr.Text() << '\n';
}