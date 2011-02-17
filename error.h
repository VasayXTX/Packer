#ifndef ERROR_H
#define ERROR_H

#include <string>

class Error
{
private:
	std::string text;

public:
	Error(const std::string &aText);

	const std::string &Text() const;
};

#endif