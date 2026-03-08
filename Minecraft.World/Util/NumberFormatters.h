#pragma once


// 4J Stu - The java formated numbers based on a local passed in, but I am just going for a constant format here
class NumberFormat
{
public:
	static std::wstring format(int value)
	{
		// TODO 4J Stu - Change the length of the formatted number
		wchar_t output[256];
		swprintf( output, 256, L"%d", value);
		std::wstring result = std::wstring( output );
		return result;
	}
};


class DecimalFormat
{
private:
	const std::wstring formatString;
public:
	std::wstring format(double value)
	{
		// TODO 4J Stu - Change the length of the formatted number
		wchar_t output[256];
		swprintf( output, 256, formatString.c_str(), value);
		std::wstring result = std::wstring( output );
		return result;
	}

	// 4J Stu - The java code took a string format, we take a printf format string
	DecimalFormat(std::wstring x) : formatString( x ) {};
};