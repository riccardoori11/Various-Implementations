#include <iostream>
#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>

	/*
		 * sizeof(std::string) gives me 32 bytes
		 * */

namespace ricc{

class String{

private:
		static constexpr std::size_t MAX{23};
		static constexpr std::size_t gf = 2;

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

		std::size_t get_short_capacity(){

				return MAX;

		}

		std::size_t get_long_capacity(){

				return data_.large.capacity;
		}

		char* data(){

				return (is_short() ? &data_.buffer[0]: data_.large.ptr);
		}

		void resize(std::size_t new_capacity){

				char* new_ptr = static_cast<char*>(::operator new(new_capacity+1));
				for (std::size_t i{}; i <= size_ ; ++i){
						std::construct_at(new_ptr + i, data_.large.ptr[i]);
				}
				::operator delete(data_.large.ptr);
				data_.large.ptr = new_ptr;
				data_.large.capacity = new_capacity;
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

		std::size_t get_capactiy(){

				if (is_short()){

						return get_short_capacity();
						
				}
				return get_long_capacity();
				

		}

		void reserve(std::size_t new_capacity){

				if (new_capacity <= data_.large.capacity){

						throw std::logic_error("new capacity cannot be smaller than current capacity");
				}
				resize(new_capacity);
		}

		void push_back(char c){

				if (is_short()){

						if (size_ == 23){
								std::size_t new_cap = MAX*gf;

								char* new_ptr = static_cast<char*>(::operator new(sizeof(new_cap+1)));
								for (std::size_t i{}; i < size_; ++i){

										std::construct_at(new_ptr +i, data_.buffer[i]);
								}
								std::construct_at(new_ptr + size_, c);
								data_.large.capacity = new_cap;
								data_.large.ptr = new_ptr;
								size_++;
								std::construct_at(new_ptr + size_, '\0');
						}
						else{
								data_.buffer[size_] = c;
								size_++;
						}
				}
				else{

						if (size_ == data_.large.capacity){

								std::size_t new_cap = data_.large.capacity * gf;
								reserve(new_cap);
								std::construct_at(data_.large.ptr + size_,c);
								size_++;
								std::construct_at(data_.large.ptr + size_,'\0');
						}
						else{
								std::construct_at(data_.large.ptr + size_,c);
								size_++;
								std::construct_at(data_.large.ptr + size_,'\0');
						}
				}
				

		}

		void assign(std::size_t size, char c){

				if (size < MAX){

						for (std::size_t i{}; i < size; ++i){

								data_.buffer[i] = c;
						}
						data_.buffer[size] = '\0';
				}
				else{

						for (std::size_t i{}; i < size; ++i){

								std::construct_at(data_.large.ptr + i, c);
						}
						std::construct_at(data_.large.ptr + size, '\0');
				}
				size_ = size;
		}

		void print(){

				std::cout << data() << std::endl;
		}
};


}
