#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace ricc{

template <typename T>

class thread_safe_queue{

private:

		mutable std::mutex mtx_;
		std::queue<T> queue_;
		std::condition_variable cond_;


public:

		thread_safe_queue()
		{
		};

		
		void push(T value){

				std::lock_guard<std::mutex> lock(mtx_);
				queue_.push(std::move(value));
				cond_.notify_one();
		}

		void wait_and_pop(T& value){

				std::unique_lock<std::mutex> lock(mtx_);
				cond_.wait(
						lock,
						[this] {

						return !queue_.empty();
						}
								);
				
				value = std::move(queue_.front());
				queue_.pop();
		}

		std::shared_ptr<T> wait_and_pop(){

				std::unique_lock<std::mutex> lock(mtx_);
				cond_.wait(

						lock,
						[this]{

						return !queue_.empty();
						}
								);
				std::shared_ptr<T> const res(std::make_shared<T>(
								std::move(queue_.front())
								));
				queue_.pop();
				return res;
		}

		bool try_pop(T& value){

				std::lock_guard<std::mutex> lock(mtx_);

				if (queue_.empty()){

						return false;
				}
				value = std::move(queue_.front());
				queue_.pop();
				return true;
		}


		std::shared_ptr<T> try_pop(){

				std::lock_guard<std::mutex> lock(mtx_);
				if (queue_.empty()){
						return std::shared_ptr<T>();
				}
				std::shared_ptr<T> const res( std::make_shared<T>(std::move(queue_.front())));

				queue_.pop();
				return res;
		}

		bool empty() const {

				std::lock_guard<std::mutex> lock(mtx_);
				return queue_.empty();
		}

		T& index(std::size_t idx){
				std::lock_guard<std::mutex> lock(mtx_);
				return queue_[idx];
		}

};


};
