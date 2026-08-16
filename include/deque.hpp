#include <iostream>
#include <array>
#include <memory>
#include <new>
#include <utility>
#include <stdexcept>
#include <vector>

namespace ricc{

/* pop right, pop left, push right, push left */
/**/


template <typename T>
class Dequeue_fixed_size{
private:

static constexpr int Block_size = 4;
static constexpr int Initial_Map_size = 8;
static constexpr std::size_t gf{2};


/*For this example, it could store 32 elements*/

struct block_deleter{

		void operator()(T* ptr) noexcept {

				::operator delete(ptr,std::align_val_t(alignof(T)));
		}};



		using block_ptr = std::unique_ptr<T,block_deleter>;
		std::vector<block_ptr> blocks;

		/* allocate a block */
		block_ptr allocate_block(){

				auto mem = ::operator new(sizeof(T) * Block_size,std::align_val_t(alignof(T)));

				return block_ptr(static_cast<T*>(mem));
		}

		
		std::size_t start = (Initial_Map_size * Block_size) / 2;
		std::size_t size_{};

		void swap(Dequeue_fixed_size& temp) noexcept{

				temp.blocks.swap(blocks);
				std::swap(start,temp.start);
				std::swap(size_, temp.size_);

		}

		void growMap(){

				

				const auto old_map_size = blocks.size();
/*
				if (old_map_size == 0){

						blocks.resize(Initial_Map_size);
						start = (Initial_Map_size * Block_size) / 2;
						return;
				}
				*/

				const auto new_map_size = old_map_size * gf;

				const auto shift_blocks = (new_map_size +1 - old_map_size) / 2;

				std::vector<block_ptr> new_blocks(new_map_size);

				for (std::size_t i{}; i < old_map_size; ++i){

						new_blocks[shift_blocks + i] = std::move(blocks[i]);
				}

				blocks.swap(new_blocks);

				start += shift_blocks * Block_size;

		}


public:

Dequeue_fixed_size():blocks(Initial_Map_size){};

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

Dequeue_fixed_size(const Dequeue_fixed_size& other):start(other.start),size_(other.size_),blocks(other.blocks.size()){

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

Dequeue_fixed_size( Dequeue_fixed_size&& other) noexcept :size_(std::exchange(other.size_,0)),start(std::exchange(other.start,0))  {
/*arr1.swap(arr2)*/

		std::cout << "Move constructor" << std::endl;
		other.blocks.swap(blocks);
}

Dequeue_fixed_size& operator= (const Dequeue_fixed_size& other){

		std::cout << "Copy assingment" << std::endl;

		if (this == &other){
				return *this;
		}

		Dequeue_fixed_size temp(other);

		swap(temp);

		return *this;

}
Dequeue_fixed_size& operator= (Dequeue_fixed_size&& other) noexcept{

		std::cout << "Move assignment" << std::endl;

		if (this == &other){

				return *this;
		}

		Dequeue_fixed_size temp(std::move(other));

		swap(temp);

		return *this;
		

}


void push_back(T&& value){


		if ((start + size_) >= (blocks.size() * Block_size)){

				growMap();
		}

		const std::size_t position = start + size_; 
		if (blocks[position / Block_size] == nullptr){

				blocks[position / Block_size]  = allocate_block();
		}

		T* destination = blocks[position/Block_size].get() + position % Block_size;

		std::construct_at(destination,value);
		++size_;

		return;
}

void push_front( T&& value){

		if ((start + size_) >= (blocks.size() * Block_size)){

				growMap();
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


void popBack(){

		const auto pos = start + size_ - 1;

		std::destroy_at(blocks[pos / Block_size].get() + (pos % Block_size));
		--size_;

}

void popFront(){

		const auto pos = start;
		std::destroy_at(blocks[pos/ Block_size].get() + (pos % Block_size));
		--size_;
		++start;
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




