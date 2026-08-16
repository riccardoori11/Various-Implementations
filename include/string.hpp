	/*
	 *
		 * sizeof(std::string) gives me 32 bytes
		 * */

#include <array>
#include <ios>
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

const char* data() const{

		if (is_short()){

				auto& it = getShortString();
				return it.buffer.data();
		}
		return getLongString().ptr;
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


void swap(String& other){

		data_.swap(other.data_);
		std::swap(size_,other.size_);
}

public:

String() = default;

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

String (String&& other):size_(std::exchange(other.size_,0))
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

String& operator = (String&& other){


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

void pushBack(char ch){

		if (is_short()){

				auto& it = getShortString();
				if (size_ < it.MAX){
						it.buffer[size_++] = ch;
						it.buffer[size_] = '\0';
				}
				
				copy_to_heap();
		}
		else{

				auto& long_sso = getLongString();

				*(long_sso.ptr + size_) = ch;
		}

		++size_;

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
