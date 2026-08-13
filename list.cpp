#include <iostream>
#include <list.hpp>

int main(){

		ricc::forward_list<int> a{};

		a.pushfront(1);
		a.pushfront(3);

		a.print();

		return 0;
}
