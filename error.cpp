#include "error.h"

Error::Error(const std::string &aText):
	text(aText)
{
	//empty
}

const std::string &Error::Text() const
{
	return text;
}