#include "unique_ptr.hpp"

int main(){

		ricc::unique_ptr<int> h(new int(10));

		ricc::unique_ptr<int>x(std::move(h));


		return 0;
}
