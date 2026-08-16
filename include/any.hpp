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

using deleter = std::unique_ptr<void,void(*)(void*)>;

constexpr static auto SSO_SIZE = sizeof(void*) * 4;

union{

		std::array<std::byte,SSO_SIZE> inline_buff;
		void* ptr;
}
sso;

deleter data_{

		nullptr,
		[](void*){}
};


const std::type_info* type_;

public:

template <typename T>
any(T object){

		using U = std::decay_t<T>;

		if constexpr (is_small<U>){

				auto ptr = std::construct_at(reinterpret_cast<U*>(sso.inline_buff.data()), object);

				auto destory_soo_buff = [](void* ptr){

						std::destroy_at(static_cast<U*>(ptr));
				};

				data_ = {
						ptr,
						destory_soo_buff
				};

		}
		else{
				auto ptr = new U(object);

				auto destory_soo_buff = [](void* ptr){

						delete static_cast<U*>(ptr);
				};

				data_ = {
						ptr,
						destory_soo_buff
				};
		}

		type_ = &typeid(U);
}

template<typename T>
T& any_cast(){

		using U = std::decay_t<T>;
		if (!type_  || type_ != &typeid(U)){

				throw std::bad_any_cast();
				
		}

		return *static_cast<U*>(data_.get());
}

};
}
