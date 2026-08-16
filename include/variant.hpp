#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>
#include <type_traits>
#include <variant>

namespace ricc{

/*Forward declaration for get*/
/*
template<typename First,typename ...Rest>
class variant;

template<typename T, typename First, typename ...Rest>
constexpr T& get(variant<First, Rest...>& value);

template<typename T, typename First, typename ...Rest>
constexpr const T& get(const variant<First, Rest...>& value);
*/
/*
 * Generalisation
 * wrap idx in index_of
 * */
template <typename T, typename... Rest>
struct index_of;
/*
 * stop when first type is T
 * */

template <typename T, typename... Rest>
struct index_of<T,T,Rest...> : std::integral_constant<std::size_t,0>{};

/*
 * Recursion to check get idx
 * */
template <typename T, typename First, typename ...Rest>
struct index_of<T,First, Rest...>: 
		std::integral_constant<std::size_t, 1+ index_of<T,Rest...>::value>{};

template <typename T, typename ...Rest>
constexpr std::size_t index_of_v = index_of<T,Rest...>::value;

template<typename First, typename... Rest>
class variant{

private:
		std::size_t index{};

		static constexpr std::size_t Storage_size_ = std::max({sizeof(First),sizeof (Rest)...});
		static constexpr std::size_t Storage_align = std::max({alignof(First),alignof(Rest)...});

		alignas(Storage_align) std::byte data_[Storage_size_];

public:
		/*value initialized*/
		variant() :index(){
				std::construct_at(reinterpret_cast<First*>(data_));
		}

		constexpr std::size_t getIndex() const {
				return  index;
		
		}

		template <typename U>
		variant(U&& object){

				using U_NORM = std::decay_t<U>;

				constexpr std::size_t index_types = index_of_v<U_NORM,First,Rest...>;

				std::construct_at(reinterpret_cast<U_NORM*>(data_),std::forward<U>(object));

				index = index_types;
				
		}

		variant(const variant& other) = delete;
		variant& operator= (const variant& other) = delete;

		variant(variant&& other) = delete;
		variant& operator = (variant&& other) = delete;

		~variant(){

		}
/*
		template<typename T, typename F, typename ...R>
		friend constexpr T& get(variant<F,R...>& value);

		template<typename T, typename F, typename ...R>
		friend constexpr const T& get(const variant<F,R...>& value);

		*/
		constexpr auto& data() noexcept {

				return data_;
		}

};

template <typename T, typename ...Rest>
constexpr bool holds_alternative(const variant<Rest...>& value){

		constexpr std::size_t expected_index = index_of_v<T,Rest...>;

		return value.getIndex() == expected_index;
}


template <typename T,typename First, typename ...Rest>
constexpr T& get(variant<First,Rest...>& value){

		constexpr std::size_t expected_index = index_of_v<T,First, Rest...>;
		using T_NORM = std::decay_t<T>;
		if (expected_index == value.getIndex()){

				return *reinterpret_cast<T_NORM*>(value.data());
		}
		else{

				throw std::bad_variant_access();
		}
}

template <typename T,typename First, typename ...Rest>
constexpr const T& get(const variant<First,Rest...>& value){

		constexpr std::size_t expected_index = index_of_v<T,First, Rest...>;
		using T_NORM = std::decay_t<T>;
		if (expected_index == value.getIndex()){

				return *reinterpret_cast<const T_NORM*>(value.data());
		}
		else{

				throw std::bad_variant_access();
		}
}
 
}
