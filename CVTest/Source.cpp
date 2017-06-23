#include "filters.h"
#include "serialization.h"
#include "CapWriter.h"




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


using namespace cv;

std::mutex mut;
std::condition_variable cv_;

cond_v ccc = NONE;


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
	lk lck(mut);
	while (true)
	{
		ccc = NONE;
		//Load into frame
		capwrt >> frame;
		cv_.wait(lck, [] {return ccc == NONE; });
		ccc = bw;
		cv_.notify_all();
		unedited = frame.clone();
		if (frame.empty())
			break;
		
		//Checking not to bypass the edges of the screen
		d += d + filterW * 4 + 5 >= frame.cols ? -d + 1 : 5;
		
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