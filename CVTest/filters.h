#pragma once
#include "Header.h"
using namespace cv;
typedef std::shared_ptr<Mat> InputArr;
typedef std::shared_ptr<Mat> OutputArr;

class filter {
public:
	filter() {};
	virtual void run(InputArr& input, OutputArr& output, Rect& toFilter) = 0;
private:
	virtual void operator()(InputArr& input, OutputArr& output, Rect& toFilter) const = 0;
};

class blur : public filter {
public:
	blur(std::mutex& mut) : mut(mut) {}
	virtual void run(InputArr& input, OutputArr& output, Rect& toBlur) override {
		std::thread thr((*this), input, output, toBlur);
		//if (thr.joinable()) thr.join();
	}
//private:
	void operator()(InputArr& input, OutputArr& output, Rect& toBlur) const override
	{
		lk lck(mut);
		while (true)
		{
			cv_.wait(lck, [] {return ccc != NONE && ccc != canny && ccc != bw && ccc != filtersDone; });
			if (ccc == gblur)
			{
				Mat tmp{ (*input)(toBlur) };
				GaussianBlur(tmp, tmp, Size(25, 25), 0);
				//Copying it back into its place
				tmp.copyTo((*output)(toBlur));
				ccc = canny;
				cv_.notify_all();
			}
			else if (ccc == EXIT)
				break;
		}
	}
	std::mutex& mut;
} blur_(mut_);
class detectEdges : public filter {
public:
	detectEdges(std::mutex& mut) : mut(mut) {}
	virtual void run(InputArr& input, OutputArr& output, Rect& toBlur) override {
		std::thread thr((*this), input, output, toBlur);
		//if (thr.joinable()) thr.join();
	}
//private:
	void operator()(InputArr& input, OutputArr& output, Rect& toDetect) const override
	{
		lk lck(mut);
		while (true)
		{
			cv_.wait(lck, [] {return ccc != NONE && ccc != bw && ccc != gblur && ccc != filtersDone; });
			if (ccc == canny)
			{
				//Capturing the next part of the frame and applying Canny edge detector to it
				Mat tmp{ (*input)(toDetect) };
				cvtColor(tmp, tmp, COLOR_BGR2GRAY);
				Canny(tmp, tmp, 50, 100);
				cvtColor(tmp, tmp, COLOR_GRAY2BGR);
				//Copying it back onto frame
				tmp.copyTo((*output)(toDetect));
				ccc = filtersDone;
				cv_.notify_all();
			}
			else if (ccc == EXIT)
				break;
		}
	}
	std::mutex& mut;
} det(mut_);
class BlackWhite : public filter {
public:
	BlackWhite(std::mutex& mut) : mut(mut) {}
	virtual void run(InputArr& input, OutputArr& output, Rect& toBlur) override {
		std::thread thr((*this), input, output, toBlur);
		//if (thr.joinable()) thr.join();
	}
//private:
	virtual void operator()(InputArr& input, OutputArr& output, Rect& toBW) const override
	{
		lk lck(mut);
		while (true) {	//Capturing the part of the frame and applying B/W filter to it
			cv_.wait(lck, [] {return ccc != NONE && ccc != gblur && ccc != canny && ccc != filtersDone; });
			if (ccc == bw)
			{
				Mat tmp{ (*input)(toBW) };
				cvtColor(tmp, tmp, COLOR_BGR2GRAY);
				cvtColor(tmp, tmp, COLOR_GRAY2BGR);
				//Copying it back into its place on the frame
				tmp.copyTo((*output)(toBW));
				ccc = gblur;
				cv_.notify_all();
			}
			else if (ccc == EXIT)
				break;
		}
	}
	std::mutex& mut;
} BW(mut_);