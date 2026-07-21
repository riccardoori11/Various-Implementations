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

		ricc::Vector<int> a;
		a.push_back(1);
		a.push_back(4);
		a.push_back(2);
		a.push_back(5);

		std::cout << a.at(3) << std::endl;



		return 0;


}
