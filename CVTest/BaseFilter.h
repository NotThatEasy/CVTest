#pragma once
#include "Header.h"
#include "SrcDestBases.h"
#include "threadWrapper.h"


class filter : public src<cv::Mat> {
public:
	//Filter default constructor
	filter() : thr(nullptr) {}
	//Filter mutex & callback constructor
	filter(std::function<void()>&& lambda) : thr(nullptr), storedFunc(std::make_unique<std::function<void()>>
		(std::forward<std::function<void()>>(lambda)))
	{ }
	virtual void start()
	{
		thr(_fn([this] {this->operator()(); }));
	}
	virtual void stop() override
	{
		if (thr.isThreading())
			thr.join();
	}
	//Running filtering
	virtual void operator()()
	{
		//creating window
		cv::namedWindow("BW");
		//Temporary matrix
		cv::Mat tmp{ frame };
		while (true)
		{
			this->run(tmp);
			//Awaiting for Escape being pressed
			if (cv::waitKey(1) == 27)
				break;
		}
	}
	virtual void run(cv::Mat& frm) 
	{
		if(storedFunc)
			storedFunc->operator()(frm);
	}
	
	//Calling stored func if any available
protected:
	std::unique_ptr<std::function<void(cv::Mat&)>> storedFunc;
	
	cv::Mat frame;
	threadWrapper thr;
	unsigned short FPS;
	unsigned short width;
	unsigned short height;
private:
	
};
