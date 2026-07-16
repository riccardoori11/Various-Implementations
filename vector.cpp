#include "vector.hpp"

auto printVectorInt(const ricc::Vector<int>&  x){

		if (x.empty()){

				std::cout <<  "Empty Vector!" << std::endl;
		}

		for (std::size_t i{}; i < x.size(); ++i ){

				std::cout << x[i] << std::endl;
		}
}

auto printVectorString(const ricc::Vector<std::string>&  x){

		if (x.empty()){

				std::cout <<  "Empty Vector!" << std::endl;
		}

		for (std::size_t i{}; i < x.size(); ++i ){

				std::cout << x[i] << std::endl;
		}
}

int main(){


		ricc::Vector<ricc::Tracker> a;
		ricc::Tracker a2(4);

		a.push_back(std::move(a2));

		return 0;


}
