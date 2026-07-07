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
		
		a.push_back(ricc::Tracker{1});
		a.push_back(ricc::Tracker{2});
		
		std::cout << "After reserve" << std::endl;
		a.reserve(100);


		return 0;


}
