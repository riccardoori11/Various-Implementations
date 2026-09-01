#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <thread>
#include <vector>

template <class It,typename T>
struct accumulate_block{

		void operator()(It first, It last, T& result){
				result = std::accumulate(first,last,result);
		}

};

namespace ricc{
;
				
constexpr std::size_t MinAmountPerThread = 10;

template <class It,typename T>


T parallel_accumulate(It first, It last,T rst){


		/*assume that it is not 0 */
		std::size_t HardWareThread = std::thread::hardware_concurrency();

		assert(HardWareThread > 0);

		std::size_t distance = std::distance(first,last);

		if (!distance){

				return rst;
		}


		 const std::size_t  max_thread = (distance + MinAmountPerThread-1)/MinAmountPerThread; 

		 assert(max_thread > 0);

		 const std::size_t NumThread = std::min(max_thread,HardWareThread);

		 const std::size_t blockSize = distance / NumThread;

		 std::vector<std::thread> threads(NumThread - 1);

		 std::vector<T> results(NumThread);

		 It block_start = first;

		 for (std::size_t i{}; i < NumThread - 1; ++i){

				 It block_end = block_start + blockSize;

				 threads[i] = std::thread(

								accumulate_block<It, T>(),
								block_start,block_end,std::ref(results[i])
								 );
				 
				 block_start = block_end;
		 }
		 
		 accumulate_block<It,T>()(block_start,last,results[NumThread-1]);

		 std::cout << threads.size() << std::endl;

		 for (auto& thread: threads){

				 thread.join();
		 }

		 return std::accumulate(results.begin(),results.end(),rst);

}

};
