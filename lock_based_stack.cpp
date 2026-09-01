#include "thread_based_stack.hpp"
#include <thread>

void add_to_stack(ricc::lb_stack<int> stack){

		stack.push(5);
}

int main(){

		ricc::lb_stack<int> a{};

		std::thread t1(add_to_stack,a);
		std::thread t2(add_to_stack,a);

		t1.join();
		t2.join();

		return 0;
}
