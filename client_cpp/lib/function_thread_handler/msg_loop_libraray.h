#ifndef __MSG_LOOP_LIB_H_
#define __MSG_LOOP_LIB_H_
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <absl/types/optional.h>
#include <future>

template<class Message>
class thread_msg_lib{
private:
	std::queue<Message> message_queue_;
	std::mutex queue_lck_;

	std::condition_variable cv_;
	std::mutex cv_lck_;

	std::atomic_bool run_thread_{ true };
	std::function<void(Message)> clean_up_;
	std::future<void> thread_op_;
public:
	~thread_msg_lib() {
		stop(clean_up_);
		//todo: it should wait here for thread to stop running
	}
	
	template<typename CleanUp>
	void stop(CleanUp clean_up) {
		clean_up_ = clean_up;
		run_thread_ = false;
		notify();
	}

	template<typename Callback, typename Func >
	void run_(Callback callback, Func run_condition) {
		while (run_condition() && run_thread_) {
			auto m = get_next_message();
			if (!m.has_value()) {
				//acquire event mutex
				std::unique_lock<std::mutex> l{ cv_lck_ };
				m = get_next_message();
				if (!m.has_value()) {
					cv_.wait(l);
					continue;
				}
			}

			callback(*m);
		}
		clear_queue();
	}


	template<typename Callback>
	void run(Callback callback) {
		auto temp = []() {return true; };
		thread_op_ = std::async(
			std::launch::async, 
			&thread_msg_lib<Message>::run_<Callback, decltype(temp)>,
			this, callback, temp);
	}

	void push_message(Message msg) {
		//todo: if clean up started don't allow putting messaging
		put_message(msg);
		notify();
	}

private:



	absl::optional<Message> get_next_message() {
		std::lock_guard<std::mutex> l{ queue_lck_ };
		if (message_queue_.empty())
			return {};
		absl::optional<Message> r = message_queue_.front();
		message_queue_.pop();
		return r;
	}

	void put_message(Message n) {
		std::lock_guard<std::mutex> l{ queue_lck_ };
		message_queue_.push(n);
	}

	void notify() {
		std::lock_guard<std::mutex> l{ cv_lck_ };
		cv_.notify_all();
	}

	void clear_queue() {
		//clean up messaging
		//came out of loop clean the message
		if (clean_up_) {
			for (auto msg = get_next_message();
				msg.has_value();
				msg = get_next_message()) {
				clean_up_(*msg);
			}
		}
		
	}

	
};

void test_msg_loop_lib();
#endif//__MSG_LOOP_LIB_H_