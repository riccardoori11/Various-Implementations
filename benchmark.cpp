#include "vector.hpp"
#include <chrono>
#include <iterator>
#include <ratio>
#include <vector>

int main(){

		std::vector<int> a1;

		constexpr auto size = 1'000'000;

		const auto start = std::chrono::steady_clock::now();

		for (std::size_t i{}; i < size; ++i){

				a1.push_back(static_cast<int>(i));
		}

		const auto end = std::chrono::steady_clock::now();

		const auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		std::cout << time << " ms" << std::endl;

		ricc::Vector<int> a2;

		const auto start1 = std::chrono::steady_clock::now();

		for (std::size_t i{}; i < size; ++i){

				a2.push_back(static_cast<int>(i));
		}

		const auto end1 = std::chrono::steady_clock::now();

		const auto time1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count();

		std::cout << time1 << " ms" << std::endl;

		return 0;
}
