#include <opencv2/opencv.hpp>
#include <iostream>
#include <Windows.h>
#include <functional>
#include <conio.h>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

#define Enter 13
#define Escape 27
const unsigned char menusize{ 3 };

auto lError = [](std::string& what, int code) { std::cout << what << '\n';
	Sleep(2000);
	std::exit(code); };

class menu
{
public:
	menu(std::string arr[], unsigned char size,  std::function<void()> funcs[menusize]) 
		:_size(size)
		, current(0)
	{
		unsigned short i = 0;
		for (;i < menusize; i++)
			_funcs[i] = funcs[i];
		i = 0;
		for (std::string& str = arr[0]; i < _size; i++)
		{
			ptrptr[i] = str;
			str = i < size - 1 ? arr[i + 1] : "";
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

typedef unsigned short unssh;
using namespace cv;

typedef Mat InputArr;
typedef Mat OutputArr;

std::mutex mut;
std::condition_variable cv_;
enum cond_v {
	EXIT,
	bw,
	gblur,
	canny,
	filtersDone,
	NONE
};
cond_v ccc = NONE;

class filter {
public:
	virtual void operator()(InputArr& input, OutputArr& output, Rect toFilter) const = 0;
};

typedef std::unique_lock<std::mutex> lk;
typedef std::lock_guard<std::mutex> lg;

class blur : public filter{
public:
	void operator()(InputArr& input, OutputArr& output, Rect toBlur) const override
	{
		lk lck(mut);
		while (true)
		{
			cv_.wait(lck, [] {return ccc != NONE && ccc != canny && ccc != bw && ccc != filtersDone; });
			if(ccc == gblur)
			{
				Mat tmp{ input(toBlur) };
				GaussianBlur(tmp, tmp, Size(25, 25), 0);
				//Copying it back into its place
				tmp.copyTo(output(toBlur));
				ccc = canny;
				cv_.notify_all();
			}
			else if (ccc == EXIT)
				break;
		}
	}
} blur_;
class detectEdges : public filter {
public:
	void operator()(InputArr& input, OutputArr& output, Rect toDetect) const override
	{
		lk lck(mut);
		while (true) 
		{
			cv_.wait(lck, [] {return ccc != NONE && ccc != bw && ccc != gblur && ccc != filtersDone; });
			if (ccc == canny)
			{
				//Capturing the next part of the frame and applying Canny edge detector to it
				Mat tmp{ input(toDetect) };
				cvtColor(tmp, tmp, COLOR_BGR2GRAY);
				Canny(tmp, tmp, 50, 100);
				cvtColor(tmp, tmp, COLOR_GRAY2BGR);
				//Copying it back onto frame
				tmp.copyTo(output(toDetect));
				ccc = filtersDone;
				cv_.notify_all();
			}
			else if (ccc == EXIT)
				break;
		}
	}
} det;
class BlackWhite : public filter {
public:
	virtual void operator()(InputArr& input, OutputArr& output, Rect toBW) const override
	{
		lk lck(mut);
		while (true) {	//Capturing the part of the frame and applying B/W filter to it
			cv_.wait(lck, [] {return ccc != NONE && ccc != gblur && ccc != canny && ccc != filtersDone; });
			if (ccc == bw)
			{
				Mat tmp{ input(toBW) };
				cvtColor(tmp, tmp, COLOR_BGR2GRAY);
				cvtColor(tmp, tmp, COLOR_GRAY2BGR);
				//Copying it back into its place on the frame
				tmp.copyTo(output(toBW));
				ccc = gblur;
				cv_.notify_all(); 
			}
			else if (ccc == EXIT)
				break;
		}
	}
} BW;

std::string capErrorText{ "Video input not opened! Program closing\n" };
std::string wrtErrorText{ "Unable to open file for writing! Program will be closed\n" };

class CapWriter {
public:
	CapWriter(std::string&& fileOutName, unsigned short code) : cap(code)
		/*, FPS(static_cast<unsigned short>(cap.get(CV_CAP_PROP_FPS)))
		, width(static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_WIDTH)))
		, height(static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_HEIGHT)))*/
	{
		FPS = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FPS));
		width = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_WIDTH));
		height = static_cast<unsigned short>(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
		//Checking if input stream is ok
		if (!cap.isOpened())
			lError(capErrorText, 1);
		//Initializing FPS, width and height with values taken from input video
		
		//Creating and opening an output video file with VideoWriter
		wrt.open(std::forward<std::string>(fileOutName), -1, FPS, Size(width, height));

		//Checking if output stream is ok
		if (!wrt.isOpened())
			lError(wrtErrorText, 2);
	}
	~CapWriter()
	{
		cap.release();
		wrt.release();
	}
	void readfrom(std::string&& fileFrom)
	{
		cap.release();
		cap.open(std::forward<std::string>(fileFrom));
		if (!cap.isOpened())
			lError(capErrorText, 1);
	}
	void readfrom(unsigned short&& camera)
	{
		cap.release();
		cap.open(std::forward<unsigned short>(camera));
		if (!cap.isOpened())
			lError(capErrorText, 1);
	}
	void writeTo(std::string&& fileOutName)
	{
		wrt.release();
		wrt.open(std::forward<std::string>(fileOutName), -1, FPS, Size(width, height));
	}
	Size getSize()
	{
		return Size(width, height);
	}
	void operator>> (Mat& object) 
	{
		cap >> object;
	}
	void operator<< (Mat& object)
	{
		wrt << object;
	}
	
private:
	unsigned short FPS;
	unsigned short width;
	unsigned short height;
	VideoCapture cap;
	VideoWriter wrt;
};


void Rec_View(const std::string& wndName, std::string&& fileOutName)
{
	CapWriter capwrt(std::forward<std::string>(fileOutName), 0);
	//Creating an OpenCV window
	namedWindow(wndName);
	namedWindow("Unedited");
	//Creating a single frame object to load from/to videostreams
	Mat frame(Size(640, 480), CV_8UC3), edited, unedited;
	unsigned short d = 0;
	unsigned short filterW = capwrt.getSize().width / 5;
	std::thread BW(BW, std::ref(frame), std::ref(frame), Rect(d, 0, filterW, frame.rows));
	std::thread bl(blur_, std::ref(frame), std::ref(frame), Rect((d + filterW) % frame.cols, 0, filterW, frame.rows));
	std::thread detect(det, std::ref(frame), std::ref(frame)
		, Rect((d + filterW * 2) % frame.cols, 0, filterW * 2, frame.rows));
	while (true)
	{
		ccc = NONE;
		//Load into frame
		capwrt >> frame;
		lk lck(mut);
		unedited = frame.clone();
		if (frame.empty())
			break;
		cv_.wait(lck, [] {return ccc == NONE; });
		ccc = bw;
		//Checking not to bypass the edges of the screen
		d += d + filterW * 4 + 5 >= frame.cols ? -d + 1 : 5;
		cv_.notify_all();
		cv_.wait(lck, [] {return ccc == filtersDone; });
		//Write the frame to the outwstream
		capwrt << frame;
		//Show frame
		imshow(wndName, frame);
		imshow("Unedited", unedited);
		if (waitKey(1) == Escape)
		{
			ccc = EXIT;
			cv_.notify_all();
			break;
		}
	}
	if (BW.joinable())
		BW.detach();
	if (bl.joinable())
		bl.detach();
	if (detect.joinable())
		detect.detach();
	cv::destroyAllWindows();
}

void viewOrig(const std::string& wndName, std::string&& fileOutName)
{
	CapWriter capwrt(std::forward<std::string>(fileOutName), 0);
	//Creating an OpenCV window
	namedWindow(wndName);
	//Creating a single frame object to load from/to videostreams
	Mat frame(Size(640, 480), CV_8UC3);
	while (true)
	{
		//Load into frame
		capwrt >> frame;
		if (frame.empty())
			break;

		//Write the frame to the outwstream
		capwrt << frame;
		//Show frame
		imshow(wndName, frame);
		if (waitKey(1) == Escape)
			break;
	}
	destroyWindow(wndName);
}


int main()
{
	//std::thread thr();
	auto origBind = std::bind(viewOrig, "Original video", "outOr.avi");
	auto Rec_VBind = std::bind(Rec_View, "Edited frames", "out.avi");
	std::function<void()> arr[] = {
		  origBind
		, Rec_VBind
		, []{ std::exit(0); }
	};
	std::string array[] = { "View original video","View both original and filtered video","Exit the application" };
	menu my(array, menusize, arr);
	my.run();
	return 0;
}