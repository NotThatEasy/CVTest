#pragma once
#include "BaseFilter.h"

class _BW : public filter {
public:
	_BW(std::mutex& mut) : lck(mut) {}
	_BW(_BW& some) : lck(*some.lck.mutex(), std::defer_lock)
		, cap(some.cap)
	{}
	_BW(std::mutex& mut, cv::VideoCapture& cap) : lck(mut, std::defer_lock), cap(cap)
	{
		FPS = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FPS));
		width = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_WIDTH));
		height = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
		frame = cv::Mat::Mat(cv::Size(width, height), CV_8UC3);
	}
	void stop() override {}
	void operator()()
	{
		this->run();
	}
protected:
	void run()
	{
		if (!cap.isOpened())
		{
			lError(std::string("Blackwhite filter failed to acquire resource"), 11);
			return;
		}
		cv::namedWindow("BW");
		cv::Mat tmp{ cv::Size(width, height), CV_8UC3 };
		while (true)
		{
			cap >> frame;
			if (frame.empty())
			{
				lError(std::string("BW => Original frame seems to be empty..."), 12);
				return;
			}
			tmp = frame.clone();
			cvtColor(frame, tmp, cv::COLOR_BGR2GRAY);
			cvtColor(tmp, tmp, cv::COLOR_GRAY2BGR);
			//Copying it back into its place
			//
			//
			//tmp.copyTo(tmp);
			//
			//
			cv::imshow("BW", tmp);
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