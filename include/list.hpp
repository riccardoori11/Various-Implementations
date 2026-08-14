#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <iostream>

namespace ricc{


template<typename T>
class forward_list{

private:

		using reference = T&;

struct ListNode{

		T value;
		ListNode* next{nullptr};

		ListNode() = default; 

		ListNode(const T& value):value(value)
		{
		}

		ListNode(T&& value):value(std::move((value)))
		{
		}
};

std::size_t size_{};
ListNode* head{nullptr};

constexpr void swap(forward_list& temp){
		
		std::swap(size_,temp.size_);
		std::swap(head,temp.head);

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

// destructor
~forward_list(){

		clear();
		
}

forward_list(const forward_list& other):size_(other.size_){

		ListNode* current = other.head->next;
		head = new ListNode(other.head->value);
		ListNode* iterator{head};

		while(current != nullptr){

				ListNode* newNode = new ListNode(current->value);
				iterator->next = newNode;
				current = current->next;
				iterator = iterator->next;

		}
		std::cout << "Copy constructed" << std::endl;
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

		ListNode* dummy = new ListNode();
		dummy ->next = head;
		ListNode* iterator = dummy;

		for (std::size_t i{}; i < count; ++i){

				ListNode* newNode = new ListNode(value);
				iterator->next = newNode;
				iterator = iterator->next;
		}

		head = dummy->next;

		delete dummy;
}

template <typename U>
void push_front(U&& value){

		ListNode* oldhead = head;
		ListNode* newNode = new ListNode(std::forward<U>(value));
		newNode->next = oldhead;
		head = newNode;
		size_++;
}

constexpr void reverse(){

		ListNode* node{nullptr};
		ListNode* current{head};

		while (current != nullptr){

				ListNode* temp = current->next;

				current->next = node;
				node = current;
				current = temp;
		}

		head = node;

}

constexpr reference front(){

		return head->value;
}


void Print(){

		ListNode* current = head;
		while (current != nullptr){

				std::cout << current->value << std::endl;
				current = current->next;
		}
}

constexpr std::size_t size(){

		return size_;
}

constexpr void popFront(){

		ListNode* old = head->next;
		delete head;
		head = old;
		--size_;
}




};


}





