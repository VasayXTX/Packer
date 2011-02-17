#ifndef PRINTER_H
#define PRINTER_H

#include <string>
#include <iostream>

#include "error.h"

class Printer
{
public:
	Printer();

	void PrintText(const std::string &aText) const;
	void PrintAbout() const;
	void PrintHelp() const;
	void PrintError(const Error &aErr) const;
};

#endif