#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

namespace container {
	template <typename T>
	class BlockingQueue {
	public:
		BlockingQueue(void) {
		}

		virtual ~BlockingQueue(void) {
		}

		void put(const T& item) {
			std::unique_lock<std::mutex> lock(_mtx);
			_que.push(item);
			lock.unlock();
			_cond.notify_one();
		}

		void get(T* item) {
			std::unique_lock<std::mutex> lock(_mtx);
			while (_que.empty()) {
				_cond.wait(lock);
			}

			*item = _que.front();
			_que.pop();
		}

		bool del(void) {
			std::unique_lock<std::mutex> lock(_mtx);
			if (_que.empty()) {
				return false;
			}

			_que.pop();
			return true;
		}

		bool peek(T* item) {
			std::unique_lock<std::mutex> lock(_mtx);
			if (_que.empty()) {
				return false;
			}

			*item = _que.front();
			return true;
		}

		bool peekBack(T* item) {
			std::unique_lock<std::mutex> lock(_mtx);
			if (_que.empty()) {
				return false;
			}

			*item = _que.back();
			return true;
		}

		bool empty(void) {
			std::unique_lock<std::mutex> lock(_mtx);
			return _que.empty();
		}

		unsigned int size(void) {
			std::unique_lock<std::mutex> lock(_mtx);
			return _que.size();
		}

	private:
		std::queue<T> _que;
		std::mutex _mtx;
		std::condition_variable _cond;
	};
}
