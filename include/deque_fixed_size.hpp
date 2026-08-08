#include <iostream>
#include <array>
#include <memory>
#include <new>
#include <utility>
#include <stdexcept>

namespace ricc{

/* pop right, pop left, push right, push left */
/**/

template <class T>
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

/*Handle RAII later*/		
Dequeue_fixed_size(const Dequeue_fixed_size& other) = delete; 
Dequeue_fixed_size( Dequeue_fixed_size&& other) = delete;

Dequeue_fixed_size& operator= (const Dequeue_fixed_size& other) = delete;
Dequeue_fixed_size& operator= (Dequeue_fixed_size&& other) = delete;


void pushBack(int value){

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

std::size_t size(){

		return size_;
}

T& at(std::size_t idx){

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




