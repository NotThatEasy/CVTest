#pragma once
#include "BaseFilter.h"

class detect : public filter {
public:
	//Mutex detection constructor
	detect(std::mutex& mut) : lck(mut) {}
	//Mutex & capturer constructor
	detect(std::mutex& mut, cv::VideoCapture& cap) : lck(mut), cap(cap)
	{
		FPS = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FPS));
		width = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_WIDTH));
		height = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
		frame = cv::Mat::Mat(cv::Size(width, height), CV_8UC3);
	}
	//Stopper not implemented
	void stop() override {}
	//Run the filtering
	void operator()()
	{
		run();
	}
protected:
	//Applying filters
	void run()
	{
		if (!cap.isOpened())
		{
			lError(std::string("Canny filter failed to acquire resource"), 11);
			return;
		}
		//creating named window for Canny output
		cv::namedWindow("Canny");
		cv::Mat tmp{ cv::Size(width, height), CV_8UC3 };
		while (true)
		{
			cap >> frame;
			if (frame.empty())
			{
				lError(std::string("Canny => Original frame seems to be empty..."), 12);
				return;
			}
			tmp = frame.clone();
			//Applying filters to the frame
			cvtColor(tmp, tmp, cv::COLOR_BGR2GRAY);
			Canny(tmp, tmp, 50, 100);
			cvtColor(tmp, tmp, cv::COLOR_GRAY2BGR);
			//Outputing the frame into window
			cv::imshow("Canny", tmp);
			//Awaiting for Escape key to be pressed
			if (cv::waitKey(1) == 27)
				break;
		}
	}
protected:
	std::unique_lock<std::mutex> lck;
	threadWrapper thr;
	std::unique_ptr<std::function<void()>> storedFunc;
	cv::VideoCapture cap;
	cv::Mat frame;
	unsigned short FPS;
	unsigned short width;
	unsigned short height;
};