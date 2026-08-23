#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <iostream>

namespace ricc{


template<typename T>
class forward_list{

public:

		using value_type = T;
		using reference = T&;
		using const_reference = const T&;
		using pointer = T*;
		using const_pointer = const T*;
		using size_type = std::size_t;


private:
struct Node{

		value_type value;
		Node* next{nullptr};

		Node() = default; 

		Node(const_reference value):value(value)
		{
		}

		Node(value_type&& value):value(std::move((value)))
		{
		}
};

template<typename U>
class Iterator{

public:
		using value_type = typename forward_list<T>::value_type;
		using reference = typename forward_list<T>::reference;
		using difference_type = std::ptrdiff_t;
		using iterator_category = std::forward_iterator_tag;
		using pointer = typename forward_list<T>::pointer;
		using iterator_concept = std::forward_iterator_tag;
		friend class forward_list<T>;

private:
		Node* curr{};

public:

		Iterator() = default;
		Iterator(Node* p):curr(p)
		{
		}
		Iterator& operator++(){

				curr = curr->next;
				return *this;
		}
		Iterator operator++(int){

				Iterator temp = *this;
				operator++();
				return temp;
		}

		bool operator == (const Iterator& other){

				return curr == other.curr;
		}

		U& operator*(){
				return curr->value;
		}

		const U& operator*() const {
				return curr->value;
		}

		U* operator -> (){

				return curr->value;
		}

		const U* operator-> () const{

				return curr->value;
		}



};

using iterator = Iterator<T>;
using const_iterator = Iterator<const T>;

std::size_t size_{};
Node* head{nullptr};

constexpr void swap(forward_list& temp){
		
		using std::swap;
		swap(size_,temp.size_);
		swap(head,temp.head);

}

constexpr void clear(){

		for (auto it{head}; it != nullptr;){
				auto old = it->next;
				delete it;
				it = old;
		}
		size_ = 0;
}


public:

forward_list() = default;

template<std::input_iterator It>
forward_list(It b, It e){

		if (b == e){

				return;
		}

		try{

				head = new Node{*b};
				auto q = head;

				++size_;

				for (++b; b != e; ++b){

						q -> next = new Node{*b};
						q = q->next;
						++size_;

				}


		}
		catch(...){

				clear();
				throw;
		}

}

forward_list(std::initializer_list<T> lst):forward_list(lst.begin(), lst.end())
{
}

// destructor
~forward_list(){

		clear();
		
}

iterator begin(){

		return {head};
}

const_iterator begin() const{

		return {head};
}

const_iterator cbegin() const{

		return begin();
}

iterator end(){

		return {};
}

const_iterator end() const{

		return {};
}

const_iterator cend() const{

		return end();
}

forward_list(const forward_list& other):size_(other.size_){

		Node* current = other.head->next;
		head = new Node(other.head->value);
		Node* iterator{head};

		while(current != nullptr){

				Node* newNode = new Node(current->value);
				iterator->next = newNode;
				current = current->next;
				iterator = iterator->next;

		}
}



forward_list& operator= (const forward_list& other){

		if (this == &other){

				return *this;
		}

		forward_list temp(other);

		swap(temp);

		return *this;

} 

forward_list (forward_list&& other) noexcept :size_(std::exchange(other.size_,0)),head(std::exchange(other.head,nullptr)  )
{
		std::cout << "Move constructor" << std::endl;
}

forward_list& operator= (forward_list&& other) noexcept {

		std::cout << "Move assignment" << std::endl;
		if (this == &other){

				return *this;
		}

		forward_list temp(std::move(other));

		swap(temp);

		return *this;
}


void assign(std::size_t count, const T& value){

		clear();

		if (count <= 0){

				throw std::range_error("Invalid size");
		}

		Node* dummy = new Node();
		dummy ->next = head;
		Node* iterator = dummy;

		for (std::size_t i{}; i < count; ++i){

				Node* newNode = new Node(value);
				iterator->next = newNode;
				iterator = iterator->next;
		}

		head = dummy->next;

		delete dummy;
}

template <typename U>
void push_front(U&& value){

		Node* oldhead = head;
		Node* newNode = new Node(std::forward<U>(value));
		newNode->next = oldhead;
		head = newNode;
		size_++;
}

iterator insert_after(iterator pos, const_reference value){

		auto new_value = new Node{value};

		new_value ->next = pos.curr->next;
		pos.curr->next = new_value;

		++size_;

		return new_value;

}


iterator erase_after(iterator pos){

		auto p = pos.curr->next->next;
		delete pos.curr->next;
		pos.curr->next = p;

		return {p->next};
}


constexpr void reverse(){

		Node* node{nullptr};
		Node* current{head};

		while (current != nullptr){

				Node* temp = current->next;

				current->next = node;
				node = current;
				current = temp;
		}

		head = node;

}

constexpr reference front(){

		return head->value;
}

constexpr const_reference front() const{
		return head->value;
}

bool operator == (const forward_list& other){

		return (size_ == other.size_ )&& (std::equal(begin(),end(),other.begin()));
}

void Print() const{

		Node* current = head;
		while (current != nullptr){

				std::cout << current->value << std::endl;
				current = current->next;
		}
}

constexpr size_type size() const{

		return size_;
}

constexpr void popFront(){

		Node* old = head->next;
		delete head;
		head = old;
		--size_;
}

bool empty() const noexcept {

		return !head;
}


};


}

