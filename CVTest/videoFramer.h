#pragma once
#include "Header.h"
#include "threadWrapper.h"
#include "SrcDestBases.h"

class videoFramer : public src<cv::Mat> {
public:
	videoFramer(std::mutex& mut) : cap(), lck{ mut, std::defer_lock }
		, callback(nullptr)
	{
		cap.open((0));
	}
	videoFramer(cv::VideoCapture&& cap, std::mutex& mut) : cap(std::move(cap)), callback(nullptr)
		, lck{ mut, std::defer_lock }
	{}
	videoFramer(cv::VideoCapture&& cap, std::function<void(cv::Mat)> callback, std::mutex& mut)
		: callback(std::make_unique<std::function<void(cv::Mat)>>(callback))
		, lck{ mut, std::defer_lock }
		, cap(std::move(cap))
	{}
	virtual void start()
	{
		//thr = std::make_unique<threadWrapper>(&videoFramer::run, *this);
	}
	virtual void stop()
	{

		thr.join();
	}
	bool threadReady()
	{
		return !thr.isThreading();
	}
	//void operator >> (cv::Mat& object) override {}
	virtual void operator()()
	{
		unsigned short FPS = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FPS));
		unsigned short width = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_WIDTH));
		unsigned short height = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_HEIGHT));

		cv::Mat frm(cv::Size(width, height), CV_8UC3);
		cv::Mat cbcked;
		//cv::VideoWriter wrt("outp.avi", -1, FPS, cv::Size(width, height));
		cv::namedWindow("Original");
		if (callback)
			cv::namedWindow("CallbackEdit");
		if (!threadReady())
		{
			lError(std::string("Already running"), 5);
			return;
		}
		//lck.lock();
		while (true)
		{
			m_cv.wait(lck, [] {return /*lst.size() > 0 ? */true/* : false*/; });
			/*if (*lst.begin() != framing)
			continue;*/
			cap >> frm;
			if (callback)
			{

				cbcked = frm.clone();
				callback->operator()(cbcked);
			}
			cv::imshow("Original", frm);
			if (cv::waitKey(1) == 27)
				break;
		}
		//lck.unlock();
	}
private:
	cv::VideoCapture cap;
	std::unique_ptr<std::function<void(cv::Mat)>> callback;
	std::unique_lock<std::mutex> lck;
	std::condition_variable m_cv;
	threadWrapper thr;
};
