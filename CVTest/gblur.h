#pragma once
#include "BaseFilter.h"

class gblur : public filter
{
public:
	gblur(std::mutex& mut, cv::VideoCapture& cap) : cap(cap)
	{
		FPS = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FPS));
		width = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_WIDTH));
		height = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
		frame = cv::Mat::Mat(cv::Size(width, height), CV_8UC3);
	}
	void operator()()
	{
		this->run();
	}
	void stop() override {}
protected:
	void run()
	{
		cap.open(0);
		if (!cap.isOpened())
		{
			lError(std::string("GBlur filter failed to acquire resource"), 11);
			return;
		}
		cv::namedWindow("GBlur");
		cv::Mat tmp{ cv::Size(width, height), CV_8UC3 };
		while (true)
		{
			cap >> frame;
			if (frame.empty())
			{
				lError(std::string("GBlur => Original frame seems to be empty..."), 12);
				return;
			}
			tmp = frame.clone();
			cv::GaussianBlur(frame, tmp, cv::Size(25, 25), 0);
			//Copying it back into its place
			//
			//
			//tmp.copyTo(tmp);
			cv::imshow("GBlur", tmp);
			if (cv::waitKey(1) == 27)
				break;
		}
	}
private:
	cv::VideoCapture& cap;
	cv::Mat frame;
	unsigned short FPS;
	unsigned short width;
	unsigned short height;
};