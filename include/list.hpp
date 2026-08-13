/*singly linked list*/
#include <iostream>

#include <utility>
namespace ricc{


template <typename T>
class forward_list{

private:

		struct ListNode{

				T value;
				ListNode* next;

				ListNode(const T& value): value(value){}

				ListNode(T& value): value(value){}

				ListNode(T&& value): value(std::move(value)){}
		};

ListNode* head{nullptr};

std::size_t size_{};


public:

forward_list() = default;

template <typename T_FORWARD>
constexpr void pushfront(T_FORWARD&& value){

		ListNode* oldhead = head;

		ListNode* newNode = new ListNode (std::forward<T_FORWARD>(value));

		newNode->next = oldhead;

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


};



};
