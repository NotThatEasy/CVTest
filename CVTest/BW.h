#pragma once
#include "BaseFilter.h"

//Black/White filter class
class _BW : public filter {
public:
	//Mutex constructor
	_BW(std::mutex& mut) : lck(mut) {}
	//Copy-constructor
	_BW(_BW& some) : lck(*some.lck.mutex(), std::defer_lock)
		, cap(some.cap)
	{}
	//Mutex & capturer constructor
	_BW(std::mutex& mut, cv::VideoCapture& cap) : lck(mut, std::defer_lock), cap(cap)
	{
		FPS = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FPS));
		width = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_WIDTH));
		height = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
		frame = cv::Mat::Mat(cv::Size(width, height), CV_8UC3);
	}
	//Stopper(no realizating)
	void stop() override {}
	//Running filtering
	void operator()()
	{
		this->run();
	}
protected:
	void run()
	{
		//If capturer is not opened
		if (!cap.isOpened())
		{
			lError(std::string("Blackwhite filter failed to acquire resource"), 11);
			return;
		}
		//creating window
		cv::namedWindow("BW");
		//Temporary matrix
		cv::Mat tmp{ cv::Size(width, height), CV_8UC3 };
		while (true)
		{
			//Pushing the current video frame into tmp frame
			cap >> frame;
			//Checking if frame is not empty
			if (frame.empty())
			{
				lError(std::string("BW => Original frame seems to be empty..."), 12);
				return;
			}
			tmp = frame.clone();
			//Filtering to B/W
			cvtColor(frame, tmp, cv::COLOR_BGR2GRAY);
			cvtColor(tmp, tmp, cv::COLOR_GRAY2BGR);
			//Showing tmp frame
			cv::imshow("BW", tmp);
			//Awaiting for Escape being pressed
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