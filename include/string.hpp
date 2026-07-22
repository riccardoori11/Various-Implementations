#include <iostream>
#include <cstring>
#include <memory>
#include <new>
#include <utility>

/*
 * sizeof(std::string) gives me 32 bytes
 * */

namespace ricc{

class String{

private:
		static constexpr std::size_t MAX{23};

		union{
				char buffer[MAX +1];
				struct {
						char* ptr;
						std::size_t capacity;
				} large;
		
		}data_
		;
		std::size_t size_{};

		bool is_short() const{

				return size_ <= MAX;
		}

		char* data(){

				return (is_short() ? &data_.buffer[0]: data_.large.ptr);
		}


public:

		String(){

				data_.buffer[0] = '\0';
		}

		String(const char* other):size_(strlen(other)){

				if (is_short()){
						std::memcpy(data_.buffer,other,size_+1);
						data_.buffer[size_] = '\0';
				}
				else{
						
						data_.large.capacity = size_;
						data_.large.ptr = static_cast<char*>(::operator new(data_.large.capacity +1));
						std::memcpy(data_.large.ptr,other,size_+1);
				}
		}
		const char* data() const{

				return (is_short() ? &data_.buffer[0]: data_.large.ptr);
		}

		// copy constructor
		String(const String& other):size_(other.size_){

				if (other.is_short()){
						for (std::size_t i{}; i < other.size_; ++i){

								data_.buffer[i] = other[i];
						}
						data_.buffer[other.size_] = '\0';
				}
				else{

						data_.large.capacity = size_;
						data_.large.ptr = static_cast<char*>(::operator new(data_.large.capacity+1));
						for (std::size_t i{}; i < size_; ++i){

								std::construct_at(data_.large.ptr + i,other.data_.large.ptr[i]);
						}
						data_.large.ptr[size_] = '\0';
				}

				std::cout << "COPY CONSTRUCTOR" << std::endl;

		}

/*lets say *this is a short and other is a large  or *this is a large and other is a short */
		// copy assignment
		String& operator= (const String& other){

				if (this != &other){
						if (other.is_short()){

								if (!is_short()){

										::operator delete(data_.large.ptr);
								}
								for (std::size_t i{}; i<= other.size_; i++){

										data_.buffer[i] = other.data_.buffer[i];
								}
								size_ = other.size_;
						}
						else{

								char* new_ptr = static_cast<char*>(::operator new(other.data_.large.capacity+1));
								for (std::size_t i{}; i <= other.size_; ++i){

										std::construct_at(new_ptr + i, other.data_.large.ptr[i]);
								}
								if (!is_short()){

										::operator delete(data_.large.ptr);
								}
								data_.large.ptr = new_ptr;
								data_.large.capacity = other.data_.large.capacity;
								size_ = other.size_;
								
						}
				}



				std::cout << "COPY ASSIGNMENT" << std::endl;
				return *this;
				
		}

		// move constructor
		String (String && other) noexcept: size_(std::move(other.size_)) {

				if (other.is_short()){

						for (std::size_t i{}; i <= size_; ++i){
								data_.buffer[i] = other.data_.buffer[i];
						}
				}
				else{
						data_.large.ptr = std::exchange(other.data_.large.ptr, nullptr);
						data_.large.capacity = std::exchange(other.data_.large.capacity,0);

				}
				other.data_.buffer[0] = '\0';
				other.size_ = 0;
				std::cout << "Move constructor" << std::endl;
		}

		// move assignment
		String& operator = (String && other) noexcept{

				if (this != &other){
						if (!is_short()){

								::operator delete(data_.large.ptr);
								data_.large.capacity = 0;
						}
						size_ = other.size_;
						if (other.is_short()){

								for (std::size_t i{}; i <= other.size_; ++i){

										data_.buffer[i] = other.data_.buffer[i];
								}
								
								other.data_.buffer[0] = '\0';
						}
						else{
								
								data_.large.ptr = other.data_.large.ptr;
								data_.large.capacity = other.data_.large.capacity;
								other.data_.large.ptr = nullptr;
								other.data_.large.capacity = 0;
						}
				}
				other.size_ = 0;
				std::cout << "MOVE ASSIGNMENT" << std::endl;
				return *this;
		};

		
		constexpr std::size_t size(){
				return size_;
		}

		char &operator[](std::size_t index){
				char* loc = data();
				return loc[index];

		}
		const char& operator[](std::size_t index) const{
				const char* loc = data();
				return loc[index];

		}

		void print(){

				std::cout << data() << std::endl;
		}
};


}
