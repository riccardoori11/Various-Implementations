#include <type_traits>
#include <iostream>
#include <cstddef>
#include <memory>
#include <array>
#include <typeinfo>

namespace ricc{

class any{

private:
template <typename T>
static constexpr bool isSmall = std::is_same_v<T,int>; 

using run_time_destructor = std::unique_ptr<void,void (*)(void*)>;

union {

		std::array<std::byte, sizeof(void*)* 4> inline_buffer{};

		void* heap_ptr;

}storage;

const std::type_info* type_{nullptr};

run_time_destructor data_{

		nullptr,
		[](void*){} 
};

public:

any() noexcept = delete;

any(const any& other) = delete;
any& operator= (const any& other) = delete;

any(any&& other) = delete;
any& operator = (any&& other) = delete;


template<typename T>
any(T object){

		using t = std::decay_t<T>;

		type_ = &typeid(t);
		if constexpr (isSmall<t>) {

				t* ptr = std::construct_at(reinterpret_cast<t*>(storage.inline_buffer.data()),std::move(object));

		auto destroy_inline = [](void* ptr){

				std::destroy_at(static_cast<t*>(ptr));
		};
		data_ = run_time_destructor(

						static_cast<void*>(ptr),
						destroy_inline
						); 

		
		}
		else{

				t* ptr = new t(std::move(object));

				auto destroy_heap = [](void* ptr){

						delete static_cast<t*>(ptr);
				};

				data_ = run_time_destructor(

						static_cast<void*>(ptr),
						destroy_heap
								);
		}


}

};



}
