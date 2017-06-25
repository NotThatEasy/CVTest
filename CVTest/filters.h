#pragma once
#include "Header.h"
using namespace cv;
typedef std::shared_ptr<Mat> InputArr;
typedef std::shared_ptr<Mat> OutputArr;

std::string thrRun{ "Thread already running" };

class filter : src<InputArr>, dest<OutputArr> {
public:
	filter() : input(InputArr()), output(OutputArr()) {}
	filter(InputArr& input, OutputArr& output) : input(input), output(output) {};
	virtual void run() = 0;
	virtual void setRectToCopyTo(Rect& rect) = 0;
	virtual void readFrom(InputArr& obj)
	{
		if (flag)
		{
			lError(thrRun, 3);
			return;
		}
		input = obj;
	}
	virtual void writeTo(OutputArr& obj)
	{
		if (flag)
		{
			lError(thrRun, 3);
			return;
		}
		output = obj;
	}
	virtual void operator()(InputArr& input, OutputArr& output, Rect& toFilter) = 0;
private:
	InputArr& input;
	OutputArr& output;
	bool flag;
};

class _blur : public filter {
public:
	_blur(std::mutex& mut, InputArr& input, OutputArr& output, Rect& toBlur) 
		: mut(mut), flag(false), input(input), output(output), toBlur(toBlur) {}
	virtual void run() override {
		std::thread thr((*this), input, output, toBlur);
		//if (thr.joinable()) thr.join();
	}
	virtual void setRectToCopyTo(Rect& rect)
	{
		if (flag)
		{
			lError(thrRun, 3);
			return;
		}
		toBlur = rect;
	}
	void operator()(InputArr& input, OutputArr& output, Rect& toBlur) override
	{
		lk lck(mut);
		flag = true;
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
private:
	InputArr& input;
	OutputArr& output;
	Rect& toBlur;
	bool flag;
	std::mutex& mut;
};
class detectEdges : public filter {
public:
	detectEdges(std::mutex& mut, InputArr& input, OutputArr& output, Rect& toDetect) 
		: mut(mut), flag(false), input(input), output(output), toDetect(toDetect) {}
	virtual void run() override {
		std::thread thr((*this), input, output, toDetect);
		//if (thr.joinable()) thr.join();
	}
	virtual void setRectToCopyTo(Rect& rect)
	{
		if (flag)
		{
			lError(thrRun, 3);
			return;
		}
	}
	void operator()(InputArr& input, OutputArr& output, Rect& toDetect) override
	{
		lk lck(mut);
		flag = true;
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
private:
	InputArr& input;
	OutputArr& output;
	Rect& toDetect;
	bool flag;
	std::mutex& mut;
};
class BlackWhite : public filter {
public:
	BlackWhite(std::mutex& mut, InputArr& input, OutputArr& output, Rect& toBW) 
		: mut(mut), flag(false), input(input), output(output), toBW(toBW) {}
	virtual void run() override {
		std::thread thr((*this), input, output, toBW);
		//if (thr.joinable()) thr.join();
	}
	virtual void setRectToCopyTo(Rect& rect)
	{
		if (flag)
		{
			lError(thrRun, 3);
			return;
		}
	}
	virtual void operator()(InputArr& input, OutputArr& output, Rect& toBW) override
	{
		lk lck(mut);
		flag = true;
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
private:
	InputArr& input;
	OutputArr& output;
	Rect& toBW;
	bool flag;
	std::mutex& mut;
};