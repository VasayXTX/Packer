#ifndef PRINTER_H
#define PRINTER_H

#include <string>
#include <iostream>
#include <time.h>

#include "error.h"

class Printer
{
public:
	Printer();

	void PrintLnText(const std::string &aText) const;
	void PrintText(const std::string &aText) const;
	void PrintAbout() const;
	void PrintHelp() const;
	void PrintError(const Error &aErr) const;
	void PrintPercent(unsigned int aVal) const;
	void PrintTime(const std::string &aText, time_t aT) const;
};

#endif