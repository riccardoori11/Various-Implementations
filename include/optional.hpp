#pragma once
#include <exception>
#include <exception>
#include <memory>
namespace ricc{

// std::bad_optional_access
struct bad_optional_access: std::exception{};

// gives ppl a clear way to say its empty
struct nullopt_t{

		constexpr explicit nullopt_t(int)
		{
		};

};

// meaning you cant build it explicitly from {}
inline constexpr nullopt_t nullopt{0};

template<typename T>
class optional{

private:

		union storage{
				struct Empty {} empty{};
				T value;
				// non trivial destructor case, used to indicate to compiler that we will use teh destructor ourselves
				~storage(){}
		} storage_{};


		bool has_value_{};

public:

optional() = default;

optional(T value):has_value_(true)
{
		std::construct_at(&storage_.value,std::move(value));
}

~optional(){

		if (has_value_){
				storage_.value.~T();
		}
}

optional(nullopt_t)
{
}

optional& operator= (nullopt_t) noexcept{
		if (has_value_){
				storage_.value.~T();
		}
		has_value_ = false;

		return *this;
}

explicit operator bool() const noexcept {
		return has_value_;
}

bool has_value()const noexcept{

		return has_value_;
}

constexpr T* operator->() noexcept{
		return &storage_.value;
}
constexpr const T* operator->() const noexcept{
		return &storage_.value;
}

constexpr T& operator*() & noexcept{
		return storage_.value;
}

constexpr const T& operator*() const& noexcept{
		return storage_.value;
}

constexpr T& value() {
		if (has_value_){
				return storage_.value;
		}

		throw bad_optional_access{};
		
}
constexpr const T& value() const{
		if (has_value_){
				return storage_.value;
		}
		throw bad_optional_access{};
}
		
};

}
