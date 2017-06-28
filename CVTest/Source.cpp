#include "Header.h"
#include <memory>
#include <list>

//auto lError = [](std::string& what, int code) { std::cout << what << '\n';
//Sleep(2000);
//std::exit(code); };


class menu
{
public:
	menu(std::list<std::string> arr, std::function<void()> funcs[menusize])
		: current(0), _size(arr.size())
	{
		unsigned short i = 0;
		for (; i < menusize; i++)
			_funcs[i] = funcs[i];
		i = 0;
		for (auto&& lnk : arr)
		{
			ptrptr[i++] = lnk;
		}
	}
	int run()
	{
		do
			switch ([this]()->int {
				SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ 0, 0 });
				for (size_t i = 0; i < current; i++)
					std::cout << ptrptr[i].c_str() << '\n';
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN
					| FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE);
				std::cout << ptrptr[current].c_str();
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN
					| FOREGROUND_RED | FOREGROUND_INTENSITY);
				std::cout << '\n';
				for (size_t i = current + 1; i < _size; i++)
					std::cout << ptrptr[i].c_str() << '\n';
				return _getch(); }())
			{
			case Enter:
				_funcs[current]();
				break;
			case 72:
				current = --current < 0 ? _size - 1 : current;
				break;
			case 80:
				current = ++current >= _size ? 0 : current;
				break;
			case Escape:
				menurunning = false;
				break;
			}
				while (menurunning);
				return 0;
	}
protected:
	bool menurunning;
	short current, _size;
	std::string ptrptr[menusize];
	std::function<void()> _funcs[menusize];
};

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
	std::thread&& operator*()
	{
		return std::move(*m_t);
	}
	//Callable object, operator() triggers thread running
	void operator()(std::function<void()>& lambda)
	{
		m_t = std::unique_ptr<std::thread>(new std::thread(lambda));
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
		m_t.release();
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

int main()
{
	std::mutex mut;
	std::condition_variable cv_;
	videoFramer framer(cv::VideoCapture(0), mut);
	_BW bw(mut, cv::VideoCapture(0));
	gblur blur(mut, cv::VideoCapture(0));
	detect det(mut, cv::VideoCapture(0));
	std::unique_ptr<threadWrapper> thr;//([&framer] {framer.operator()(); });
	std::unique_ptr<threadWrapper> bw_;
	std::unique_ptr<threadWrapper> blur_;
	std::unique_ptr<threadWrapper> det_;
	std::function<void()> funcs[]{ 
		[&framer, &thr] { 
			thr = std::make_unique<threadWrapper>(
				[&framer] {
					framer.operator()(); 
				}); 
		}
	, [&bw, &bw_] {
			bw_ = std::make_unique<threadWrapper>(
				[&bw] {
					bw.operator()(); 
			}); 
		}
	, [&blur, &blur_] { 
			blur_ = std::make_unique<threadWrapper>(
				[&blur] {
					blur.operator()(); 
				}); 
		}
	, [&det, &det_] {
			det_ = std::make_unique<threadWrapper>(
				[&det] {
					det.operator()(); 
				}); 
		} 
	};
	std::string strs[]{ "Original video","BW video","GBlur video","Canny edge video" };
	for (auto lnk : funcs)
		lnk();
	/*menu my(std::list<std::string>(&strs[0], &strs[4]), funcs);	
	my.run();*/
	thr->join();
	bw_->join();
	blur_->join();
	det_->join();
	return 0;
}
