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

extern std::mutex mut_;
extern std::condition_variable cv_;

auto lError = [](std::string& what, int code) { std::cout << what << '\n';
Sleep(2000);
std::exit(code); };

typedef std::unique_lock<std::mutex> lk;
typedef std::lock_guard<std::mutex> lg;
typedef unsigned short unssh;

#define Enter 13
#define Escape 27
const unsigned char menusize{ 3 };


std::string capErrorText{ "Video input not opened! Program closing\n" };
std::string wrtErrorText{ "Unable to open file for writing! Program will be closed\n" };

