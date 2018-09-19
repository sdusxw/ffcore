/***************************************************************************
 *
 * Copyright (c) 2014 www.sdboon.com, Inc. All Rights Reserved
 * $Project BoonTrain ,v 1.0 2014/03/08 11:22:00
 * sunxiwei @ Machine Learning Technology Center.
 *
 **************************************************************************/

/**
 * @file concurrent_queue.h
 * @author sxw@sdboon.com)
 * @date 2015/05/14 08:53:00
 * @version $Revision: 1.0 $
 * @brief header of concurrent_queue
 *
 **/
#ifndef CONCURRENT_QUEUE_H
#define CONCURRENT_QUEUE_H

#include <deque>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

template<typename Data>
class concurrent_queue
{
private:
	std::deque<Data> the_queue;
	mutable boost::mutex the_mutex;
	boost::condition_variable the_condition_variable;
public:
	void push(Data const& data)
	{
		boost::mutex::scoped_lock lock(the_mutex);
		the_queue.push_back(data);
		lock.unlock();
		the_condition_variable.notify_one();
	}

	bool empty() const
	{
		boost::mutex::scoped_lock lock(the_mutex);
		return the_queue.empty();
	}

	int get_size() const
	{
		boost::mutex::scoped_lock lock(the_mutex);
		return the_queue.size();
	}

	void clear()
	{
		boost::mutex::scoped_lock lock(the_mutex);
		the_queue.clear();
	}

	bool try_pop(Data& popped_value)
	{
		boost::mutex::scoped_lock lock(the_mutex);
		if (the_queue.empty())
		{
			return false;
		}

		popped_value = the_queue.front();
		the_queue.pop_front();
		return true;
	}

	void wait_and_pop(Data& popped_value)
	{
		boost::mutex::scoped_lock lock(the_mutex);
		while (the_queue.empty())
		{
			the_condition_variable.wait(lock);
		}

		popped_value = the_queue.front();
		the_queue.pop_front();
	}

};
#endif
