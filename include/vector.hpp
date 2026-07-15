#include <iostream>
#include <memory>
#include <new>
#include <utility>

namespace ricc{
struct Tracker{

		static inline int constructed{};
		int val{};
		
		Tracker(int val):val{val}{
				++constructed;
		}

		// copy constructor
		Tracker(const Tracker& other){

				std::cout << "TRACKER COPIED!" << std::endl;
		}
		Tracker(Tracker&& other){

				std::cout << "TRACKER MOVED!" << std::endl;
		}

		Tracker& operator= (const Tracker& other) = delete;
		Tracker& operator = (Tracker&& other) = delete;

};
template <typename T>
class Vector{

private:

T* data_; 
std::size_t size_;
std::size_t capacity_;

public:

constexpr Vector():data_{nullptr}, size_{},capacity_{}
{
}

~Vector(){
		
		for (std::size_t i{}; i < size_; ++i){

				std::destroy_at(data_ + i);
		}
		::operator delete(data_);
}

// copy constructor
constexpr Vector(const Vector& other):size_{}, capacity_{other.capacity_}
{
		
		// raw memory
		data_ = static_cast<T*>(::operator new(sizeof(T) * capacity_));
		for (; size_ < other.size_; ++size_){

				std::construct_at(data_ + size_,other.data_[size_]);
		}
		

}
// copy assignment
constexpr Vector& operator= (const Vector& other){

		T* new_data{static_cast<T*>(::operator new(sizeof(T) * other.capacity_))};
		std::size_t new_size{};
		
		for (; new_size < other.size_; ++new_size){

				std::construct_at(new_data + new_size, other.data_[size_]); }

		// delete the obejcts that live inside
		for (std::size_t i{}; i < size_; ++i){

				std::destroy_at(data_+i);
		}

		::operator delete(data_);
		data_ = new_data;
		size_ = new_size;
		capacity_ = other.capacity_;
		return *this;
}

// move constructor
constexpr Vector(Vector && other) noexcept: 
		data_{std::exchange(other.data_,nullptr)}, size_{std::exchange(other.size_,0)},capacity_{std::exchange(other.capacity_,0)}  
{
}

// move assignment
constexpr Vector& operator=(Vector&& other) noexcept
{
		std::swap(data_,other.data_);
		std::swap(size_,other.size_);
		std::swap(capacity_,other.capacity_);
		return *this;

}

// move the old elements into the new storage
constexpr void reserve(std::size_t new_capacity){

		T* new_data = static_cast<T*>(::operator new(sizeof(T) * new_capacity));


		for (std::size_t i{}; i < size_ ; ++i){
				std::construct_at(new_data + i, std::move(data_[i]));
		}

		for (std::size_t i{}; i < size_; ++i){
				std::destroy_at(data_ + i);
		}

		::operator delete(data_);
		data_ = new_data;
		capacity_ = new_capacity;
}


constexpr void push_back(T value){

		if (size_ == capacity_){

				int new_capacity = (size_ == 0) ? 1: capacity_*2;
				reserve(new_capacity);
		}
		std::construct_at(data_ + size_,value);
		++size_;
}

constexpr std::size_t size() const{

		return size_;
}

constexpr std::size_t capacity() const{

		return capacity_;
}

constexpr T& operator[](const std::size_t i) const{
		return data_[i];
}

constexpr bool empty() const{

		return size_ == 0;
}

};
}
