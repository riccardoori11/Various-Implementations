#include <iostream>
#include <memory>

int main(){
		
		int* p = new int [10];
		std::unique_ptr<int> p1{p};	
		

		return 0;

}
