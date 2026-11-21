#pragma once
#include <cstddef>
#include <new>
#include <iostream>
#include <optional>
#include <type_traits>
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

explicit unique_ptr(T* p): ptr(p)
{
}

unique_ptr(const unique_ptr& other) = delete;

unique_ptr& operator = (const unique_ptr& other) = delete;

unique_ptr(unique_ptr&& other) noexcept: ptr(std::exchange(other.ptr, nullptr)){

}

template<typename U> requires std::is_convertible_v<U*, T*>
unique_ptr(unique_ptr<U>&& other) noexcept: ptr(other.release()){

}


unique_ptr& operator= (unique_ptr&& other) noexcept{

		if (this == &other){

				return *this;
		}
		auto* old_ptr = ptr;
		ptr = other.release();
		delete old_ptr;
		return *this;
}

template<typename U> requires std::is_convertible_v<U*, T*>
unique_ptr& operator= (unique_ptr<U>&& other) noexcept{

		auto* old_ptr = ptr;
		ptr = other.release();
		delete old_ptr;

		return *this;
}

[[nodiscard]] T* release() noexcept{

		return std::exchange(ptr, nullptr);
}

void reset(T* pointer = nullptr) noexcept{
		auto* old = ptr;

		ptr = pointer;

		delete old;
}


T* operator->() const noexcept {
		return ptr;
}


T& operator*() const noexcept{

		return (*ptr);
}


T* get() const noexcept{

		return ptr;
}

void swap(unique_ptr& other){

		std::swap(ptr,other.ptr);
}

// compiler could convert to bool silently
explicit operator bool() const noexcept{
		return ptr ? true : false;
}
};

template<typename T1,typename T2>
bool operator == (const unique_ptr<T1>&x , const unique_ptr<T2>& y){

		return x.get() == y.get();
}

template<typename T1>
bool operator == (const unique_ptr<T1>& X, std::nullptr_t ){

		return X.get() == nullptr;

}

template<typename T,typename ... Args>
ricc::unique_ptr<T> make_unique(Args&&... args){
		T* raw_ptr = new T(std::forward<Args>(args)...);
		return unique_ptr(raw_ptr);
}

}

