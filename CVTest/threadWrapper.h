#pragma once
#include "Header.h"

/*Class wrapping the threads, destructor calls join for the threads
in case of them still running*/
class threadWrapper {
public:
	//Default constructor, pointing m_t to nullptr
	threadWrapper() : m_t(nullptr) {}
	//Constructor which takes std::thread&& as an arg, running thread instantly
	threadWrapper(std::function<void()>& lambda)
		: m_t(new std::thread(std::function<void()>(lambda)))
	{}
	threadWrapper(std::function<void()>&& lambda)
		: m_t(new std::thread(std::function<void()>
		(std::forward<std::function<void()>>(lambda))))
	{}
	template<class Callable>
	threadWrapper(std::function<void()>* func, Callable object)
		: m_t(new std::thread(func, object))
	{}
	void operator=(threadWrapper&& t)
	{
		*m_t = std::move(*t.ret());
	}
	std::unique_ptr<std::thread> ret()
	{
		return nullptr;
	}
	std::thread&& operator*()
	{
		return std::move(*m_t);
	}
	//Callable object, operator() triggers thread running
	void operator()(std::function<void()>& lambda)
	{
		m_t = std::unique_ptr<std::thread>(new std::thread(lambda));
	}
	void join()
	{
		if (!m_t->joinable())
			lError(std::string("Unable to join!\n"), 6);
		m_t->join();
	}
	//Checks if thread is already running
	bool isThreading()
	{
		return m_t == nullptr ? false : true;
	}
	//Destructor of the thread wrapper
	~threadWrapper()
	{
		m_t.release();
	}
protected:
	//Smart pointer, storing thread
	std::unique_ptr<std::thread> m_t;
};