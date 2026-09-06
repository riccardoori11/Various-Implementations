#include <limits>
#include <list>
#include <memory>
#include <new>
#include <optional>


/*Skipfield 
 * Skipfield pattern
 * freelist
 * freeList Head
 *
 * */

constexpr std::size_t npos = std::numeric_limits<std::size_t>::max();

namespace ricc{

template <typename T>
class hive{

private:


struct slot{

		std::optional<T> val;
		std::size_t next = npos;

};
// concept map this ?
struct raw_deleter{

		void operator()(slot* ptr){

				std::destroy_at((ptr));
				::operator delete(ptr,std::align_val_t(alignof(slot)));
		}

};

constexpr static std::size_t Block_size{8};

using raw_ptr = std::unique_ptr<slot,raw_deleter>;

raw_ptr allocate(std::size_t new_size){

		auto mem = ::operator new(sizeof(slot)* Block_size,std::align_val_t(alignof(slot)));
		return raw_ptr(static_cast<slot*>(mem));

}

constexpr static std::size_t Map_size{4};
constexpr static int gf{2};
std::size_t size_{};
std::size_t capacity_{};

slot* freehead{npos};


auto getLogicalPos(std::size_t idx){

		// ?
}


public:


hive() = default;

};




};
