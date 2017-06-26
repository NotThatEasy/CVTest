#pragma once
#include "Header.h"

template <class T>
class serializer{
public:
	serializer();
	virtual void encode() = 0;
private:
	std::string str;
	T& lnk;
	std::vector<uchar> buf;
};


class matSerializer :  serializer<Mat> {
public:
	matSerializer(std::string&& ext, Mat& frame)
		: lnk(frame), buf(), str(std::forward<std::string>(ext))
	{}
	void encode()
	{
		imencode(std::forward<std::string>(str), lnk, buf);
	}
private:
	std::string str;
	Mat& lnk;
	std::vector<uchar> buf;
};

template<class T>
class deserializer {
public:
	deserializer();
	virtual void decode(T& whereTo) = 0;
private:
	std::vector<uchar>& vec;
};
class matDeserializer : deserializer<Mat> 
{
public:
	matDeserializer(std::vector<uchar>& vec) : vec(vec) {}
	void decode(Mat& whereTo) 
	{
		whereTo = imdecode(vec, IMREAD_UNCHANGED);
	}
private:
	std::vector<uchar>& vec;
};
