#include "filters.h"
#include <memory>
#include <list>

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
	threadWrapper(std::function<void()>& lambda)
		: m_t(new std::thread(std::function<void()>(lambda)))
	{}
	threadWrapper(std::function<void()>&& lambda)
		: m_t(new std::thread(std::function<void()>
		(std::forward<std::function<void()>>(lambda))))
	{}
	template<class Callable>
	threadWrapper(std::function<void()>* func, Callable object)
		: m_t(new std::thread(args...))
	{}
	void operator=(threadWrapper&& t)
	{
		*m_t = std::move(*t.ret());
	}
	std::unique_ptr<std::thread> ret()
	{

	}
	std::thread& operator*()
	{
		return std::move(*m_t);
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
	std::unique_lock<std::mutex> lck;
	std::condition_variable m_cv;
	threadWrapper thr;
protected:
	std::function<void(std::string, unsigned short)> m_on_error;
};
//Base source class
template<class T>
class src : public processing<T>
{
	//virtual void operator >> (T& obj) = 0;
protected:
	std::function<void(std::string, unsigned short)> m_on_error;
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
	videoFramer(cv::VideoCapture cap, std::function<void(cv::Mat)> callback, std::mutex& mut)
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
		cv::VideoWriter wrt("outp.avi", -1, FPS, cv::Size(width, height));
		cv::namedWindow("Original");
		if (callback)
			cv::namedWindow("CallbackEdit");
		if (!threadReady())
		{
			lError(std::string("Already running"), 5);
			return;
		}
		lck.lock();
		while (true)
		{
			m_cv.wait(lck, [] {return lst.size() > 0 ? true : false; });
			if (*lst.begin() != framing)
				continue;
			cap >> frm;
			if (callback)
			{

				cbcked = frm.clone();
				callback->operator()(cbcked);
			}
			cv::imshow("Original", frm);
		}
		lck.unlock();
	}
private:
	cv::VideoCapture cap;
	std::unique_ptr<std::function<void(cv::Mat)>> callback;
	std::unique_lock<std::mutex> lck;
	std::condition_variable m_cv;
	threadWrapper thr;
};

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
	void start() override {}
	void stop() override {}
	void run()
	{
		if (!cap.isOpened())
		{
			lError(std::string("Blackwhite filter failed to acquire resource"), 11);
			return;
		}
		cv::namedWindow("Original");
		cv::namedWindow("Filtered");
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
			cv::imshow("Original", frame);
			cv::imshow("Edited", tmp);
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
	void start() override {}
	void stop() override {}
	void run()
	{
		if (!cap.isOpened())
		{
			lError(std::string("Blackwhite filter failed to acquire resource"), 11);
			return;
		}
		cv::namedWindow("Original");
		cv::namedWindow("Filtered");
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
			cv::GaussianBlur(frame, tmp, cv::Size(25, 25), 0);
			//Copying it back into its place
			//
			//
			//tmp.copyTo(tmp);
			//
			//
			cv::imshow("Original", frame);
			cv::imshow("Edited", tmp);
		}
	}
private:
	cv::VideoCapture& cap;
	cv::Mat frame;
	unsigned short FPS;
	unsigned short width;
	unsigned short height;
};

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
	void start() override {}
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
			lError(std::string("Blackwhite filter failed to acquire resource"), 11);
			return;
		}
		cv::namedWindow("Original");
		cv::namedWindow("Filtered");
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
			cvtColor(tmp, tmp, cv::COLOR_BGR2GRAY);
			Canny(tmp, tmp, 50, 100);
			cvtColor(tmp, tmp, cv::COLOR_GRAY2BGR);
			//Copying it back into its place
			//
			//
			//tmp.copyTo(tmp);
			//
			//
			cv::imshow("Original", frame);
			cv::imshow("Edited", tmp);
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

int main()
{
	std::mutex mut;
	std::condition_variable cv_;
	std::unique_ptr<cv::VideoCapture> ptrVC{ std::move(std::make_unique<cv::VideoCapture>(0)) };
	std::unique_ptr<cv::VideoCapture> ptr{ std::move(std::make_unique<cv::VideoCapture>(0)) };
	videoFramer framer(cv::VideoCapture(0), mut);
	_BW bw(mut, cv::VideoCapture(0));
	gblur blur(mut, cv::VideoCapture(0));
	detect det(mut, cv::VideoCapture(0));
	std::thread thr([&framer] {framer.operator()(); });
	thr.join();
	return 0;
}
