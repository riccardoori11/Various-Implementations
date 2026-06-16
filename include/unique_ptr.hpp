#include <memory>
#include <utility>

namespace ricc{

template<typename T>
class unique_ptr{
private:
		T* ptr;
public:
//member functions
//constructor
unique_ptr():ptr{nullptr}{}
// copy constructor delete
unique_ptr(const unique_ptr& u) = delete;

//copy assignment 
unique_ptr&  operator=(const unique_ptr&) = delete;

// moving constructor
unique_ptr(const unique_ptr&& other){
		std::exchange(ptr,other);
}

// moving assignment
unique_ptr& operator= (const unique_ptr&& other){
		
		if (&other == this){
				return (*this);	
		}
		delete ptr;
		ptr = std::exchange(other.ptr, nullptr);
		return (*this);
}

// parameterized pointer
unique_ptr(T* p): ptr(p){}

// get
T* get(){return ptr;}

//swap
void swap(unique_ptr& other){
		std::swap(ptr,other.ptr);
}

// reset
void reset(T& other){
		if (ptr == other){
				return;
		}
		else{
				delete ptr;
				std::swap(ptr,other.ptr);
		}
}

};
}


