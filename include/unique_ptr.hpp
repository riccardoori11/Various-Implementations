#include <cstddef>
#include <utility>
namespace ricc{

template<typename T>
class unique_ptr{

		private:
				T* ptr;

		public:
				//constructor
				unique_ptr(){unique_ptr(nullptr);};

				// copy constructable removed
				unique_ptr(unique_ptr& p) = delete;

				// copy assignable removed
				unique_ptr& operator = (const unique_ptr&) = delete;	

				//moving constructor
				unique_ptr(unique_ptr && other):ptr{nullptr}
				{
						
						std::swap(ptr,other.ptr);
						
				}

				// moving assignment
				unique_ptr & operator = (unique_ptr && other){
						if(ptr == other){
								return *this;	
						} 
						delete ptr;
						ptr = other.ptr;
						other.ptr = nullptr;
						return *this;
				}

				// parameterized constructor
				unique_ptr(T* t):ptr{t}{};

				T* get(){
						return ptr;
				}	
				T* release(){
						T* temp = ptr;
						ptr = nullptr;
						return temp;
				};

				T& operator*(){
						return *ptr;
				}
				
				void reset(T* other){
						delete ptr;
						std::swap(ptr,other);		
				}
			
};

template<typename T1, typename T2>
bool operator==( ricc::unique_ptr<T1>& x, ricc::unique_ptr<T2>& y ){
		return x.get() == y.get();	
};
template<typename T1>
bool operator==(ricc::unique_ptr<T1>&x, std::nullptr_t p){
		return x.get() == p;
}



}





















