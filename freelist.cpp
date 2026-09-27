#include <cstdio>
#include <iostream>

/*
 * small demo to showcase how a freelist data structure is used to reuse memory
 *
 * */

struct Node{

		int value;
		Node* next;

};

void PrintList(Node* n){

		while (n != nullptr){

				std::cout << n->value << std::endl;
				n = n->next;
		}

}

int main(){

		Node storage[3] = {{1, nullptr}, {2,nullptr},{3,nullptr}};
		storage[0].next = &storage[1];
		storage[1].next = &storage[2];


		Node* free_head = nullptr;
		Node* head = &storage[0];

		std::cout << '\n';

		PrintList(head);

		Node* removed = head->next;
		head -> next = removed->next;

		removed->next = free_head;
		free_head = removed;

		std::cout << '\n';

		PrintList(head);

		Node* reused = free_head;
		free_head = nullptr;

		reused->value = 4;
		reused->next = head->next;
		head->next = reused;

		std::cout << '\n';

		PrintList(head);

		return 0;
}
