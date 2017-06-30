#pragma once
#include "BaseFilter.h"

//Gaussian blur filter
class gblur : public filter
{
public:
	//Mutex & capturer constructor
	gblur(cv::Mat frame) : frame(frame) {}
private:
	//Applying filters
	void run(cv::Mat& frm) override
	{
			//If not acquired
			if (frame.empty())
			{
				lError(std::string("GBlur => Original frame seems to be empty..."), 12);
				return;
			}
			frm = frame.clone();
			//Applying gaussian blur
			cv::GaussianBlur(frame, frm, cv::Size(25, 25), 0);
			//Showing the frame
			cv::imshow("GBlur", frm);
			
	}
private:
	threadWrapper thr;
	cv::Mat frame;
};