#include <iostream>
#include <optional>

struct Node {
  int value;
  Node* next;
};

Node* create_node(int value) {
  auto node = new Node;
  node->value = value;
  node->next = nullptr;
  return node;
}

class List {
public:
  List() : head(create_node(-1)) {}
  ~List() {
    auto curr = head;
    while (curr) {
      auto next = curr->next;
      delete curr;
      curr = next;
    }
  }
  List(const List& other) = delete;
  List& operator=(const List& other) = delete;
  List(List&& other) = delete;
  List& operator=(List&& other) = delete;

  void push(int value) {
    auto curr = head;
    while (curr->next) {
      curr = curr->next;
    }
    curr->next = create_node(value);

    ++num_elems;
  }

  std::optional<int> pop() {
    Node* prev = head;
    auto curr = head->next;

    // TODO: adapt interface to std::optional
    if (curr == nullptr) {
      return std::nullopt;
    }

    while (curr && curr->next) {
      prev = curr;
      curr = curr->next;
    }

    int ret = curr->value;
    delete prev->next;
    prev->next = nullptr;

    --num_elems;
    return ret;
  }

  void delete_at(int idx) {
    if (idx < 0)
      return;

    Node* prev = head;
    auto curr = head->next;
    while (curr && idx) {
      prev = curr;
      curr = curr->next;
      --idx;
    }

    // NOTE: mistake made here. it's possible that idx is zero (ie delete at 0)
    // but curr is null
    if (idx || !curr)
      return;

    auto temp = prev->next;
    prev->next = curr->next;
    delete temp;
    --num_elems;
  }

  [[nodiscard]] int size() const { return num_elems; }
  void display() const {
    auto curr = head->next;
    if (!curr) {
      std::cout << "EMPTY LIST\n";
      return;
    }

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
  int num_elems = 0;
};

int main() {

  List list;
  list.push(1);
  list.push(2);
  list.push(3);

  list.display();                   // expected: 1 -> 2 -> 3
  std::cout << list.size() << "\n"; // expected: 3

  list.delete_at(1);
  std::cout << list.size() << "\n"; // expected: 2
  list.display();                   // expected: 1 -> 3

  std::cout << list.pop().value() << "\n"; // expected: 3
  list.display();                          // expected: 1

  list.delete_at(0);
  std::cout << list.size() << "\n";
  list.display();

  list.push(1);
  std::cout << list.pop().value() << "\n";

  // NOTE: chatgpt came up with these test cases. should've included them to
  // figure out bug in delete_at
  List empty;
  std::cout << empty.pop().has_value()
            << "\n";    // should print -1 or fail gracefully
  empty.delete_at(0);   // should do nothing
  empty.display();      // should say EMPTY LIST
  empty.delete_at(100); // should do nothing

  return 0;
}
