#include "Header.h"
#include <memory>
#include <list>
#include "BW.h"
#include "gblur.h"
#include "detect.h"
#include "videoFramer.h"
#include <chrono>

std::list<tasks> lst;

int main()
{
	//class which is going to take video frames from camera and output them into a named window
		videoFramer framer(std::move(cv::VideoCapture(0)));
		framer.start();

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		//Filters go ass follows: Black/White, Gaussian blur, Canny edge detection
		_BW bw(framer.getMat());
		gblur blur(framer.getMat());
		detect det(framer.getMat());
		//Array of functions (menu deprecated) to call from
		std::function<void()> funcs[]{
			[&bw] {
			//Running thread outputing original video
		bw.start();
		}
	, [&blur] {
			//Running thread outputing original video
		blur.start();
		}
	, [&det] {
			//Running thread outputing original video
			det.start();
		}
		};
		//String array for menu cases
		std::string strs[]{ "Original video","BW video","GBlur video","Canny edge video" };
		//Calling all the callbacks from the menu
		for (auto lnk : funcs)
			lnk();
		framer.stop();
		bw.stop();
		blur.stop();
		det.stop();
	//Exiting the function
	return 0;
}
