#include <algorithm>
#include <cassert>
#include <functional>
#include <future>
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

		T operator()(It first, It last){

				return std::accumulate(first,last,T());
		}

};


class JoinThreads{

		std::vector<std::thread>& threads;

public:

		JoinThreads(std::vector<std::thread>& thread):threads(thread)
		{
		}

		~JoinThreads(){

				for (auto& thread: threads){

						if (thread.joinable()){

								thread.join();
						}
				}

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


		 for (auto& thread: threads){

				 thread.join();
		 }

		 return std::accumulate(results.begin(),results.end(),rst);

}

template<typename It, typename  T>
T parallel_accumulatePackaged_task(It first, It last,T rst){


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

		 JoinThreads joiner(threads);

		 std::vector<std::future<T>> results(NumThread - 1);

		 It block_start = first;

		 for (std::size_t i{}; i < NumThread - 1; ++i){

				 It block_end = block_start + blockSize;

				 std::packaged_task<T(It,It)> task{accumulate_block<It, T>()};
				 results[i] = task.get_future();
				 threads[i] = std::thread(

								std::move(task),
								block_start,block_end
								 );
				 
				 block_start = block_end;
		 }
		 
		 T last_result = accumulate_block<It,T>()(block_start,last);

		 T result = rst;

		 for (std::size_t i{}; i < NumThread - 1; ++i){

				 result += results[i].get();
		 }

		 result += last_result;

		 return result;

}

};
