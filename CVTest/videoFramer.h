#pragma once
#include "Header.h"
#include "threadWrapper.h"
#include "SrcDestBases.h"

//Class outputing the webcam input to the screen
class videoFramer : public src<cv::Mat> {
public:
	//Mutex constructor
	videoFramer() {}
	//Mutex & capturer constructor
	videoFramer(cv::VideoCapture&& cap) : cap(std::move(cap)) {}
	virtual void start()
	{
		thr(_fn([this] { this->operator()(); }));
	}
	virtual void stop() override
	{
		if (thr.isThreading())
			thr.join();
	}
	bool threadReady()
	{
		return !thr.isThreading();
	}
	cv::Mat getMat()
	{
		return frm;
	}
	virtual void operator()()
	{
		unsigned short FPS = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FPS));
		unsigned short width = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_WIDTH));
		unsigned short height = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
		frm = cv::Mat(cv::Size(width, height), CV_8UC3);
		//cv::VideoWriter wrt("outp.avi", -1, FPS, cv::Size(width, height));
		cv::namedWindow("Original");
		while (true)
		{
			this->run();
			if (cv::waitKey(1) == 27)
				break;
		}
	}
protected:
	void run()
	{
		cap >> frm;
		cv::imshow("Original", frm);
	}
private:
	//capture variable
	cv::VideoCapture cap;
	cv::Mat frm;
	threadWrapper thr;
};
