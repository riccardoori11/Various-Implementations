#include <array>
#include <limits>
#include <list>
#include <memory>
#include <new>
#include <optional>


/*Skipfield 
 * Skipfield pattern
 * freelist
 * freeList Head
 * use union ?
 * */

constexpr std::size_t initial_blockSize{8};

namespace ricc{

template <typename T>
class hive{

private:

struct free_list{

		std::size_t next;
		std::size_t prev;



};

union slot{

		T value;
		free_list lst;

};

struct block_deleter{

		void operator()(slot* ptr){

				::operator delete(ptr,std::align_val_t(alignof(slot)));
		}

};

using block_ptr = std::unique_ptr<slot,block_deleter>;

constexpr static int gf{2};

static block_ptr allocate(std::size_t cap){

		auto mem = ::operator new(sizeof(slot) * cap,std::align_val_t(alignof(slot)));
		return block_ptr(static_cast<slot*>(mem));
}

/*Place holder to represent nothing essentially
 * basically the sentinel*/
static constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

struct group{

		std::size_t capacity_{};
		std::size_t size_{};
		group* next{nullptr};
		group* prev{nullptr};

		block_ptr elements;

		std::unique_ptr<std::size_t []> skipField;

		std::size_t free_head{npos};

		std::size_t next_unused{};

		group(std::size_t cap):elements(allocate(cap)),skipField(std::make_unique<std::size_t []>(cap+ 1)), capacity_(cap) 
		{
		}


};

group* first_g;	
group* last_g;

std::size_t total_size{};

void appendGroup(std::size_t cap){

		group * new_group = new group(cap);

		new_group -> prev = last_g;

		if (last_g != nullptr){
				last_g->next = new_group;
		}
		else{

				first_g = new_group;
		}

		last_g = new_group;

}

void insert_In_Block(group g, const T& value){



}


public:


hive() = default;
/*
 * Insertion has 3 different situations
 *  Reuse old position
 *  add at the end
 *  or allocate another group
 *
 *
 * */
void insert(const T& value){


		if (last_g == nullptr){

				appendGroup(initial_blockSize);
		}

		slot& dst = last_g->elements.get()[last_g->next_unused];

		std::construct_at(&dst.value, value);

		++last_g->next_unused;
		++last_g->size_;
		++total_size;
}

[[nodiscard]] std::size_t size() const noexcept{

		return total_size;

}

void erase(std::size_t pos, group& g){


		slot* slots = g.elements.get();

		const std::size_t old_head = g.free_head;

		std::destroy_at(&slots[pos].value);

		std::construct_at(&slots[pos].lst);
		slots[pos].lst.prev = npos;
		slots[pos].lst.next = old_head;

		if (old_head != npos){
				slots[old_head].lst.prev = pos;
		}

		g.free_head = pos;

		--g.size_;
		--total_size;
		
}

};




};
