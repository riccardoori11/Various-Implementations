#include <utility>

namespace ricc{


template <typename T>
class unique_ptr{


		private:

				T* ptr{nullptr};

		public:

				// constructor
				constexpr unique_ptr() noexcept = default;

				constexpr ~unique_ptr(){
						delete ptr;
				}

				// copy constructor
				unique_ptr(const unique_ptr& other) = delete;

				// coppy assignment
				unique_ptr& operator= (const unique_ptr& other) = delete;

				// move constructor
				constexpr unique_ptr(unique_ptr && other) noexcept: ptr(std::exchange(other.ptr,nullptr)){
				}

				//move assignment
				// delete other first or else memory leak
				unique_ptr& operator = (unique_ptr&& other){
						if (this != &other){

								delete ptr;
								ptr = std::exchange(other.ptr,nullptr);
						}
						return *this;
				};

				// paremeterized constructor
				unique_ptr(T* ptr): ptr(ptr){};

				
				T* get(){
						return ptr;
				}

				constexpr T* release(){

						st
				}




};
}

