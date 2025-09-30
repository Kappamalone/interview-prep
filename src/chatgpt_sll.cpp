#include <iostream>

struct Node {
  int value;
  Node* next;

  Node(int val) : value(val), next(nullptr) {}
};

class SinglyLinkedList {
public:
  SinglyLinkedList() : head(nullptr), tail(nullptr), len(0) {}

  ~SinglyLinkedList() {
    Node* curr = head;
    while (curr) {
      Node* next = curr->next;
      delete curr;
      curr = next;
    }
  }

  void push_back(int value) {
    Node* node = new Node(value);
    if (!head) {
      head = tail = node;
    } else {
      tail->next = node;
      tail = node;
    }
    ++len;
  }

  // Removes the last element and returns its value.
  // Returns false if the list is empty.
  bool pop_back(int& out) {
    if (!head)
      return false;

    if (head == tail) {
      out = head->value;
      delete head;
      head = tail = nullptr;
    } else {
      Node* curr = head;
      while (curr->next != tail) {
        curr = curr->next;
      }
      out = tail->value;
      delete tail;
      tail = curr;
      tail->next = nullptr;
    }
    --len;
    return true;
  }

  int size() const { return len; }

  void print() const {
    Node* curr = head;
    while (curr) {
      std::cout << curr->value;
      if (curr->next)
        std::cout << " -> ";
      curr = curr->next;
    }
    std::cout << "\n";
  }

private:
  Node* head;
  Node* tail;
  int len;
};
