#include <iostream>
#include "vector.hpp"


template <typename T>
constexpr T&& my_move(T& lval){

		return static_cast<T&&>(lval);
}

int main(){

		ricc::Vector<int> a;
		ricc::Vector<int> b;

		a = my_move(b); 
		return 0;
}




