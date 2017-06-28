#include "Header.h"
#include <memory>
#include <list>
#include "BW.h"
#include "gblur.h"
#include "detect.h"
#include "videoFramer.h"

std::list<tasks> lst;

int main()
{
	//Mutex and condition_variable
	std::mutex mut;
	std::condition_variable cv_;
	//class which is going to take video frames from camera and output them into a named window
	videoFramer framer(cv::VideoCapture(0), mut);
	//Filters go ass follows: Black/White, Gaussian blur, Canny edge detection
	_BW bw(mut, cv::VideoCapture(0));
	gblur blur(mut, cv::VideoCapture(0));
	detect det(mut, cv::VideoCapture(0));
	//Creating empty unique pointers for future threads
	std::unique_ptr<threadWrapper> thr;
	std::unique_ptr<threadWrapper> bw_;
	std::unique_ptr<threadWrapper> blur_;
	std::unique_ptr<threadWrapper> det_;
	//Array of functions (menu deprecated) to call from
	std::function<void()> funcs[]{ 
		[&framer, &thr] { 
		//Running thread outputing original video
			thr = std::make_unique<threadWrapper>(
				[&framer] {
					framer.operator()(); 
				}); 
		}
	, [&bw, &bw_] {
			//Running thread outputing original video
			bw_ = std::make_unique<threadWrapper>(
				[&bw] {
					bw.operator()(); 
			}); 
		}
	, [&blur, &blur_] { 
			//Running thread outputing original video
			blur_ = std::make_unique<threadWrapper>(
				[&blur] {
					blur.operator()(); 
				}); 
		}
	, [&det, &det_] {
			//Running thread outputing original video
			det_ = std::make_unique<threadWrapper>(
				[&det] {
					det.operator()(); 
				}); 
		} 
	};
	//String array for menu cases
	std::string strs[]{ "Original video","BW video","GBlur video","Canny edge video" };
	//Calling all the callbacks from the menu
	for (auto lnk : funcs)
		lnk();
	//Joining the threads
	thr->join();
	bw_->join();
	blur_->join();
	det_->join();
	//Exiting the function
	return 0;
}
