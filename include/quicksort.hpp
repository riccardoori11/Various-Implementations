#include <algorithm>
#include <optional>
#include "thread_based_stack.hpp"
#include <exception>
#include <future>
#include <list>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <thread>
#include <vector>



template<typename T>
struct sorter{

		struct chunk_to_sort{
				std::list<T> data;
				std::promise<std::list<T>> promise;
		};

		std::vector<std::thread> thread_pool;
		std::size_t thread_count ;
		ricc::lb_stack<chunk_to_sort> chunks;

		std::atomic<bool> end_of_data;

sorter(): end_of_data(false),thread_count(std::thread::hardware_concurrency() - 1)
		{

		}

~sorter(){

		end_of_data = true;
		for (auto& thread:thread_pool){

				thread.join();
		}
}

void sort_thread(){

		while (!end_of_data){

				try_sort_chunk();
				std::this_thread::yield();
		}
}

std::list<T> do_sort(std::list<T>& chunk){

		if (chunk.empty()){

				return {};
		}

		std::list<T> result;

		result.splice(result.begin(),chunk, chunk.begin());

		auto pivot = *result.begin();

		auto divide_point = std::partition(chunk.begin(),chunk.end(), [&](auto& t){

				return	t < pivot;
				
						});

		chunk_to_sort new_lower_chunk;

		new_lower_chunk.data.splice(new_lower_chunk.data.end(), chunk, chunk.begin(), divide_point);

		std::future<std::list<T>>  new_lower = new_lower_chunk.promise.get_future();

		chunks.push(std::move(new_lower_chunk));

		if (thread_pool.size() < thread_count){

				thread_pool.push_back(std::thread(&sorter::sort_thread,this));
		}

		std::list<T> new_higher(do_sort(chunk));

		result.splice(result.end(),new_higher);

		while (new_lower.wait_for(std::chrono::seconds(0)) != std::future_status::ready){

				try_sort_chunk();
		}

		result.splice(result.begin(), new_lower.get());

		return result;

}


void try_sort_chunk(){

		std::optional<chunk_to_sort> chunk = chunks.pop();

		if (chunk){

				sort_chunk(chunk);
		}
}


void sort_chunk(std::optional<chunk_to_sort>& chunk){

		chunk->promise.set_value(do_sort(chunk->data));
}


};

namespace ricc{


template<typename T>
std::list<T> quickSort_chunk(std::list<T> list){

		if (list.empty()){

				return {};
		}

		sorter<T> s;
		return s.do_sort(list);
}



template<typename T>
std::list<T> quicksort_fp(std::list<T> input){

		if (input.empty()){
		
				return {};
		}

		std::list<T> result;

		result.splice(result.begin(), input, input.begin());

		auto pivot = *result.begin();

		auto divide_point = std::partition(input.begin(),input.end(),[&](T& t){

				return t < pivot;
						});

		std::list<T> lower_part;

		lower_part.splice(lower_part.begin(),input, input.begin(), divide_point);

		auto new_lower = quicksort_fp(std::move(lower_part));
		auto new_higher = quicksort_fp(std::move(input));

		result.splice(result.begin(), std::move(new_lower));
		result.splice(result.end(), std::move(new_higher));

		return result;

}

template<typename T>
std::list<T> parallelQuickSort(std::list<T> input){

		if (input.empty()){

				return {};
		}

		std::list<T> result;

		result.splice(result.begin(), input, input.begin());

		auto pivot = *result.begin();

		auto divide_point = std::partition(input.begin(),input.end(),[&](T& t){

				return t < pivot;
						});

		std::list<T> lower_part;

		lower_part.splice(lower_part.begin(), input,input.begin(), divide_point);

		std::future<std::list<T>> newLower(std::async(&parallelQuickSort<T>,std::move(lower_part)));

		auto new_higher = parallelQuickSort(std::move(input));

		result.splice(result.end(), std::move(new_higher));

		result.splice(result.begin(), std::move(newLower.get()));

		return result;


}



};



