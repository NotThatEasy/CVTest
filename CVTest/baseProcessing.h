#pragma once
//Base processing class
template<class T>
class processing {
public:
	processing() {}
	~processing() {}

	virtual void stop() = 0;



protected:
	std::unique_lock<std::mutex> lck;
	std::condition_variable m_cv;
	threadWrapper thr;
protected:
	std::function<void(std::string, unsigned short)> m_on_error;
};