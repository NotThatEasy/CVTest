#pragma once
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

enum cond_v {
	EXIT,
	bw,
	gblur,
	canny,
	filtersDone,
	NONE
};