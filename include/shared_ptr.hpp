#include <atomic>
#include <utility>

namespace ricc{

template <typename T>
class shared_ptr{

		T* ptr{nullptr};
		std::atomic<std::size_t > *ctr{nullptr};


public:

		shared_ptr() = default;

		shared_ptr(T* ptr): ptr(ptr){

				if (ptr) try{

						ctr = new std::atomic<std::size_t>{1};
				}
				
				catch(...){

						delete ptr;
						throw;
				}
		}

		shared_ptr(const shared_ptr& other):ptr(other.ptr),ctr(other.ctr){

				if (ctr){
						++(*ctr);
				}
				
		}

		shared_ptr(shared_ptr&& other) noexcept :ptr(std::exchange(other.ptr,nullptr)),ctr(std::exchange(other.ctr, nullptr))
		{
		}

		bool empty() const noexcept{

				return !ptr;
		}

		operator bool() const noexcept{

				return !empty();
		}

		void swap(shared_ptr& temp) noexcept{

				using std::swap;
				swap(ptr,temp.ptr);
				swap(ctr,temp.ctr);
		}


		shared_ptr& operator = (const shared_ptr& other){

				shared_ptr{other}.swap(*this);
				return *this;
		}

		shared_ptr& operator = (shared_ptr&& other) noexcept {

				shared_ptr{std::move(other)}.swap(*this);
				return *this;

		}

		~shared_ptr(){

				if (ctr){
				auto expected = ctr->load();
				auto desired = expected -1;

				while (ctr->compare_exchange_weak(expected,desired)){
						desired = expected -1;
				}

				if (desired == 0){

						delete ptr ;
						delete ctr;
				}
		}
		}

		bool operator ==(const shared_ptr& other) const noexcept{

				return ptr == other.ptr;
		}

		bool operator !=(const shared_ptr& other) const noexcept{

				return ptr != other.ptr;
		}

		T* get() noexcept {

				return ptr;
		}

		const T* get() const noexcept{

				return ptr;
		}

		T& operator*() noexcept{

				return *ptr;
		}

		const T& operator*() const noexcept{

				return *ptr;
		}

		T* operator ->() noexcept{

				return ptr;
		}
		const T* operator ->() const noexcept{

				return ptr;
		}


};



}



