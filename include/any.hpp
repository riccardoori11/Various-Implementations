#include <cstddef>
#include <iostream>

namespace ricc{

class any{

private:

static constexpr std::size_t max_size{8};
static constexpr std::size_t alignment{alignof(void*)};


union storage{

		/* inline buffer */
		alignas(alignment) std::byte bytes[max_size];
		void* ptr;

} storage_;

 bool IsInline(){


}

const void *ops{nullptr};

public:

any() noexcept = default;



};



}
