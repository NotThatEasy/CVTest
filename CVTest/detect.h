#pragma once
#include "BaseFilter.h"

class detect : public filter {
public:
	//detect(std::mutex& mut) : lck(mut) {}
	detect(std::mutex& mut, cv::VideoCapture& cap) : lck(mut), cap(cap)
	{
		FPS = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FPS));
		width = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_WIDTH));
		height = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
		frame = cv::Mat::Mat(cv::Size(width, height), CV_8UC3);
	}
	void stop() override {}
	void operator()()
	{
		run();
	}
protected:
	void run()
	{
		if (!cap.isOpened())
		{
			lError(std::string("Canny filter failed to acquire resource"), 11);
			return;
		}
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
			cvtColor(tmp, tmp, cv::COLOR_BGR2GRAY);
			Canny(tmp, tmp, 50, 100);
			cvtColor(tmp, tmp, cv::COLOR_GRAY2BGR);
			//Copying it back into its place
			//
			//
			//tmp.copyTo(tmp);
			//
			//
			cv::imshow("Canny", tmp);
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