#include <initializer_list>
#include <iostream>

namespace ricc{
template<typename T, std::size_t size_>
class Array{

private:

		T data_[size_];

		void swap(Array& other){

				using std::swap;
				swap(data_,other.data_);
		}

public:

		Array() = default;

		~Array() = default;

		std::size_t size() noexcept{

				return size_;

		}

		const std::size_t size() const noexcept{

				return size_;

		}

		Array (const Array& other){
				for (std::size_t i{}; i < other.size(); ++i ){

						data_[i] = other.data_[i];
				}
		}

		Array& operator = (const Array& other){

				Array{other}.swap(*this);
				return *this;

		}

		/*
		 * what happend if they arent the same size
		 * */
		Array (Array&& other) noexcept{

				for (std::size_t i{}; i < other.size(); ++i){

						data_[i] = std::move(other.data_[i]);
				}
		}

		Array& operator = (Array&& other) noexcept{

				Array{std::move(other)}.swap(*this);
				return *this;

		}

		Array(std::initializer_list<T> lst){

				auto* ptr = data_;

				for (auto a: lst){

						 *ptr = a;
						 ptr++;

				}

		}

		T& operator[](std::size_t idx) noexcept{

				return data_[idx];
		}

		const T& operator[](std::size_t idx) const noexcept{

				return data_[idx];
		}


};

};
