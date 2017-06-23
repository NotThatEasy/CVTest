#pragma once
#include "Header.h"
using namespace cv;
typedef Mat InputArr;
typedef Mat OutputArr;

class blur : public filter {
public:
	void operator()(InputArr& input, OutputArr& output, Rect toBlur) const override
	{
		lk lck(mut);
		while (true)
		{
			cv_.wait(lck, [] {return ccc != NONE && ccc != canny && ccc != bw && ccc != filtersDone; });
			if (ccc == gblur)
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