#pragma once
#include <atomic>
#include <cstddef>
#include <cstring>
#include <utility>

namespace ricc{

namespace detail{
struct control_block{

std::atomic<std::size_t> hard_cpt{1};
/*all the shared_ptr hold the weak reference*/
std::atomic<std::size_t> weak_cpt{1};
};
}

template <typename T>
class shared_ptr{
private:

		using pointer = T*;
		using reference = T&;

		T* ptr{nullptr};
		detail::control_block* ctrl_ptr{nullptr};

void release(){
		if (ctrl_ptr){
				if (ctrl_ptr->hard_cpt.fetch_sub(1) == 1){
						delete ptr;
						if (ctrl_ptr->weak_cpt.fetch_sub(1) == 1){
								delete ctrl_ptr;
						}
				}
		}
}

public:
shared_ptr() = default;

shared_ptr(std::nullptr_t) noexcept
{
}

explicit shared_ptr(T* p):ptr(p)
{
		if (p){
				try{
						ctrl_ptr = new detail::control_block;
				}
				catch(...){
						delete p;
						throw;
				}
		}
}

shared_ptr(const shared_ptr& other):ptr(other.ptr), ctrl_ptr(other.ctrl_ptr)
{
		if (ctrl_ptr){
				ctrl_ptr->hard_cpt++;
		}
}

~shared_ptr()
{
		if (ctrl_ptr){
				if (ctrl_ptr->hard_cpt.fetch_sub(1) == 1){
						delete ptr;
						if (ctrl_ptr->weak_cpt.fetch_sub(1) == 1){
								delete ctrl_ptr;
						}
				}
		}

}

void swap(shared_ptr& other) noexcept
{
		using std::swap;
		swap(ptr,other.ptr);
		swap(ctrl_ptr,other.ctrl_ptr);
}

void reset() noexcept
{
		release();
		std::exchange(ptr,nullptr);
		std::exchange(ctrl_ptr,nullptr);
}

void reset(T* p){
		shared_ptr(p).swap(*this);
}

shared_ptr& operator= (const shared_ptr& other){

		shared_ptr temp(other);
		swap(temp);
		return *this;
}

shared_ptr(shared_ptr&& other) noexcept :ptr(std::exchange(other.ptr, nullptr)), ctrl_ptr(std::exchange(other.ctrl_ptr, nullptr))
{
}

shared_ptr& operator= (shared_ptr&& other) noexcept
{
		shared_ptr temp(std::move(other));
		swap(temp);
		return *this;
}

pointer operator->() const noexcept{
		return ptr;
}

reference operator*() const noexcept
{
		return *ptr;
}

T* get() const noexcept
{
		return ptr;
}

std::size_t use_count() const noexcept 
{
		if (ctrl_ptr){
				return ctrl_ptr->hard_cpt.load();
		}
		return 0;
}
explicit operator bool() const noexcept
{
		return ptr ? true : false;
}
};

template<typename T1,typename T2>
bool operator==(const shared_ptr<T1>&x, const shared_ptr<T2>&y)
{
		return x.get() == y.get();
}

template<typename T>
bool operator==(const shared_ptr<T>&x, std::nullptr_t)
{
		return x.get() == nullptr;
}
template<typename T,typename ...Args>
ricc::shared_ptr<T> make_shared(Args&&... args){
		auto* ptr = new T(std::forward<Args>(args)...);
		return shared_ptr(ptr);
}
}



