#include <new>
#include <utility>
namespace ricc{

template <typename T>

class vector{

private:


		T* begin{nullptr};
		T* end{nullptr};
		T* capacity{nullptr};

		constexpr static int gf{2};

		void reallocate(std::size_t new_capacity){

				T* new_begin = static_cast<T*>(::operator new(new_capacity * sizeof(T)));
				T* new_end = new_begin;

				for (T* it = begin; it != end; ++it){

						::new (new_end) T(std::move(*it));
						++new_end;
				}

				for(T* it = begin; it != end; ++it){
						it->~T();
				}
				
				

				begin = new_begin;
				end = new_end;
				capacity = new_begin + new_capacity;
				
		}
		
public:

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//                                                                                                MEMBER FUNCTIONS
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
		// constructor
		vector() = default;
		
		// destructor
		~vector(){
				clear();
				::operator delete(begin);
			
		};

		void push_back(const T& U){

				if (end == capacity){
						
						std::size_t new_capacity = (capacity - begin) * gf;
						if (new_capacity == 0){

								new_capacity = 1;
						}
						reallocate(new_capacity);
				}
				::new (end) T(U);
				++end;

		}

		// size
		std::size_t size(){

				return end -begin;
		}

		//clear function
		void clear(){
				// start from end go down and make it null?
				for (T* p = begin; p != end; ++p){
						~T();
				}
				begin = end;
				
		}

};

};
