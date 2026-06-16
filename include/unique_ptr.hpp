#include <cstddef>
#include <type_traits>
#include <utility>

namespace ricc{
		
		template<typename T>
		class unique_ptr{
				private:
						T* ptr;
				public:
						
						// constructor
						unique_ptr():ptr(nullptr){}
						// constructor by assignment
						unique_ptr &operator= (unique_ptr &other)  = delete;	
						
						// constructor by copy
						unique_ptr(unique_ptr &other) = delete;
						
						// construcotr with parameter
						unique_ptr(T* p): ptr(p){}

						//operator bool (checks whether or not it owns an object)
						operator bool(){
								return ptr != nullptr;
						}
						
						// access unerlying ptr
						T* get(){
								return ptr;
						}
						// reset ptr
						void reset(T* other){
								if (other != nullptr){
										delete_underlying_ptr();
										std::exchange(ptr,other);
								}	
						}

						//helper function	
						void delete_underlying_ptr(){
								if (ptr != nullptr){
										delete ptr;
										ptr = nullptr;
								}
						}
						
						T& operator*(){return (*ptr);}	
		};	
		
						
}
//non member function
template<typename T1,typename T2>
bool operator==(ricc::unique_ptr<T1>& x, ricc::unique_ptr<T2>& y){
		return x.get() == y.get();
}
template<typename T1>
bool operator==(ricc::unique_ptr<T1>& x,std::nullptr_t y){
		return x.get() == nullptr;
}


