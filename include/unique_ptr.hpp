#include <new>
#include <iostream>
#include <utility>

namespace ricc{


template <typename T>
class unique_ptr{


		private:

				T* ptr{nullptr};

		public:

				unique_ptr() = default;

				unique_ptr(T* p): ptr(p)
				{
				}

				// copy constructor
				unique_ptr(const unique_ptr& other) = delete;

				// copy assignment
				unique_ptr& operator = (unique_ptr& other) = delete;

				// move constructor
				unique_ptr(unique_ptr&& other) {
						std::cout << "Move constructor" << std::endl;
						std::swap(ptr,other.ptr);
				}

				// move assignment
				unique_ptr & operator = (unique_ptr&& other) noexcept {
						std::cout << "Move assignment" << std::endl;
						if (this == &other){

								return (*this);
						}

						/*free ptr */

						if (ptr != nullptr){

								delete ptr;
						}

						ptr = std::exchange(other.ptr,nullptr);
						return *this;
				}

				T* release() noexcept {
						return std::exchange(ptr,nullptr);
				}

				T& operator*(){

						return *ptr;
				}

				T* get(){
						return ptr;
				}

				
				~unique_ptr(){

						if (ptr != nullptr){
								delete ptr;
						}
						T* ptr = nullptr;
						return;					

				}

				

};
}

