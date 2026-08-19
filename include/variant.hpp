#include <algorithm>
#include <utility>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <variant>

namespace ricc{

/*Forward declaration for get*/
template<typename First,typename ...Rest>
class variant;

template<typename T, typename First, typename ...Rest>
constexpr T& get(variant<First, Rest...>& value);

template<typename T, typename First, typename ...Rest>
constexpr const T& get(const variant<First, Rest...>& value);
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

		template<typename T>
		static void Copy_type(void* destination, const void* source){

				const auto* object_source = static_cast<const T*>(source);
				std::construct_at( static_cast<T*>(destination), *object_source);
		}


		template<typename T>
		static void Move_Type(void* destination, void* source){

				auto *dst = static_cast<T*>(destination);
				auto *src = static_cast<T*>(source);

				std::swap(*dst,*src);

		}
/*
		template<typename T>
		static void Destroy_type(void* ptr){

				auto* obj = static_cast<T*>(ptr);

				std::destroy_at(obj);
		}

		*/
		/*type erasure*/
		struct operations{

				using Copy = void (*)(void* , const void* );
				Copy copy;

				using Move = void (*)(void* , void* );
				Move move;

				using Destroy = void(*)(void);
				Destroy destroy;
				
		};

		constexpr static operations op[] = {
				operations{
						&Copy_type<First>,
						&Move_Type<First>,
				},
				operations{
						&Copy_type<Rest...>,
						&Move_Type<Rest...>,
				}
				

		};



public:
		/*value initialized*/
		variant() :index(){
				std::construct_at(reinterpret_cast<First*>(data_));
		}

		constexpr std::size_t getIndex() const {
				return  index;
		
		}

		template <typename U>
		requires (!std::is_same_v<variant,std::remove_cvref_t<U>>)
		variant(U&& object){

				using U_NORM = std::decay_t<U>;

				constexpr std::size_t index_types = index_of_v<U_NORM,First,Rest...>;

				std::construct_at(reinterpret_cast<U_NORM*>(data_),std::forward<U>(object));

				index = index_types;
				
		}
		variant(const variant& other){

				op[other.index].copy
						(
						 data_,
						 other.data_
						 );

				index = other.index;

		}
		
		variant& operator= (const variant& other) = delete;

		variant(variant&& other):index(std::exchange(other.index,0))
		{

				op[other.index].move(

						data_,
						other.data_
								);
		}
		variant& operator = (variant&& other) = delete;
/*
		~variant(){

				op[index].destroy(data_);
		}
*/
		template<typename T, typename F, typename ...R>
		friend constexpr T& get(variant<F,R...>& value);

		template<typename T, typename F, typename ...R>
		friend constexpr const T& get(const variant<F,R...>& value);

	

};

template <typename T,typename First, typename ...Ts>
constexpr bool holds_alternative(const variant<First,Ts...>& value){

		constexpr std::size_t expected_index = index_of_v<T,First,Ts...>;

		return value.getIndex() == expected_index;
}


template <typename T,typename First, typename ...Rest>
constexpr T& get(variant<First,Rest...>& value){

		constexpr std::size_t expected_index = index_of_v<T,First, Rest...>;
		using T_NORM = std::decay_t<T>;
		if (expected_index == value.getIndex()){

				auto* ptr = reinterpret_cast<T_NORM*>(value.data_);
				return *ptr;
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

				auto *ptr =  reinterpret_cast<const T_NORM*>(value.data_);
				return *ptr;
				 
		}
		else{

				std::bad_variant_access();
		}
}
 
}
