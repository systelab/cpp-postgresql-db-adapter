#pragma once

#include "DbAdapterInterface/IBinaryValue.h"

namespace systelab { namespace db { namespace postgresql {

	class BinaryValue : public IBinaryValue
	{
	public:
		BinaryValue(std::istream& inputStream);
		~BinaryValue() override = default;

		std::ostream getOutputStream() const override;
		std::istream getInputStream() const override;

	private:
		std::ostringstream m_buffer;
	};
}}}