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
	std::cout << "Packer v1.0. Program for compression data\n";
	std::cout << "Copyright(c) Vasiliy Tsubenko\n";
	std::cout << "For more information, run program with key \"/h\"\n";
}

void Printer::PrintHelp() const
{
	std::cout << "Packer v1.0. Program for compression data\n";
	std::cout << "Copyright(c) Vasiliy Tsubenko\n\n";
	std::cout << "\tHow to use program for compression:\n";
	std::cout << "\t> Packer.exe /c ALG ARCHIVE_FILE_NAME LIST_FILE_NAME\n";
	std::cout << "\t\tALG:\n";
	std::cout << "\t\t\t/r\tcompression using RLE\n";
	std::cout << "\t\t\t/h\tcompression using Huffman\n";
	std::cout << "\t\tARCHIVE_FILE_NAME:\n";
	std::cout << "\t\t\tName of archived file\n";
	std::cout << "\t\tLIST_FILE_NAME:\n";
	std::cout << "\t\t\tList of files for compressing (FILE_NAME1, FILE_NAME2, ..)\n\n";
	std::cout << "\tHow to use program for decompression:\n";
	std::cout << "\t> Packer.exe /d ARCHIVE_FILE_NAME\n";
	std::cout << "\t\tARCHIVE_FILE_NAME:\n";
	std::cout << "\t\t\tName of archived file";
}

void Printer::PrintError(const Error &aErr) const
{
	std::cout << "Error: " << aErr.Text() << '\n';
}