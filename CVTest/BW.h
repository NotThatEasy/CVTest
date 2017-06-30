#pragma once
#include "BaseFilter.h"

//Black/White filter class
class _BW : public filter {
public:
	//Mutex constructor
	_BW(cv::Mat frame) : frame(frame) {}
	//Copy-constructor
	_BW(_BW& some) : frame(some.frame) {}

protected:
	void run(cv::Mat& frm)
	{
		//Checking if frame is not empty
		if (frame.empty())
		{
			lError(std::string("BW => Original frame seems to be empty..."), 12);
			return;
		}
		frm = frame.clone();
		//Filtering to B/W
		cv::cvtColor(frame, frm, cv::COLOR_BGR2GRAY);
		cv::cvtColor(frm, frm, cv::COLOR_GRAY2BGR);
		//Showing tmp frame
		cv::imshow("BW", frm);
	}
private:
	cv::Mat frame;
	threadWrapper thr;
};