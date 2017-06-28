#pragma once
#include "baseProcessing.h"
template<class T>
class src : public processing<T>
{
	//virtual void operator >> (T& obj) = 0;
protected:
	std::function<void(std::string, unsigned short)> m_on_error;
};
template <class T>
class dest : public processing<T>
{
public:
	dest() {}
	~dest() {}
	virtual void stop() = 0;
	std::function<void(std::string)> m_on_error;
private:
	std::mutex m_m;
	std::condition_variable m_cv;
	std::unique_ptr<std::thread> m_t;
protected:
	virtual void run() = 0;
};