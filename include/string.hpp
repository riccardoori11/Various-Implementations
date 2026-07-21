#include <iostream>
#include <cstring>
#include <new>

/*
 * sizeof(std::string) in my machine gives me 32 bytes
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

		bool is_short(){

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



		
		constexpr std::size_t size(){
				return size_;
		}

		char &operator[](std::size_t index){
				char* loc = data();
				return loc[index];

		}

		void print(){

				std::cout << data() << std::endl;
		}

		
		// copy constructor
	/*
	 *String(const String &other):size){

				if (is_short()){

						std::memcpy(buffer,other,);
				}
		}
	 * */	


};


}
