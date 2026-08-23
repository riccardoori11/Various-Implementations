#include "vector.hpp"
#include <assert.h>

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


struct A{

		std::string name;

		A(std::string name):name(std::move(name)){

				std::cout << "A has been Constructed" << std::endl;
		}

		A(const A& other):name(other.name){
				std::cout << "A has been Copy Constructed" << std::endl;
		}

		A& operator = (const A& other){
				name = other.name;
				std::cout << "A has been Copy assigned" << std::endl;
				return *this;
		}

		A(A&& other) noexcept : name(std::exchange(other.name,"")){
				std::cout << "A has been Move Constructed" << std::endl;
		}
		A& operator = (A&& other) noexcept {

				name = std::move(other.name);
				std::cout << "A has been Move assigned" << std::endl;

				return *this;
		}


};


int main(){
		ricc::Vector<int> a{};
		a.push_back(4);
		ricc::Vector<int> b{a};

		assert(a[0] == b[0]);

		ricc::Vector<int> c;
		c = b;

		assert(b[0] == c[0]);

		ricc::Vector<int> d(std::move(a));

		assert(d[0] == 4);
		assert(a.size() == 0);
		assert(a.capacity() == 0);

		b = std::move(d);

		assert(b[0] == 4);

		b.pop_back();

		assert(b.empty());
/*
		ricc::Vector<A> A;
		A.reserve(10);
		struct A two{"two"};

		struct A three{"three"};

		std::cout << "emplace" << std::endl;
		A.emplace_back("Ricc");
		std::cout << "emplace with A&" << std::endl;
		A.emplace_back(two);
		std::cout << "emplace with A&&" << std::endl;
		A.emplace_back(std::move(three));


*/

/*		ricc::Vector<int> std12;
		std12.push_back(45);
		std12.push_back(45);
		std12.push_back(45);
		
		std::cout << std12 << std::endl;

		*/

		ricc::Vector<int> A{1,23,4,44444};

		assert(A.front() == 1);
		assert(A.back() == 44444);
		return 0;


}
