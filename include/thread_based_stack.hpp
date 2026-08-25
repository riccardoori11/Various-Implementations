#include <exception>
#include <memory>
#include <mutex>
#include <stack>

namespace ricc{

struct empty_stack: std::exception{

		const char* what() const throw();

};

template <typename T>
class lb_stack{

private:

		std::stack<T> data_{};
		mutable std::mutex mtx_;

public:


		lb_stack() = default;

		lb_stack(const lb_stack& other){

				std::lock_guard<std::mutex> lock(other.mtx_);
				data_ = other.data_;
		}

		lb_stack& operator =(const lb_stack& other) = delete;

		void push(T& new_value){

				std::lock_guard<std::mutex> lock(mtx_);
				data_.push(std::move(new_value));
		}


		bool empty() const {

				std::lock_guard<std::mutex> lock(mtx_);
				return data_.empty();
		}

		std::shared_ptr<T> pop(){

				std::lock_guard<std::mutex> lock(mtx_);

				if (empty()){

						throw empty_stack();
				}

				std::shared_ptr<T> const res{std::make_shared<T>(std::move(data_.top()))};

				data_.pop();
				return res;

		}

		void pop(T& value){

				std::lock_guard<std::mutex> lock(mtx_);

				if (empty()){

						throw empty_stack();
				}

				value = std::move(data_.top());
				data_.pop();

		}



};

};
