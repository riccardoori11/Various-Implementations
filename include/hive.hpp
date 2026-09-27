#include <iostream>
#include <memory>


namespace ricc{


template<typename T>
class Fixed_sized_hive{

		static constexpr std::size_t SIZE{10};

		// group is memory block + elements + metadata
		
		union Slot{

				T object;
				std::size_t next_free;
				Slot() : next_free(SIZE)
				{
				}
		};

		struct Group{

				std::size_t skip[SIZE]{};
				Slot elements[SIZE];
				std::size_t used{};
				std::size_t size_{};

		};

		std::unique_ptr<Group> ptr = std::make_unique<Group>();

		class iterator{

				Group* g;
				// used to indicate offset for elements and skipfield
				// May be worse for performance but is easier to implement for now
				std::size_t index;

				void skip(){
						
						if (index < g->used){

								index += g->skip;
						}

						if (index >= g->used){
								std::cout << "No more live elements ahead" << std::endl;
						}
				}

				int& operator*(){
						return g->elements[index].value;
				}

				iterator& operator++ (){
						++index;
						skip();
						return *this;
				}

		};


public:

		Fixed_sized_hive() = default;

		Fixed_sized_hive(const Fixed_sized_hive& other) = delete;

		Fixed_sized_hive& operator = ( const Fixed_sized_hive& other) = delete;

		Fixed_sized_hive ( Fixed_sized_hive&& other) = delete;
		Fixed_sized_hive& operator = ( Fixed_sized_hive&& other) = delete;

		
		iterator begin(){

				iterator it{ptr.get(), 0};

				it.skip();

				return it;
				
		}

		iterator insert(T value){


		}
};


};
