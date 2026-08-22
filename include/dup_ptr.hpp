#include <type_traits>


/*
 * duplicating pointer -> duplicates pointee, also sole ownersip
 * */

struct cloneable{

		virtual cloneable* clone() const = 0;
		virtual ~cloneable() = default;
};

struct Copier{

		template<typename T>
		T* operator()(const T* p) const {

				return new T{*p};
		}

};

struct Cloner{

		template<typename T>
		T* operator() (const T* p) const {

				return p->clone();
		}

};

namespace ricc{

template <typename T, typename Dup = std::conditional_t<std::is_base_of_v<cloneable, T>, Cloner, Copier>>

class dup_ptr{

		[[no_unique_address]] 
		Dup dup{};
		T* ptr{nullptr};


public:

		dup_ptr() = default;

		bool empty() const {

				return !ptr;
		}

		dup_ptr(T* p):ptr(p ? Dup{}(p): nullptr)
		{
		}

		dup_ptr(const dup_ptr& other):ptr(other.empty() ? nullptr : Dup{}(other.ptr))
		{
		}

		T& operator*(){

				return *ptr;
		}

		const T& operator*() const{

				return *ptr;
		}



};



};
