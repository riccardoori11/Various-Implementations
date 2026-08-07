#include <iostream>
#include <utility>
#include <stdexcept>

namespace ricc{

/* pop right, pop left, push right, push left */


template <typename T>
class Dequeue{

private:


static constexpr std::size_t sizeOfBlock{4};
static constexpr std::size_t NUMBER_OF_BLOCKS{8};

T* blocks[NUMBER_OF_BLOCKS]{};

std::size_t back_block{NUMBER_OF_BLOCKS / 2};
std::size_t front_block{NUMBER_OF_BLOCKS / 2};

std::size_t front_index{};
std::size_t back_index{};

std::size_t size{};

public:

Dequeue() = default;


Dequeue(const Dequeue& other):back_block(other.back_block),front_block(other.front_block),front_index(other.front_index),back_index(other.back_index),size(other.size){

		std::cout << "Copy constructor" << std::endl;

		for (std::size_t i{}; i < NUMBER_OF_BLOCKS; ++i){

				if (other.blocks[i] != nullptr){

						blocks[i] = new T[sizeOfBlock];

						for (std::size_t j{}; j < sizeOfBlock; ++j){

								blocks[i][j] = other.blocks[i][j];
						}
				}
		}

}


Dequeue& operator= (const Dequeue& other){

		std::cout << "Copy assignment" << std::endl;

		if (this == &other){

				return *this;
		}

		for (std::size_t i{}; i < NUMBER_OF_BLOCKS; ++i){

				delete[] blocks[i];
				blocks[i] = nullptr;
		}

		for (std::size_t j{}; j < NUMBER_OF_BLOCKS; ++j){

				if (other.blocks[j] != nullptr){

						blocks[j] = new T[sizeOfBlock];

						for (std::size_t k{}; k < sizeOfBlock; ++k){

								blocks[j][k] = other.blocks[j][k];
						}
				}
		}

		return *this;


}

Dequeue(Dequeue&& other) noexcept:
		back_block(std::exchange(other.back_block, 0)),front_block(std::exchange(other.front_block, 0)),front_index(std::exchange(other.front_index,0))
		,back_index(std::exchange(other.back_index,0)),size(std::exchange(other.size,0)){

				std::cout << "Move constructor" << std::endl;
		for (std::size_t i{}; i < NUMBER_OF_BLOCKS; ++i){

				blocks[i] = std::exchange(other.blocks[i], nullptr);
		}

}


Dequeue operator= (Dequeue&& other) {

		if (this == &other){

				return *this;
		}

}


};



}
