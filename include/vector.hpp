#include <new>
namespace ricc{

template <typename T>

class vector{

private:


		T* begin{nullptr};
		T* end{nullptr};
		T* capacity{nullptr};

		constexpr static int gf{2};

		void reallocate(std::size_t size){

				T* new_begin = static_cast<T*>(::operator new(size * gf));
				T* new_end = new_begin;


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
		void clear(){

				// start from end go down and make it null?
				for (T* p = begin; p != end; ++p){
						~T();
				}
				begin = end;
		}

};

};
