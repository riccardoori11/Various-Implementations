#include <iostream>
#include <cstring>
#include <memory>
#include <new>

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
		const char* data() const{

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

		// copy assignment
		String& operator= (const String& other){
				if (this != &other){
						if (other.is_short()){
								for (; size_ < other.size_ ; ++size_){

										data_.buffer[size_] = other[size_];
								}
								data_.buffer[size_] = '\0';
						}
						else{

								data_.large.ptr = static_cast<char*>(::operator new(data_.large.capacity +1));
								for (; size_ < other.size_; ++size_){

										std::construct_at(data_.large.ptr + size_, other.data_.buffer[size_]);
								}
								data_.large.ptr[size_] = '\0';
						}

				}
				std::cout << "COPY ASSIGNMENT" << std::endl;
				return *this;
				
		}

		// move constructor
		String (String && other) noexcept: size_(std::move(other.size_)) {

				if (other.is_short()){

						std::swap(data_.buffer,other.data_.buffer);
				}
				else{

						std::swap(data_.large.ptr,other.data_.large.ptr);
				}
				std::cout << "Move constructor" << std::endl;
		}

		// move assignment
		String& operator = (String && other) noexcept{

				if (this != &other){
						size_ = other.size_;
						if (other.is_short()){
								std::swap(data_.buffer, other.data_.buffer);
						}
						else{
								std::swap(data_.large.ptr,other.data_.large.ptr);
						}
				}

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
