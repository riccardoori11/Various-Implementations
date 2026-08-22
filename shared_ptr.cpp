#include <chrono>
#include <iostream>
#include <random>
#include <ratio>
#include <thread>
#include "shared_ptr.hpp"

struct X{

		int n;

		X(int n):n(n) 
		{}

		~X(){

				std::cout << "~X()" << '\n';
		}


};

auto main()->int{

		std::random_device pnrg{};
		std::uniform_int_distribution<int> die(200,300);

		int time = die(pnrg);

		ricc::shared_ptr<X> p{new X{3}};

		std::thread t0{[p,time] {

				std::this_thread::sleep_for(std::chrono::milliseconds{time});
				std::cout << "End of t0, p->n" << p-> n << '\n';
		}};

		std::thread t1{[p,time] {

				std::this_thread::sleep_for(std::chrono::milliseconds{time});
				std::cout << "End of t1, p->n" << p-> n << '\n';
		}};

		t1.detach();
		t0.detach();

		std::this_thread::sleep_for(std::chrono::milliseconds(350));

		std::cout << "end main" << '\n';
		
		return 0;
}
