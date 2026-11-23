#pragma once
	/*
	 *
		 * sizeof(std::string) gives me 32 bytes
		 * */
#include <algorithm>
#include <cstring>
#include <array>
#include <ios>
#include <limits>
#include <ostream>
#include <memory>
#include <iostream>
#include <new>
#include <stdexcept>
#include <utility>
#include <variant>

namespace ricc{



class String{

private:

/**/
static constexpr std::size_t gf{2};


struct short_string{

		static constexpr std::size_t MAX{23};
		std::array<char, MAX+1> buffer;
};

struct long_string{

		char* ptr{};
		std::size_t capacity_{};
};

std::variant<short_string,long_string> data_;

[[nodiscard]] bool is_short() const {

		return std::holds_alternative<short_string>(data_);
}

[[nodiscard]] bool is_long() const {

		return std::holds_alternative<long_string>(data_);
}

auto& getShortString() const{

		return std::get<short_string>(data_);
}

auto& getLongString() const{

		return std::get<long_string>(data_);
}

auto& getShortString() {

		return std::get<short_string>(data_);
}

auto& getLongString() {

		return std::get<long_string>(data_);
}


std::size_t size_{};

auto helper_allocate(std::size_t new_capacity){
		return static_cast<char*>(::operator new(new_capacity+1));
}
auto deallocate(char* ptr){
		::operator delete(ptr);
}

void copy_to_heap(){

		auto& small = getShortString();

		const std::size_t new_capacity = small.MAX * gf;

		char* ptr = helper_allocate(new_capacity);

		for (std::size_t i{}; i < size_; ++i){
				std::construct_at(ptr + i,small.buffer[i]);
		}
		std::construct_at(ptr + size_, '\0');
		data_.emplace<long_string>(long_string(ptr,new_capacity));
}


void swap(String& other) noexcept{
		data_.swap(other.data_);
		std::swap(size_,other.size_);
}

public:

static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();
const char* data() const noexcept{

		if (is_short()){

				auto& it = getShortString();
				return it.buffer.data();
		}
		return getLongString().ptr;
}

// Both representations keep a '\0' after the last character, so data() is
// already a valid C string.
const char* c_str() const noexcept{
		return data();
}

std::size_t size() const noexcept{
		return size_;
}

bool empty() const noexcept{
		return size_ == 0;
}

String() = default;

String(const char* c_string, std::size_t count): size_(count){

		if (count <= short_string::MAX){
				auto& small = getShortString();
				std::ranges::copy_n(c_string, count, small.buffer.begin());
				small.buffer[size_] = '\0';
		}
		else{
				auto* ptr = helper_allocate(count);
				std::ranges::copy_n(c_string, count, ptr);
				std::construct_at(ptr + count,'\0');
				data_.emplace<long_string>(ptr,size_);
		}
}

String(const char* c_string):String(c_string,std::strlen(c_string)){
}

String substr(std::size_t pos = 0, std::size_t count = npos) const{
		
		if (pos > size_){
				throw std::out_of_range{"pos has to be smaller tha size"};
		}
		std::size_t length = size_ - pos;
		return String(data() + pos, std::min(length,count));
} 

String (const String& other): size_(other.size_){
		if (other.is_short()){
				auto& small = getShortString();
				auto& other_small = std::get<short_string>(other.data_);
				for (std::size_t i{}; i < size_; ++i){

						small.buffer[i] = other_small.buffer[i];
				}
				small.buffer[size_] = '\0';
		}
		else{
				const auto& other_large = std::get<long_string>(other.data_);
				char* ptr = helper_allocate(other_large.capacity_);
				for (std::size_t i{}; i < size_; ++i){
						std::construct_at(ptr+ i,*(other_large.ptr + i));
				}
				std::construct_at(ptr +  size_, '\0');
				data_.emplace<long_string>(ptr,other_large.capacity_);
		}
} 

String& operator = (const String& other){

		if (this == &other){
				return *this;
		}
		String temp(other);
		swap(temp);
		return *this;

};

String (String&& other) noexcept :size_(std::exchange(other.size_,0))
{

		if (other.is_short()){

				auto& small = getShortString();
				auto& other_small = other.getShortString();
				for (std::size_t i{}; i < size_; ++i){

						std::swap(small.buffer[i],other_small.buffer[i]);
				}

		}
		else{

				auto& getOther_large = other.getLongString();

				char* ptr = std::exchange(getOther_large.ptr,nullptr);
				std::size_t cap = std::exchange(getOther_large.capacity_,0);

				data_.emplace<long_string>(ptr,cap);
		}
}

String& operator = (String&& other) noexcept{

		if (this == &other){
				return *this;
		}
		String temp(std::move(other));
		swap(temp);
		return *this;
}

~String(){
		if (is_long()){
				auto& heap = getLongString();
				deallocate(heap.ptr);
		}
};

char& front(){
		if (is_short()){
				auto& it = getShortString();
				return it.buffer[0];
		}
		else{
				auto& it = getLongString();
				return it.ptr[0];
		}
}

bool operator==(const String& other) const noexcept{

		if (size_ != other.size_){return false;}

		return std::ranges::equal(data(), data() + size_, other.data(), other.data() + other.size_);
}

// appends count characters starting at source used by both operator+=
String& append(const char* source, std::size_t count){

		if (count == 0){
				return *this;
		}

		const std::size_t new_size = size_ + count;
		const std::size_t capacity = is_short() ? short_string::MAX : getLongString().capacity_;

		if (new_size <= capacity){
				char* dest = is_short() ? getShortString().buffer.data() : getLongString().ptr;
				std::ranges::copy_n(source, static_cast<std::ptrdiff_t>(count), dest + size_);
				dest[new_size] = '\0';
		}
		else{
				const std::size_t new_capacity = std::max(new_size, capacity * gf);
				char* ptr = helper_allocate(new_capacity);
				std::ranges::copy_n(data(), static_cast<std::ptrdiff_t>(size_), ptr);
				std::ranges::copy_n(source, static_cast<std::ptrdiff_t>(count), ptr + size_);
				ptr[new_size] = '\0';

				if (is_long()){
						deallocate(getLongString().ptr);
				}
				data_.emplace<long_string>(ptr, new_capacity);
		}

		size_ = new_size;
		return *this;
}

String& operator+=(const String& other){
		return append(other.data(), other.size_);
}

String& operator+=(const char* c_string){
		return append(c_string, std::strlen(c_string));
}

friend String operator+(String lhs, const String& rhs){
		lhs += rhs;
		return lhs;
}

friend String operator+(String lhs, const char* rhs){
		lhs += rhs;
		return lhs;
}

void pushBack(const char& ch){
		if (is_short()){
				auto& it = getShortString();
				if (size_ < it.MAX){
						it.buffer[size_++] = ch;
						it.buffer[size_] = '\0';
						return;
				}
				copy_to_heap();
		}
				auto& long_sso = getLongString();
				if (long_sso.capacity_ == size_){
						/*char has no constructor, so we can simply copy*/
						std::size_t new_cap = std::max<std::size_t>(1, long_sso.capacity_ * gf);
						auto* new_ptr = helper_allocate(new_cap);
						std::copy(long_sso.ptr, long_sso.ptr+ size_, new_ptr);
						deallocate(long_sso.ptr);
						long_sso.ptr = new_ptr;
						long_sso.capacity_ = new_cap;
				}
				*(long_sso.ptr + size_) = ch;
				++size_;
				long_sso.ptr[size_] = '\0';
}
char& operator[](std::size_t idx){

		if (idx >= size_){
				throw std::out_of_range("idx too big");
		}
		if (is_short()){
			auto& it = getShortString();
				return it.buffer[idx];
		}
		else{
				auto& it = getLongString();
				return *(it.ptr + idx);
		}
}

const char& operator[](std::size_t idx) const {

		if (idx >= size_){

				throw std::out_of_range("idx too big");
		}
		if (is_short()){

			auto& it = getShortString();
				return it.buffer[idx];

		}

		else{

				auto& it = getLongString();
				return *(it.ptr + idx);
		}

}
friend std::ostream& operator << (std::ostream& output, const String& string){

		return output.write(string.data(), static_cast<std::streamsize>(string.size_));
}
};

 
}

template<>
struct std::hash<ricc::String>{
		// FNV algorithm
		static constexpr std::size_t offset_basis{14695981039346656037ULL};
		static constexpr std::size_t FNV_Prime{1099511628211};
		std::size_t operator()(const ricc::String& value) const{
				std::size_t hash = offset_basis;
				for(std::size_t i{}; i < value.size(); ++i){
						// could be an unsigned char
						hash ^= static_cast<std::size_t>(*(value.data() + i));
						hash *= FNV_Prime;
				}
				return hash;
		}

};
