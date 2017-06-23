#pragma once
#include "Header.h"
class serializer {
public:
	serializer(std::string&& ext, Mat& frame)
		: lnk(frame), buf(), str(std::forward<std::string>(ext))
	{}
	void method()
	{
		imencode(std::forward<std::string>(str), lnk, buf);
	}
private:
	std::string str;
	Mat& lnk;
	std::vector<uchar> buf;
};
class deserializer {
public:
	deserializer(std::vector<uchar>& buf) : vec(buf)
	{}
	Mat decode()
	{
		return imdecode(vec, IMREAD_UNCHANGED);
	}
private:
	std::vector<uchar>& vec;
};