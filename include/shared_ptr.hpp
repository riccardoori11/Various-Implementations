#pragma once
#include <atomic>
#include <cstddef>
#include <cstring>
#include <exception>
#include <functional>
#include <type_traits>
#include <utility>

namespace ricc{

struct bad_weak_ptr: std::exception{};

namespace detail{
struct control_block{

std::atomic<std::size_t> hard_cpt{1};
/*all the shared_ptr hold the weak reference*/
std::atomic<std::size_t> weak_cpt{1};


void* object{nullptr};
void (*destroy)(void*){nullptr};

control_block(void* obj, void (*del)(void*)) : object(obj), destroy(del) {}
};

template <typename U>
void delete_as(void* p){
		delete static_cast<U*>(p);
}
}

/*
 * forward declaration
 * */
template <typename T>
class weak_ptr;

template <typename T>
class shared_ptr{
private:

		using pointer = T*;
		using reference = T&;
		T* ptr{nullptr};
		detail::control_block* ctrl_ptr{nullptr};
		template <typename U> friend class shared_ptr;
		template <typename U> friend class weak_ptr;

		shared_ptr(T*p, detail::control_block* ctrl_p):ptr(p),ctrl_ptr(ctrl_p)
		{
		}

void release(){
		if (ctrl_ptr){
				if (ctrl_ptr->hard_cpt.fetch_sub(1) == 1){
						ctrl_ptr->destroy(ctrl_ptr->object);
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

explicit shared_ptr(const weak_ptr<T>& other): shared_ptr(other.lock())
{
		if (!ctrl_ptr)
		{
				throw ricc::bad_weak_ptr{};
		}
}

template <typename U>
requires std::is_convertible_v<U*, T*>
explicit shared_ptr(U* p):ptr(p)
{
		if (p){
				try{
						ctrl_ptr = new detail::control_block(p, &detail::delete_as<U>);
				}
				catch(...){
						delete p;
						throw;
				}
		}
}

shared_ptr(const shared_ptr& other) noexcept :ptr(other.ptr), ctrl_ptr(other.ctrl_ptr)
{
		if (ctrl_ptr){
				ctrl_ptr->hard_cpt++;
		}
}

/* shared_ptr<Derived> -> shared_ptr<Base>*/
template <typename U>
requires std::is_convertible_v<U*, T*>
shared_ptr(const shared_ptr<U>& other) noexcept :ptr(other.ptr), ctrl_ptr(other.ctrl_ptr)
{
		if (ctrl_ptr){
				ctrl_ptr->hard_cpt++;
		}
}

template <typename U>
requires std::is_convertible_v<U*, T*>
shared_ptr(shared_ptr<U>&& other) noexcept :ptr(std::exchange(other.ptr, nullptr)), ctrl_ptr(std::exchange(other.ctrl_ptr, nullptr))
{
}

~shared_ptr()
{
		if (ctrl_ptr){
				if (ctrl_ptr->hard_cpt.fetch_sub(1) == 1){
						ctrl_ptr->destroy(ctrl_ptr->object);
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

template <typename U>
requires std::is_convertible_v<U*, T*>
void reset(U* p){
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
		return shared_ptr<T>(ptr);
}

template<typename T>
class weak_ptr{
private:

		friend class shared_ptr<T>;

		T* ptr{nullptr};
		detail::control_block* ctrl_ptr{nullptr};
public:
		
		weak_ptr() = default;
		
		weak_ptr(const shared_ptr<T>& shared_p):ptr(shared_p.ptr), ctrl_ptr(shared_p.ctrl_ptr){
				if (ctrl_ptr){
						ctrl_ptr->weak_cpt++;
				}
		}

		template <typename U>
		requires std::is_convertible_v<U*, T*>
		weak_ptr(const shared_ptr<U>& shared_p):ptr(shared_p.ptr), ctrl_ptr(shared_p.ctrl_ptr){
				if (ctrl_ptr){
						ctrl_ptr->weak_cpt++;
				}
		}

		~weak_ptr(){
				if (ctrl_ptr){
						if (ctrl_ptr->weak_cpt.fetch_sub(1) == 1){
								delete ctrl_ptr;
						}
				}
		}

		weak_ptr(const weak_ptr& other): ptr(other.ptr), ctrl_ptr(other.ctrl_ptr)
		{
				if (ctrl_ptr){
						ctrl_ptr->weak_cpt++;
				}
		}

		weak_ptr(weak_ptr&& other) noexcept:ptr(std::exchange(other.ptr,nullptr)), ctrl_ptr(std::exchange(other.ctrl_ptr,nullptr))
		{
		}

		void swap(weak_ptr& other) noexcept
		{
				using std::swap;
				swap(ptr,other.ptr);
				swap(ctrl_ptr,other.ctrl_ptr);
		}

		weak_ptr& operator= (const weak_ptr& other)
		{
				weak_ptr temp(other);
				swap(temp);
				return *this;
		}

		weak_ptr& operator= (weak_ptr&& other) noexcept
		{
				weak_ptr temp(std::move(other));
				swap(temp);
				return *this;
		}

		void reset() noexcept{
				weak_ptr().swap(*this);
		}

		/* count owners */
		std::size_t use_count() const noexcept
		{
				if (ctrl_ptr){
						return ctrl_ptr->hard_cpt.load();
				}
				return 0;
		}

		bool expired() const noexcept
		{
				return use_count() == 0; 
		}

		shared_ptr<T> lock() const noexcept
		{
				if (ctrl_ptr){
						std::size_t cpt = ctrl_ptr->hard_cpt.load();
						while (cpt != 0){
								if (ctrl_ptr->hard_cpt.compare_exchange_weak(cpt,cpt+1)){
										return shared_ptr<T>(ptr,ctrl_ptr);
								}
						}
				}
				return shared_ptr<T>();
		}
};
}

template <typename T>
struct std::hash<ricc::shared_ptr<T>>{
		std::size_t operator()(const ricc::shared_ptr<T>& p) const noexcept{
				return std::hash<T*>{}(p.get());
		}
};

