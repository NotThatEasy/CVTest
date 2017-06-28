#include "Header.h"
#include <memory>
#include <list>
#include "BW.h"
#include "gblur.h"
#include "detect.h"
#include "videoFramer.h"

//auto lError = [](std::string& what, int code) { std::cout << what << '\n';
//Sleep(2000);
//std::exit(code); };





std::list<tasks> lst;




//Base source class






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
