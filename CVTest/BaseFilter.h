#pragma once
#include "Header.h"
#include "SrcDestBases.h"
#include "threadWrapper.h"

class filter : public src<cv::Mat> {
public:
	filter() {}
	filter(std::mutex& mut) : thr(nullptr), lck(mut, std::defer_lock) {}
	filter(std::mutex& mut, std::function<void()>&& lambda) : thr(nullptr), lck(mut, std::defer_lock)
		, storedFunc(std::make_unique<std::function<void()>>(std::forward<std::function<void()>>(lambda)))
	{ }
	void operator()(std::function<void()>&& lambda)
	{
		//thr = std::make_unique<threadWrapper>(std::forward<std::function<void()>>(lambda));
	}
	void operator()()
	{
		if (storedFunc == nullptr)
		{
			lError(std::string("No lambda captured"), 15);
			return;
		}
		thr.operator=(std::move(threadWrapper(*storedFunc)));
	}
protected:
	std::unique_lock<std::mutex> lck;
	threadWrapper thr;
	std::unique_ptr<std::function<void()>> storedFunc;
	cv::Mat frame;
	unsigned short FPS;
	unsigned short width;
	unsigned short height;
};
