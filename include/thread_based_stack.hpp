#include <cstddef>
#include <exception>
#include <optional>
#include <memory>
#include <mutex>
#include <stack>


namespace ricc{

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

		void push(T new_value){

				std::lock_guard<std::mutex> lock(mtx_);
				data_.push(std::move(new_value));
		}


		bool empty() const {

				std::lock_guard<std::mutex> lock(mtx_);
				return data_.empty();
		}

		std::optional<T> pop(){

				std::lock_guard<std::mutex> lock(mtx_);

				if (data_.empty()){

						return std::nullopt;
				}

				auto res = std::move(data_.top());

				data_.pop();
				return res;

		}

		void pop(T& value){

				std::lock_guard<std::mutex> lock(mtx_);

				if (data_.empty()){
						return;

				}

				value = std::move(data_.top());
				data_.pop();

		}



};


};
