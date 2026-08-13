#include <iostream>
#include <assert.h>
#include <list.hpp>

int main(){

		ricc::forward_list<char> letters{};
		letters.push_front('a');

		assert(letters.front() == 'a');
		letters.push_front('b');
		assert(letters.front() != 'a');

/*
		b.print();

		b.reverse();
		b.print();
*/



		return 0;
}
