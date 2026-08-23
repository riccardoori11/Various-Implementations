#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>

namespace ricc{

template <typename T>
class Vector{

public:

		using value_type = T;
		using reference = T&;
		using const_reference = const T&; 
		using pointer = T*;
		using const_pointer = const T*;
		using size_type = std::size_t;


private:

pointer data_{nullptr}; 
size_type size_{};
size_type capacity_{};
constexpr static int growth_factor{4};


struct deleter_helper{

		void operator()(T* ptr){

				return ::operator delete(ptr,std::align_val_t(alignof(T)));
		}
};

using vector_ptr = std::unique_ptr<T,deleter_helper>;

vector_ptr allocate_helper(std::size_t new_capacity){
		auto mem = ::operator new(sizeof(T) * new_capacity);
		return vector_ptr(static_cast<T*>(mem));

}



public:

Vector() = default;

Vector (size_type n, const_reference init):capacity_(n),size_(n)
{

				auto mem = allocate_helper(capacity_);
				std::uninitialized_fill(mem.get(),n,init);
				data_ = mem.release();
		
}

Vector(std::initializer_list<T> lst)
{
		auto mem = allocate_helper(lst.size());

		try{
				for (auto value: lst){

						std::construct_at(mem.get() + size_, value);
						++size_;
				}

		}
		catch(...){
				while (size_ != 0){
						std::destroy_at(mem.get() + size_);
						--size_;
				}
				throw;
		}

		data_ = mem.release();
}


~Vector(){
		
		for (std::size_t i{}; i < size_; ++i){
				std::destroy_at(data_ + i);
		}
}


// copy constructor
constexpr Vector(const Vector& other):capacity_{other.capacity_}
{
		
		auto mem = allocate_helper(capacity_);
		T* destination = mem.get();
try{
		for (; size_< other.size_; ++size_){
				std::construct_at(mem.get() + size_,*(other.data_ + size_));
		}

}
catch(...){

		while (size_ != 0){

				--size_;
				std::destroy_at(mem.get() + size_);
		}

		throw;

}

		data_ = mem.release();
		
}




void swap(Vector& other){

		using std::swap;

		swap(data_,other.data_);
		swap(size_,other.size_);
		swap(capacity_,other.capacity_);
}

// copy assignment
constexpr Vector& operator= (const Vector& other){
		
		Vector {other}.swap(*this);
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

		Vector (std::move(other)).swap(*this);

		return *this;

}

//copies the old elements into the new storage
// using this as temporary helper allocate function
constexpr void reserve(size_type new_capacity){

		if (new_capacity <= capacity_){

				throw std::bad_array_new_length();
		}

		auto mem = allocate_helper(new_capacity);

		auto new_ptr = mem.get();

		for (std::size_t i{}; i < size_; ++i){

				std::construct_at(new_ptr + i,*(data_ + i));
		}

		for (std::size_t i{}; i < size_; ++i){

				std::destroy_at(data_+ i);
		}

		capacity_ = std::exchange(new_capacity,0);

		deleter_helper{}(data_);

		data_ = mem.release();

}


 void push_back(T&& value){

		if (size_ == capacity_){

				std::size_t new_capacity = (size_ == 0) ? 1: capacity_*growth_factor;
				reserve(new_capacity);
		}
		std::construct_at(data_ + size_,std::move(value));
		++size_;
}

 void push_back(const_reference value){

		if (size_ == capacity_){

				std::size_t new_capacity = (size_ == 0) ? 1: capacity_*growth_factor;
				reserve(new_capacity);
		}
		std::construct_at(data_ + size_,value);
		++size_;
}

/*
*applying move semantics, here args represents any number of forward references
template <typename... Args>
void emplace_back(Args&&... args){

		if (size_ == capacity_){

				std::size_t new_capacity = (size_ == 0) ? 1: capacity_ * growth_factor;
				reserve(new_capacity);
		}

		T* destination = data_ + size_;

		std::construct_at(destination, std::forward<Args>(args)...);
		size_++;
}
*/
template <typename... Args>
constexpr reference emplace_back(Args&&... args){

		if (size_ == capacity_){

				std::size_t new_capacity = (size_ == 0) ? 1: capacity_ * growth_factor;
				reserve(new_capacity);
		}

		std::construct_at(data_ + size_, std::forward<Args>(args)...);
		size_++;

		return *(data_+ size_);
}

size_type size() const{

		return size_;
}

size_type capacity() const{

		return capacity_;
}

reference operator[](const std::size_t i) {
		return *(data_ + i);
}

const_reference operator[](const std::size_t i) const {
		return *(data_ + i);
}

bool empty() const{

		return size_ == 0;
}

reference front(){

		return data_[0];
}

const_reference front() const{

		return data_[0];
}

reference back(){

		return data_[size_-1];
}

const_reference back() const{

		return data_[size_-1];
}

constexpr pointer  begin (){

		return data_; 
}

const const_pointer cbegin () const{

		return data_; 
}

constexpr pointer end(){

		return data_ + size_;
}

constexpr const_pointer  cend() const{

		return data_ + size_;
}

constexpr void pop_back(){

		if (this->empty()){
				return;
		}
		std::destroy_at(data_ + size_ - 1);
		--size_;
}

constexpr void shrink_to_fit(){


		if (capacity_ == size_){

				return;
		}

		reserve(size_);


}

constexpr const_reference at(const std::size_t pos) {

		if (pos >= size_){

				throw std::out_of_range("Position not valid");
		}

		return data_[pos];
}

constexpr reference at(std::size_t pos) const{

		if (pos >= size_){

				throw std::out_of_range("Position not valid");
		}

		return data_[pos];
}

friend std::ostream& operator << (std::ostream& output, const Vector& vector){

		output << '[';

		for (std::size_t i{}; i < vector.size_; ++i){
				if (i != 0){

						output << ',';
				}
				output << vector.data_[i];
		}

		output << ']';

		return output;
		
}

};




}
