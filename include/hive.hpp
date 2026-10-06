#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>


namespace ricc{


template<typename T>
class Fixed_sized_hive{

		static constexpr std::size_t SIZE{10};

		// group is memory block + elements + metadata
		
		inline static std::size_t nps = std::numeric_limits<std::size_t>::max();
		union Slot{

				T object;
				std::size_t next_free;
				Slot() : next_free(nps)
				{}
		}slot;

		struct Group{

				std::size_t skip[SIZE]{};
				Slot elements[SIZE];
				std::size_t free_listHead{nps};
				std::size_t size_{};
				std::size_t used{}; 

				/*
		 * for low complexity jumping
		 * doing high complexity for now
		 * */	
				void rebuild_skip(){


						std::size_t run = 0;
						for (std::size_t i = used; i> 0;){
								--i;
								if (skip[i] != 0){
										skip[i] = ++run;
								}
								else{
										run = 0;
								}
						}
				}
 
		};

		std::unique_ptr<Group> ptr = std::make_unique<Group>();

		bool is_erased(std::size_t i){

				return i < ptr->used && ptr->skip == 0;
		}
public:

		Fixed_sized_hive() = default;

		Fixed_sized_hive(const Fixed_sized_hive& other) = delete;

		Fixed_sized_hive& operator = ( const Fixed_sized_hive& other) = delete;

		Fixed_sized_hive ( Fixed_sized_hive&& other) = delete;
		Fixed_sized_hive& operator = ( Fixed_sized_hive&& other) = delete;
		struct iterator{

				Group* g;
				// used to indicate offset for elements and skipfield
				// May be worse for performance but is easier to implement for now
				std::size_t index;
				void skip(){
						
						if (index < g->used){

								index += g->skip[index];
						}

						if (index >= g->used){
								return;
						}
				}

				T& operator*(){
						return g->elements[index].object;
				}

				iterator& operator++ (){
						++index;
						skip();
						return *this;
				}

				bool operator ==(const iterator& other) const = default;

		};
		
		iterator begin(){

				iterator it{ptr.get(), 0};

				it.skip();

				return it;
		
		}

		iterator end(){

				return {ptr.get(),ptr->used};
		}

		// in this fixed size version there are 2 possible cases for insertion:
		// either you insert in an erased slot
		// or you insert at the very end 
		iterator insert(const T& value){

				Group*g = ptr.get();
				std::size_t empty_slot;
				if ( g->free_listHead != nps){

						empty_slot = g->free_listHead;
						g->free_listHead = g->elements[empty_slot].next_free;
				}
				else if( g->used < SIZE){
						 empty_slot = g->used++;
				}
				else{
						throw std::runtime_error("no more space available");
				}

				g->elements[empty_slot].object = value;
				g->skip[empty_slot] = 0;
				g->rebuild_skip();
				return {g,empty_slot};
		}

		// three cases to think about 
		void erase(iterator it){

				Group* g = ptr.get();
				auto i = it.index;

				g->elements[i].next_free = g->free_listHead;
				g->free_listHead = i;
				g->skip[i] = 1;
				g->rebuild_skip();

		}

};
};
