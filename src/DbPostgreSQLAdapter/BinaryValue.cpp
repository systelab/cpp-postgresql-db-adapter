#include "stdafx.h"
#include "BinaryValue.h"

#include <streambuf>


namespace systelab { namespace db { namespace postgresql {

	BinaryValue::BinaryValue(std::istream& inputStream)
	{

	}

	std::ostream BinaryValue::getOutputStream() const
	{
		throw("Not implemented");
	}

	std::istream BinaryValue::getInputStream() const
	{
		throw("Not implemented");
	}

}}}