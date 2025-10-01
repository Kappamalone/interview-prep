#include <iostream>
#include <optional>
#include <ostream>
#include <utility>

// > okay, so let's implement a doubly linked list
//
// > first, I'd like to define a node struct that contains our data,
// as well as pointers to the next and previous element
//
// > after that, a linked list class that contains a pointer to a head and tail
// pointer to allow for O(1) push_front, pop_front, push_back, pop_back
//
// > other interface stuff: size, << operator

struct Node {
  int value;
  Node* next;
  Node* prev;
};

class LinkedList {
public:
  LinkedList() : head(nullptr), tail(nullptr), length(0) {}

  // disallow copying to prevent double frees
  // LinkedList(const LinkedList& other) = delete;
  // LinkedList& operator=(const LinkedList& other) = delete;

  // example:
  LinkedList(const LinkedList& other)
      : head(nullptr), tail(nullptr), length(0) {
    auto curr = other.head;
    while (curr) {
      push_back(curr->value);
      ++length;
      curr = curr->next;
    }
  }

  LinkedList& operator=(const LinkedList& other) {
    if (this == &other)
      return *this;

    clear();
    auto curr = other.head;
    while (curr) {
      push_back(curr->value);
      ++length;
      curr = curr->next;
    }
    return *this;
  }

  // nice to have
  LinkedList(LinkedList&& other) noexcept
      : head(std::exchange(other.head, nullptr)),
        tail(std::exchange(other.tail, nullptr)),
        length(std::exchange(other.length, 0)) {}

  LinkedList& operator=(LinkedList&& other) noexcept {
    if (this == &other)
      return *this;

    // MUST CLEAR OWN CONTENTS
    clear();

    head = std::exchange(other.head, nullptr);
    tail = std::exchange(other.tail, nullptr);
    length = std::exchange(other.length, 0);
    return *this;
  }

  // bonus if i have time: memory management
  ~LinkedList() { clear(); }

  void clear() {
    auto curr = head;
    while (curr) {
      auto next = curr->next;
      delete curr;
      curr = next;
    }
    head = tail = nullptr;
    length = 0;
  }

  void push_front(int value) {
    auto node = new Node{value, nullptr, nullptr};
    ++length;

    // nothing in the list
    if (!head && !tail) {
      head = node;
      tail = head;
      return;
    }

    // else push to front
    node->next = head;
    head->prev = node;
    head = node;
  }

  std::optional<int> pop_front() noexcept {
    if (!head && !tail)
      return std::nullopt;

    --length;
    if (head == tail) {
      auto value = head->value;
      delete head;
      head = nullptr;
      tail = nullptr;
      return value;
    } else {
      auto value = head->value;
      head = head->next;
      delete head->prev;
      head->prev = nullptr;
      return value;
    }
  }

  void push_back(int value) {
    auto node = new Node{value, nullptr, nullptr};
    ++length;

    // nothing in the list
    if (!head && !tail) {
      head = node;
      tail = head;
      return;
    }

    // else append to end
    tail->next = node;
    node->prev = tail;
    tail = node;
  }

  std::optional<int> pop_back() noexcept {
    if (!head && !tail)
      return std::nullopt;

    --length;
    if (head == tail) {
      auto val = head->value;
      delete head;
      head = nullptr;
      tail = nullptr;
      return val;
    } else {
      auto val = tail->value;
      tail = tail->prev;
      delete tail->next;
      tail->next = nullptr;
      return val;
    }
  }

  [[nodiscard]] size_t size() const { return length; }

  friend std::ostream& operator<<(std::ostream& os, const LinkedList& ll);

private:
  Node* head;
  Node* tail;
  size_t length;
};

std::ostream& operator<<(std::ostream& os, const LinkedList& ll) {
  os << "list size: " << ll.size() << " | ";

  auto curr = ll.head;
  while (curr) {
    os << curr->value;
    if (curr->next)
      os << " -> ";
    curr = curr->next;
  }

  return os;
}

int main() {

  LinkedList ll;
  ll.push_back(1);
  ll.push_back(2);
  ll.push_front(3);

  std::cout << ll << "\n"; // 3 -> 1 -> 2
  auto value = ll.pop_front();
  if (value.has_value())
    std::cout << "pop front: " << value.value() << "\n"; // 3

  value = ll.pop_back();
  if (value.has_value())
    std::cout << "pop back: " << value.value() << "\n"; // 2

  std::cout << ll << "\n";

  return 0;
}

// reflection:
// 1) increment length after allocation to provide some level of exception
// safety
// 2) disable copying to show understanding of ownership
//  -> if time permits write move functions
// 3) nits:
//  - size_t for length
//  - noexcept on size and pop (no allocation)
