#include "quicksort.hpp"
#include <assert.h>
#include <list.hpp>
#include <iostream>
#include <list>

template<typename T>
std::ostream& operator << (std::ostream& os, std::list<T>& list){

		for (auto i: list){

				os << ' ' << i;
		}
		return os;
}

int main(){

		std::list<int> a = {1,231,9,3,12,0,1,312};
		auto sorted = std::move(ricc::quicksort_fp(a));
		std::list<int> value{0 ,1, 1, 3, 9, 12, 231, 312};
		assert(sorted == value);

		auto sorted1 = std::move(ricc::parallelQuickSort(a));
		assert(sorted1 == value);


		auto sorted2 = ricc::quickSort_chunk(a);
		assert(sorted2 == value);


		return 0;
}
