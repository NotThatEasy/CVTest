#include <opencv2/opencv.hpp>
#include <iostream>
#include <Windows.h>
#include <functional>
#include <conio.h>
#include <string>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <list>
#include <thread>

auto lError = [](std::string& what, int code) { std::cout << what << '\n';
Sleep(2000);
std::exit(code); };




enum tasks {
	_EXIT,
	BW,
	blur,
	canny,
	framing
};

std::list<tasks> lst;


/*Class wrapping the threads, destructor calls join for the threads
	in case of them still running*/
class threadWrapper {
public:
	//Default constructor, pointing m_t to nullptr
	threadWrapper() : m_t(nullptr) {}
	//Constructor which takes std::thread&& as an arg, running thread instantly
	threadWrapper(std::function<void()>&& lambda)
		: m_t(new std::thread(std::forward<std::function<void()>>(lambda)))
	{
		
	}
	//Callable object, operator() triggers thread running
	void operator()(std::function<void()>& lambda)
	{
		*m_t = std::thread(lambda);
	}
	void join()
	{
		if (!m_t->joinable())
			lError(std::string("Unable to join!\n"), 6);
		m_t->join();
	}
	//Checks if thread is already running
	bool isThreading()
	{
		return m_t == nullptr ? false : true;
	}
	//Destructor of the thread wrapper
	~threadWrapper()
	{
		if (m_t->joinable())
			m_t->join();
	}
protected:
	//Smart pointer, storing thread
	std::unique_ptr<std::thread> m_t;
};
//Base processing class
template<class T>
class processing {
public:
	processing() {}
	~processing() {}
	
	virtual void start() = 0;
	virtual void stop() = 0;
	
	

protected:
	std::mutex m_m;
	std::condition_variable m_cv;
	threadWrapper thr;
protected:
	std::function<void(std::string, unsigned short)> m_on_error;
	virtual void run() = 0;
};
//Base source class
template<class T>
class src : public processing<T>
{
	virtual void operator >> (T& obj) = 0;
protected:
	std::function<void(std::string, unsigned short)> m_on_error;
};

class videoFramer : public src<cv::Mat>{
	videoFramer() : cap()
		, callback(nullptr)
	{
		cap->open((0));
	}
	videoFramer(std::unique_ptr<cv::VideoCapture> cap) : callback(nullptr)
	{
		this->cap = std::move(cap);
	}
	videoFramer(std::unique_ptr<cv::VideoCapture> cap, std::function<void()> callback)
		: callback(std::make_unique<std::function<void()>>(callback))
		, m_on_error(lError)
	{
		this->cap.swap(std::move(cap));
	}
	virtual void start()
	{
		//thr(std::thread(&videoFramer::run, *this));
	}
	virtual void stop()
	{



		thr.join();
	}
protected:
	bool threadReady()
	{
		return !thr.isThreading();
	}
	virtual void run()
	{
		unsigned short FPS = static_cast<unsigned short>(cap->get(CV_CAP_PROP_FPS));
		unsigned short width = static_cast<unsigned short>(cap->get(CV_CAP_PROP_FRAME_WIDTH));
		unsigned short height = static_cast<unsigned short>(cap->get(CV_CAP_PROP_FRAME_HEIGHT));
		std::unique_lock<std::mutex> lck(m_m);
		cv::Mat frm(cv::Size(width, height), CV_8UC3);
		cv::VideoWriter wrt("outp.avi", -1, FPS, cv::Size(width, height));
		cv::namedWindow("Original");
		if (!threadReady())
		{
			lError(std::string("Already running"), 5);
			return;
		}
		while (true)
		{
			m_cv.wait(lck, [] {return lst.size() > 0 ? true : false; });
			if (*lst.begin() != framing)
				continue;
			*cap >> frm;
			cv::imshow("Original", frm);
		}
	}
private:
	std::function<void(std::string, unsigned short)> m_on_error;
	std::unique_ptr<cv::VideoCapture> cap;
	std::unique_ptr<std::function<void()>> callback;
	std::mutex m_m;
	std::condition_variable m_cv;
	threadWrapper thr;
};
template <class T>
class dest : public processing<T>
{
public:
	dest() {}
	~dest() {}
	virtual void start() = 0;
	virtual void stop() = 0;
	std::function<void(std::string)> m_on_error;
private:
	std::mutex m_m;
	std::condition_variable m_cv;
	std::unique_ptr<std::thread> m_t;
protected:
	virtual void run() = 0;
};


class filter : public src<cv::Mat> {
	filter() : m_m(), m_cv(), m_t(nullptr) {}
	filter(std::function<void()>&& lambda) :m_m(), m_cv()
	{
		m_t = std::make_unique<threadWrapper>(new threadWrapper(
			std::forward<std::function<void()>>(lambda)));
	}
	void operator()(std::function<void()>&& lambda)
	{
		m_t = std::make_unique<threadWrapper>(new threadWrapper(
			std::forward<std::function<void()>>(lambda)));
	}
protected:
	std::mutex m_m;
	std::condition_variable m_cv;
	std::unique_ptr<threadWrapper> m_t;
};

class BW : public filter
{

};

std::unique_ptr<filter> ftr;


int _main()
{
	return 0;
}


