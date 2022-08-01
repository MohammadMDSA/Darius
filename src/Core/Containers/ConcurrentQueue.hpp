#pragma once
#include <queue>
#include <optional>
#include <mutex>
#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>

#ifndef D_CONTAINERS
#define D_CONTAINERS Darius::Core::Containers
#endif // !D_CONTAINERS

namespace Darius::Core::Containers
{
	template<typename T>
	class ConcurrentQueue {
		std::queue<T> queue_;
		mutable std::mutex mutex_;

		// Moved out of public interface to prevent races between this
		// and pop().
		bool empty() const {
			return queue_.empty();
		}

	public:
		ConcurrentQueue() = default;
		ConcurrentQueue(const ConcurrentQueue<T>&) = delete;
		ConcurrentQueue& operator=(const ConcurrentQueue<T>&) = delete;

		ConcurrentQueue(ConcurrentQueue<T>&& other) {
			std::lock_guard<std::mutex> lock(mutex_);
			queue_ = std::move(other.queue_);
		}

		virtual ~ConcurrentQueue() { }

		unsigned long Size() const
		{
			std::lock_guard<std::mutex> lock(mutex_);
			return queue_.size();
		}

		std::optional<T> Pop() {
			std::lock_guard<std::mutex> lock(mutex_);
			if (queue_.empty()) {
				return {};
			}
			T tmp = queue_.front();
			queue_.pop();
			return tmp;
		}

		void Pop(T& data) {
			std::lock_guard<std::mutex> lock(mutex_);
			if (queue_.empty()) {
				return;
			}
			data = queue_.front();
			queue_.pop();
		}

		void Push(const T& item) {
			std::lock_guard<std::mutex> lock(mutex_);
			queue_.push(item);
		}

		bool IsEmpty() const
		{
			std::lock_guard<std::mutex> lock(mutex_);
			return queue_.empty();
		}
	};
}