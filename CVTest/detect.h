#pragma once
#include "BaseFilter.h"

class detect : public filter {
public:
	//Mutex detection constructor
	detect() : frame() {}
	//Mutex & capturer constructor
	detect(cv::Mat frame) : frame(frame) {}

protected:
	//Applying filters
	void run(cv::Mat& frm) override
	{
			if (frame.empty())
			{
				lError(std::string("Canny => Original frame seems to be empty..."), 12);
				return;
			}
			frm = frame.clone();
			//Applying filters to the frame
			cvtColor(frm, frm, cv::COLOR_BGR2GRAY);
			cv::Canny(frm, frm, 50, 100);
			cvtColor(frm, frm, cv::COLOR_GRAY2BGR);
			//Outputing the frame into window
			cv::imshow("Canny", frm);
			
	}
	cv::Mat frame;
	threadWrapper thr;
};