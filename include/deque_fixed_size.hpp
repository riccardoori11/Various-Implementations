#include <iostream>
#include <array>
#include <memory>
#include <new>
#include <utility>
#include <stdexcept>

namespace ricc{

/* pop right, pop left, push right, push left */
/**/

struct Tracker{

		inline static std::size_t alive{};
		inline static std::size_t constructed{};
		inline static std::size_t destroyed{};

		Tracker(){
				++alive;
				++constructed;
		}

		Tracker(const Tracker& other){

				alive++;
				++constructed;
		}
		Tracker( Tracker&& other){

				alive++;
				++constructed;
		}

		~Tracker(){

				--alive;
				++destroyed;
		}


};

template <typename T>
class Dequeue_fixed_size{
private:

static constexpr int Block_size = 4;
static constexpr int Map_size = 8;

/*For this example, it could store 32 elements*/
static constexpr int capacity = Block_size * Map_size;

struct block_deleter{

		void operator()(T* ptr) noexcept {

				::operator delete(ptr,std::align_val_t(alignof(T)));
		}};



		using block_ptr = std::unique_ptr<T,block_deleter>;
		std::array<block_ptr, Map_size> blocks{};

		/* allocate a block */
		block_ptr allocate_block(){

				auto mem = ::operator new(sizeof(T) * Block_size,std::align_val_t(alignof(T)));

				return block_ptr(static_cast<T*>(mem));
		}

		
		std::size_t start = capacity / 2;
		std::size_t size_{};


public:

Dequeue_fixed_size() = default;

~Dequeue_fixed_size(){
/*Deconstruct elements
 * and then deallocate raw memory
 * however our allocator is RAII safe, thus I dont know if we necessarily need to */

		for (std::size_t i{}; i < size_; ++i){

				std::size_t pos = start + i;
				auto which_block = pos / Block_size;
				auto offset = pos % Block_size;

				std::destroy_at(blocks[which_block].get() + offset);
		}
		
}

Dequeue_fixed_size(const Dequeue_fixed_size& other):start(other.start),size_(other.size_){

		std::cout << "Copy constructor" << std::endl;

		for (std::size_t i{}; i < size_; ++i){

				auto pos = start + i;
				if (blocks[pos / Block_size] == nullptr){

						blocks[pos/ Block_size] = allocate_block();
				}
				auto offset = pos % Block_size;

				T* destination = blocks[pos / Block_size].get() + offset;
				std::construct_at(destination, *(other.blocks[pos/ Block_size].get() + offset));

		} 
}

Dequeue_fixed_size( Dequeue_fixed_size&& other):size_(std::exchange(other.size_,0)),start(std::exchange(other.start,0)){
/*arr1.swap(arr2)*/

		std::cout << "Move constructor" << std::endl;
		other.blocks.swap(blocks);
}

Dequeue_fixed_size& operator= (const Dequeue_fixed_size& other){

}
Dequeue_fixed_size& operator= (Dequeue_fixed_size&& other) = delete;


void push_back(T&& value){

		const std::size_t position = start + size_; 

		if (position == capacity){

				throw std::out_of_range("Position the same size of capacity");
		}

		if (blocks[position / Block_size] == nullptr){

				blocks[position / Block_size]  = allocate_block();
		}

		T* destination = blocks[position/Block_size].get() + position % Block_size;

		std::construct_at(destination,value);
		++size_;

		return;
}

void push_front( T&& value){

		if (start + size_ == capacity){

				throw std::out_of_range("capacity is full");
		}

		const std::size_t pos = start-1;
		if (blocks[pos / Block_size] == nullptr){

				blocks[pos / Block_size] = allocate_block();
		}
		const auto offset = pos % Block_size;
		T* destination = blocks[pos/Block_size].get() + offset;

		std::construct_at(destination, value);

		start = pos;
		++size_;
		return;

}



constexpr std::size_t size(){

		return size_;
}

constexpr T& at(std::size_t idx){

		if (idx >= size_){

				throw std::out_of_range("idx too large");
		}

		std::size_t position = start + idx;

		std::size_t block = position / Block_size;
		std::size_t offset = position % Block_size;

		return *(blocks[block].get() + offset);
}


};


};




