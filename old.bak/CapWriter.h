#pragma once
#include "Header.h"




class CapWriter{
public:
	CapWriter(std::string&& fileOutName, unsigned short code) : cap(code)
	{
		
		//Checking if input stream is ok
		if (!cap.isOpened())
			lError(capErrorText, 1);

		//Creating and opening an output video file with VideoWriter
		wrt.open(std::forward<std::string>(fileOutName), -1, FPS, Size(width, height));

		//Checking if output stream is ok
		if (!wrt.isOpened())
			lError(wrtErrorText, 2);
	}
	~CapWriter()
	{
		cap.release();
		wrt.release();
	}
	Size& frameSize()
	{
		return Size(width, height);
	}
	void operator >> (Mat& object)
	{
		cap >> object;
	}
	void operator<< (Mat& object)
	{
		wrt << object;
	}

private:
	unsigned short FPS;
	unsigned short width;
	unsigned short height;
	VideoCapture cap;
	VideoWriter wrt;
};
