#include <iostream>
#include <list>
#include <map>
#include <unordered_map>
#include <utility>

// second time writing an order book to keep it fresh in my memory

// first: explanation
//
// so our order book needs to support add order, delete order, modify order
// orders can be one of two sides, and each side is ordered by price.
// to support fast insertion, deletion and modification, let's use hashmaps as a
// tertiary structure.
//
// so each book has a bids and asks std::map<int, Level>, a hashmap from id to
// order and a hashmap from price to level (optional, for speed)

// second, interface design
// third, tests <---- make sure to do this step first for practical questions!!
// fourth, polish

using OrderId = uint64_t;
using Price = uint64_t;
struct Order {
  OrderId id;
  Price price;
  uint64_t qty;
  bool is_bid;
};

using Level = std::list<Order>;

struct OrderPtr {
  std::map<Price, Level>::iterator level_it;
  Level::iterator order_it;
};

class Book {
public:
  Book() {}

  bool add_order(OrderId id, Price price, uint64_t qty, bool is_bid) {
    if (orders.find(id) != orders.end())
      return false;

    auto& side = is_bid ? bids : asks;
    auto [level_it, _] = side.try_emplace(price);

    // MAJOR MISTAKE:
    // this HAS to be a reference, since level_it->second IS the doubly linked
    // list. in general, prefer to use the raw iterator and don't reassign to
    // variables
    auto& order_it = level_it->second;
    order_it.emplace_back(id, price, qty, is_bid);
    orders.emplace(std::piecewise_construct, std::forward_as_tuple(id),
                   std::forward_as_tuple(level_it, prev(order_it.end())));

    return true;
  }

  bool delete_order(OrderId id) {
    auto it = orders.find(id);
    if (it == orders.end())
      return false;

    bool is_bid = it->second.order_it->is_bid;
    it->second.level_it->second.erase(it->second.order_it);
    if (it->second.level_it->second.empty()) {
      is_bid ? bids.erase(it->second.level_it)
             : asks.erase(it->second.level_it);
    }

    orders.erase(it);
    return true;
  }

  bool modify_order(OrderId id, Price new_price, uint64_t new_qty) {
    auto it = orders.find(id);
    if (it == orders.end() || new_qty == 0) {
      return false;
    }

    auto old_price = it->second.order_it->price;
    it->second.order_it->qty = new_qty;

    if (new_price != old_price) {
      auto is_bid = it->second.order_it->is_bid;
      delete_order(id);
      add_order(id, new_price, new_qty, is_bid);
      return true;
    }

    return true;
  }

  [[nodiscard]] std::pair<Price, Price> get_bbo() const noexcept {
    Price best_bid = bids.empty() ? 0 : bids.rbegin()->first;
    Price best_ask = asks.empty() ? 0 : asks.begin()->first;
    return {best_bid, best_ask};
  }

  friend std::ostream& operator<<(std::ostream& os, const Book& book) {
    os << "====================\n";
    for (auto level_it = book.asks.rbegin(); level_it != book.asks.rend();
         ++level_it) {
      os << "ask: $" << level_it->first << " | ";
      for (auto order_it = level_it->second.begin();
           order_it != level_it->second.end();) {
        os << "{ id: " << order_it->id << " , qty: " << order_it->qty << " }";
        if (++order_it != level_it->second.end())
          os << " -> ";
      }
      os << "\n";
    }

    os << "\n";

    for (auto level_it = book.bids.rbegin(); level_it != book.bids.rend();
         ++level_it) {
      os << "bid: $" << level_it->first << " | ";
      for (auto order_it = level_it->second.begin();
           order_it != level_it->second.end();) {
        os << "{ id: " << order_it->id << " , qty: " << order_it->qty << " }";
        if (++order_it != level_it->second.end())
          os << " -> ";
      }
      os << "\n";
    }
    os << "====================";
    return os;
  }

private:
  std::map<Price, Level> bids; // best bid = end of map
  std::map<Price, Level> asks; // best ask = start of map
  std::unordered_map<OrderId, OrderPtr> orders;
};

int main() {

  Book book{};

  // output: "0 0"
  std::cout << book.get_bbo().first << " " << book.get_bbo().second << "\n";

  std::cout << std::boolalpha;
  // output: true
  std::cout << book.add_order(0, 10, 10, true) << "\n";
  // output: false
  std::cout << book.add_order(0, 10, 10, true) << "\n";
  // output: true
  std::cout << book.add_order(1, 15, 10, false) << "\n";
  // output: true
  std::cout << book.add_order(2, 16, 10, false) << "\n";

  std::cout << std::noboolalpha;

  // output: "10 15"
  std::cout << book.get_bbo().first << " " << book.get_bbo().second << "\n";

  book.delete_order(1);
  // output: "10 16"
  std::cout << book.get_bbo().first << " " << book.get_bbo().second << "\n";

  book.modify_order(2, 17, 20);
  // output: "10 17"
  std::cout << book.get_bbo().first << " " << book.get_bbo().second << "\n";

  book = Book{};
  book.add_order(1, 10, 100, true);
  book.modify_order(1, 12, 200);
  book.add_order(2, 20, 100, false);

  std::cout << book << "\n";

  // output: 12 20
  std::cout << book.get_bbo().first << " " << book.get_bbo().second << "\n";

  book.delete_order(1);
  // output: -1 20
  std::cout << book.get_bbo().first << " " << book.get_bbo().second << "\n";

  book.delete_order(2);
  // output: -1 -1
  std::cout << book.get_bbo().first << " " << book.get_bbo().second << "\n";

  book.add_order(3, 10, 10, true);
  book.add_order(4, 9, 8, true);
  book.add_order(5, 5, 1, true);
  book.add_order(6, 5, 10, true);

  book.add_order(7, 15, 5, false);
  book.add_order(8, 20, 10, false);

  std::cout << book << "\n";
  // output: 10 15
  std::cout << book.get_bbo().first << " " << book.get_bbo().second << "\n";

  return 0;
}
