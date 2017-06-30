#pragma once
#include "threadWrapper.h"
#include "BaseFilter.h"

class BaseFactory : public src<filter> {
public:
	std::unique_ptr<filter>&& make_filter(std::function<void(cv::Mat&)>&& lambda)
	{
		return std::make_unique<filter>(std::move(lambda));
	}
};