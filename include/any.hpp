#include <any>
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
static constexpr bool is_small = std::is_same_v<T,int>;  



struct operations{

		void* (*copy)(void*, const void*);

};



constexpr static auto SSO_SIZE = sizeof(void*) * 4;

union SBO{

		std::array<std::byte,SSO_SIZE> inline_buff;
}
sbo;


void* data_;

template<typename T>
static void* copy_type(void* destination, const void* source){

		using T_NORM = std::decay_t<T>;

		T* T_destination = static_cast<T*>(destination);

		const T* T_src = static_cast<const T*>(source);

		if constexpr (is_small<T_NORM>) {
				return std::construct_at(T_destination, *T_src);
		}
		else{
				std::cout << "Heap allocated!!!!" << std::endl;
				return new T(*T_src);
		}
}
template<typename T>
inline static operations op{

		&copy_type<T>
};

const std::type_info* type_;

const operations* oper;

public:

any() = default;

template <typename T>
any(T object){

		using T_NORM = std::decay_t<T>;

		type_ = &typeid(T_NORM);

		oper = &op<T_NORM>;

		if constexpr (is_small<T_NORM>) {
		
				T* loc = reinterpret_cast<T_NORM*>(sbo.inline_buff.data());

				auto* ptr = std::construct_at(loc,object );
				data_ = ptr;
		}
		else{

				auto* ptr = new T(object);
				data_ = ptr;
		}

}

any(const any& other):oper(other.oper){	

		std::cout << "Copied " << std::endl;

		data_ = oper->copy(

				sbo.inline_buff.data(),
				other.data_
						);

}

template<typename T>
T& any_cast(){

		using T_NORM = std::decay_t<T>;

		return *reinterpret_cast<T_NORM*>(data_);
}


};
}
