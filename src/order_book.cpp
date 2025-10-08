#include <cstdint>
#include <iostream>
#include <list>
#include <map>
#include <unordered_map>

// an order book is required to support the following operations
// - add_order(id, side, price, qty)
// - delete_order(id)
// - modify_order(id, new_price, new_qty)
//
// furthermore, we are required to implement price-time priority
// aka best price trades, and first come first serve

// ideally we'd get all three operations down to just O(1) using a *lot* of
// pointers. if we're implementing these functions using STL containers and
// iterators, the best we can achieve for add order is log n, delete order is
// average case O(1) (unless we delete the level, in which case it's log n) and
// modify order is O(1) or O(log n) depending on if the price is different
// (since we delete + insert)
//
// note from future uzman: you can get O(1) average add by just having a map of
// price to level iterator!!
//
// we have one order book per symbol. each order book maintains two std::map's,
// from a given price level to a structure that holds doubly linked lists
// representing individual orders
//
// why a doubly linked list? because we'd want to have iterator stability to
// support deletion. deletion is accomplished using an unordered_map from order
// id to an OrderPtr struct - containing an *iterator* to a node in our doubly
// linked list as well as an iterator the containing price level
//
// and with that, we should have a pretty robust orderbook that definitely has
// some room for improvement, but suffices for interview settings

using OrderId = uint64_t;
using Price = uint32_t;
struct Order {
  OrderId id;
  bool is_bid;
  Price price;
  uint32_t qty;
};

// could make this a struct and track per level aggregrates such as total qty
// and num orders
using Level = std::list<Order>;

struct OrderPtr {
  std::map<Price, Level>::iterator level_it;
  std::list<Order>::iterator order_it;
  bool is_bid;
};

class OrderBook {
public:
  // rule of zero, no management of any resources woo!
  OrderBook() {}

  void add_order(OrderId id, bool is_bid, Price price, uint32_t qty) {
    if (orders.find(id) != orders.end()) {
      return;
    }

    auto order = Order{.id = id, .is_bid = is_bid, .price = price, .qty = qty};

    auto& side = is_bid ? bids : asks;
    auto [level, _] = side.try_emplace(price);
    level->second.push_back(order);
    orders[id] = OrderPtr{.level_it = level,
                          .order_it = std::prev(level->second.end()),
                          .is_bid = is_bid};
  }

  void delete_order(OrderId id) {
    auto it = orders.find(id);
    if (it == orders.end()) {
      return;
    }

    auto& level_it = it->second.level_it;
    auto& order_it = it->second.order_it;
    auto& is_bid = it->second.is_bid;
    level_it->second.erase(order_it);
    if (level_it->second.empty()) {
      is_bid ? bids.erase(level_it) : asks.erase(level_it);
    }
    orders.erase(it);
  }

  void modify_order(OrderId id, Price new_price, uint32_t new_qty) {
    // let's say for example's sake that if price is the same, then the order
    // maintains priority, else it's treated as a new order (ie delete + add)

    auto it = orders.find(id);
    if (it == orders.end()) {
      return;
    }

    auto& order_it = it->second.order_it;
    // NIT: new_qty = 0 is essentially a delete
    if (new_qty == 0) {
      delete_order(order_it->id);
      return;
    }

    if (order_it->qty != new_qty) {
      order_it->qty = new_qty;
    }

    if (it->second.order_it->price != new_price) {
      Order order{.id = order_it->id,
                  .is_bid = order_it->is_bid,
                  .price = new_price,
                  .qty = order_it->qty};
      delete_order(order.id);
      add_order(order.id, order.is_bid, order.price, order.qty);
    }
  }

  [[nodiscard]] std::pair<int, int> get_bbo() const noexcept {
    auto best_bid = bids.empty() ? -1 : bids.rbegin()->first;
    auto best_ask = asks.empty() ? -1 : asks.begin()->first;
    return {best_bid, best_ask};
  }

  friend std::ostream& operator<<(std::ostream& os, const OrderBook& book) {
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
  // note, we need to treat any ordered operation on asks as reversed
  std::map<Price, Level> bids;
  std::map<Price, Level> asks;

  std::unordered_map<OrderId, OrderPtr> orders;
};

int main() {

  OrderBook book;

  book.add_order(1, true, 10, 100);
  book.modify_order(1, 12, 200);
  book.add_order(2, false, 20, 100);

  std::cout << book << "\n";

  // output: 12 20
  std::cout << book.get_bbo().first << " " << book.get_bbo().second << "\n";

  book.delete_order(1);
  // output: -1 20
  std::cout << book.get_bbo().first << " " << book.get_bbo().second << "\n";

  book.delete_order(2);
  // output: -1 -1
  std::cout << book.get_bbo().first << " " << book.get_bbo().second << "\n";

  book.add_order(3, true, 10, 10);
  book.add_order(4, true, 9, 8);
  book.add_order(5, true, 5, 1);
  book.add_order(6, true, 5, 10);

  book.add_order(7, false, 15, 5);
  book.add_order(8, false, 20, 10);

  std::cout << book << "\n";
  // output: 10 15
  std::cout << book.get_bbo().first << " " << book.get_bbo().second << "\n";

  return 0;
}

// general reflection:
// - bbo was flipped! should've written better tests with multiple orders to
// test order was correct
// - add delete modify could return some result type
// - modify to zero qty should be treated as delete or handled explicitly
//
// can improve performance of std::list by instead using an intrusive linked
// list + a custom allocator so that pointers are contiguous

/* chatgpt answer

std::list is simple and correct, but its scattered allocations kill cache
locality. In production I’d replace it with an intrusive list whose nodes come
from a pre-allocated pool so that all order nodes live contiguously in
memory - same O(1) semantics, vastly better performance.

*/
