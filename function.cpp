#include <functional>
#include <iostream>

auto printNum(int i){

		std::cout << i << std::endl;
}

int main(){

		std::function<void(int)> f_display = printNum;
		f_display(-9);
		
		return 0;
}
