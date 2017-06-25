#pragma once
#include "Header.h"




class CapWriter : src<std::string>, dest<unsigned short>{
public:
	CapWriter(std::string&& fileOutName, unsigned short code) : cap(code)
		/*, FPS(static_cast<unsigned short>(cap.get(CV_CAP_PROP_FPS)))
		, width(static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_WIDTH)))
		, height(static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_HEIGHT)))*/
	{
		FPS = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FPS));
		width = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_WIDTH));
		height = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
		//Checking if input stream is ok
		if (!cap.isOpened())
			lError(capErrorText, 1);
		//Initializing FPS, width and height with values taken from input video

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
	void readFrom(std::string&& fileFrom)
	{
		cap.release();
		cap.open(std::forward<std::string>(fileFrom));
		if (!cap.isOpened())
			lError(capErrorText, 1);
	}
	void readFrom(unsigned short& camera) override
	{
		cap.release();
		cap.open(camera);
		if (!cap.isOpened())
			lError(capErrorText, 1);
	}
	void writeTo(std::string& fileOutName) override
	{
		wrt.release();
		wrt.open(std::forward<std::string>(fileOutName), -1, FPS, Size(width, height));
	}
	Size getSize()
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
