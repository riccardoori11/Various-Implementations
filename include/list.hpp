#include <utility>
#include <iostream>

namespace ricc{

template <typename T>
class forward_list{

private:

struct ListNode{

		T value;
		ListNode* next;

		ListNode(const T& value): value(value)
		{
		}

		ListNode(T&& value): value(std::move(value))
		{
		}

};

ListNode* head{nullptr};
std::size_t size_{};

void swap(forward_list& temp){

		std::swap(head,temp.head);
		std::swap(size_,temp.size_);
}

public:

forward_list() = default;	

~forward_list(){

		if (head != nullptr){

				for (ListNode* current{head}; current != nullptr;){

						std::cout << "Deleted" << std::endl;
						auto next = current->next;
						delete current;
						current = next;

				}

		}
		size_ = 0;

}

/*
 * Copy constructor
 * */

forward_list(const forward_list& other):size_(other.size_){

		
		ListNode* current = other.head->next;
		head = new ListNode(other.head->value);
		ListNode* iterator{head};
		while (current != nullptr){
				ListNode* node = new ListNode(current->value);
				iterator->next = node;
				current = current->next;
				iterator = iterator->next;
		}
		std::cout << "Copy constructor" << std::endl;

}

/*
 * Move constructor
 * */
forward_list (forward_list&& other) noexcept: size_(std::exchange(other.size_,0)),head(std::exchange(other.head,nullptr)){

		std::cout << "Move constructor" << std::endl;
}

/* Copy assignment */
forward_list& operator = (const forward_list& other){

		std::cout << "Copy assignment" << std::endl;
		if (this == &other){

				return *this;
		}

		forward_list temp(other);
		swap(temp);

		return *this;
}

/* Move assingment */
forward_list& operator= (forward_list&& other) noexcept {

		std::cout << "Move assignment" << std::endl;

		if (this == &other){

				return *this;
		}

		forward_list temp(std::move(other));

		swap(temp);

		return *this;
}

void reverse() noexcept {

		ListNode* node{nullptr};

		while (head != nullptr){
				
				ListNode* old = head->next;
				head->next = node;
				node = head;
				head = old;
		}
		head = node;

}

bool empty() const noexcept {

		return size_ == 0;
}

template<typename U>
void push_front(U&& value){

		ListNode* oldhead = head;

		ListNode* newNode = new ListNode(std::forward<U>(value));

		newNode -> next = oldhead;

		++size_;
		head = newNode;
}

constexpr std::size_t size(){

		return size_;
}

void print(){

		ListNode* current = head;

		while (current != nullptr){

				std::cout << current->value << std::endl;

				current = current->next;
		}
		
}

constexpr T& front(){

		return head->value;
}

};


};



