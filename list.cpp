#include <iostream>
#include <assert.h>
#include <list.hpp>

int main(){

		ricc::forward_list<char> letters{};
		letters.push_front('a');

		assert(letters.front() == 'a');

		ricc::forward_list<char> letters1{letters};
		assert(letters1.front() == 'a');

		ricc::forward_list<char> letters2{};
		letters2 = letters;
		assert(letters2.front() == 'a');

		ricc::forward_list<char> letters3{};
		letters3 = std::move(letters);
		assert(letters3.front() == 'a');

		letters3.push_front('b');
		assert(letters3.front() == 'b');

		letters3.reverse();
		assert(letters3.front() == 'a');

		letters3.popFront();
		assert(letters3.front() == 'b');
		assert(letters3.size() == 1);

		letters3.assign(2, 'a');
		letters3.Print();

		return 0;
}
