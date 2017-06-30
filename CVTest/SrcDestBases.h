#pragma once
#include "baseProcessing.h"
//Base source processing class
template<class T>
class src : public processing<T>
{
protected:
	std::function<void(std::string, unsigned short)> m_on_error;
};
//Base destination processing class
template <class T>
class dest : public processing<T>
{
public:
	dest() {}
	~dest() {}
	virtual void stop() = 0;
private:
	std::mutex m_m;
	std::condition_variable m_cv;
	std::unique_ptr<std::thread> m_t;
protected:
	virtual void run() = 0;
};
