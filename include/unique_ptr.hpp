#include <cstddef>
#include <new>
#include <iostream>
#include <optional>
#include <utility>

namespace ricc{
/* Rule of five
 *  get
 * release
 * ==
 * overloading function *
 * */

template <typename T>
class unique_ptr{


private:

		T* ptr{nullptr};

public:

		
		unique_ptr() = default;

		~unique_ptr(){

				if (ptr == nullptr){

						return;
				}
				delete ptr;
				ptr = nullptr;
		}

		unique_ptr(T* p): ptr(p)
		{
		}

		unique_ptr(const unique_ptr& other) = delete;

		unique_ptr& operator = (const unique_ptr& other) = delete;

		unique_ptr(unique_ptr&& other){

				std::swap(ptr,other.ptr);
		}

		unique_ptr& operator= (unique_ptr&& other){

				if (this == &other){

						return *this;
				}

				delete ptr;
				ptr = std::exchange(other.ptr, nullptr);

				return *this;
		}

		T& operator*(){

				return (*ptr);
		}

		T* get(){

				return ptr;
		}

		const T* get() const{

				return ptr;
		}

		void swap(unique_ptr& other){

				std::swap(ptr,other.ptr);
		}

				


};

template<typename T1,typename T2>
bool operator == (const unique_ptr<T1>&x , const unique_ptr<T2>& y){

		return x.get() == y.get();
}

template<typename T1>
bool operator == (const unique_ptr<T1>& X, std::nullptr_t r){

		return X.get() == nullptr;

}

}

