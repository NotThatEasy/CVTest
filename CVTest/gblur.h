#pragma once
#include "BaseFilter.h"

//Gaussian blur filter
class gblur : public filter
{
public:
	//Mutex & capturer constructor
	gblur(std::mutex& mut, cv::VideoCapture& cap) : cap(cap)
	{
		FPS = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FPS));
		width = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_WIDTH));
		height = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
		frame = cv::Mat::Mat(cv::Size(width, height), CV_8UC3);
	}
	//Running gaussian blur filtering
	void operator()()
	{
		this->run();
	}
	//Still not implemented
	void stop() override {}
protected:
	//Applying filters
	void run()
	{
		if (!cap.isOpened())
		{
			cap.open(0);
		}
		//Creating output window
		cv::namedWindow("GBlur");
		cv::Mat tmp{ cv::Size(width, height), CV_8UC3 };
		while (true)
		{
			//Capturing the frame into the frame matrix
			cap >> frame;
			//If not acquired
			if (frame.empty())
			{
				lError(std::string("GBlur => Original frame seems to be empty..."), 12);
				return;
			}
			tmp = frame.clone();
			//Applying gaussian blur
			cv::GaussianBlur(frame, tmp, cv::Size(25, 25), 0);
			//Showing the frame
			cv::imshow("GBlur", tmp);
			//Expecting user to press Escape
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