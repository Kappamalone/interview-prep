#include <iostream>
#include <optional>
#include <ostream>
#include <utility>

// our circular buffer will use two pointers, a head and a tail
// to control where we can insert and remove elements from
//
// we'll reserve capacity + 1 in our buffer to distinguish between an
// empty and a full circular buffer

template <typename T>
class CircularBuffer {
public:
  CircularBuffer(size_t capacity)
      : read_pos(0), write_pos(0), buffer(new T[capacity + 1]),
        capacity(capacity + 1) {}

  ~CircularBuffer() { delete[] buffer; }

  // managing raw pointers, no copy
  CircularBuffer(const CircularBuffer& other) = delete;
  CircularBuffer& operator=(const CircularBuffer& other) = delete;

  CircularBuffer(CircularBuffer&& other) noexcept
      : read_pos(std::exchange(other.read_pos, 0)),
        write_pos(std::exchange(other.write_pos, 0)),
        buffer(std::exchange(other.buffer, nullptr)),
        capacity(std::exchange(other.capacity, 0)) {}

  CircularBuffer& operator=(CircularBuffer&& other) noexcept {
    if (this == &other)
      return *this;

    if (buffer)
      delete[] buffer;
    read_pos = std::exchange(other.read_pos, 0);
    write_pos = std::exchange(other.write_pos, 0);
    buffer = std::exchange(other.buffer, nullptr);
    capacity = std::exchange(other.capacity, 0);

    return *this;
  }

  // empty: head = tail
  // full: tail + 1 = head

  bool push(T value) {
    if (increment(write_pos) == read_pos) {
      return false;
    }

    buffer[write_pos] = value;
    write_pos = increment(write_pos);
    return true;
  };

  std::optional<T> pop() {
    if (read_pos == write_pos) {
      return std::nullopt;
    }

    // nit: move here for eg if buffer is non-trivial type
    auto ret = std::make_optional(std::move(buffer[read_pos]));
    read_pos = increment(read_pos);
    return ret;
  }

  [[nodiscard]] size_t size() const {
    if (read_pos <= write_pos) {
      return write_pos - read_pos;
    } else {
      // head > tail
      // so head... end of array
      // then start of array to tail
      return capacity - read_pos + write_pos;
    }
  }

  friend std::ostream& operator<<(std::ostream& os, const CircularBuffer& cb) {
    size_t curr = cb.read_pos;
    while (curr != cb.write_pos) {
      std::cout << cb.buffer[curr] << " ";
      curr = cb.increment(curr);
    }
    return os;
  }

private:
  size_t increment(size_t pos) const { return (pos + 1) % capacity; }

  size_t read_pos;
  size_t write_pos;
  T* buffer;
  size_t capacity;
};

int main() {
  CircularBuffer<int> cb(5);

  cb.push(1);
  cb.push(2);
  cb.push(3);
  cb.push(4);
  cb.push(5);

  std::cout << (cb.push(6) ? "SUCCEED" : "FAIL") << "\n"; // FAIL
  std::cout << cb << "\n";                                // 1 2 3 4 5

  if (auto val = cb.pop()) {
    std::cout << *val << "\n"; // 1
  }
  if (auto val = cb.pop()) {
    std::cout << *val << "\n"; // 2
  }
  if (auto val = cb.pop()) {
    std::cout << *val << "\n"; // 3
  }
  if (auto val = cb.pop()) {
    std::cout << *val << "\n"; // 4
  }

  cb.push(6);
  cb.push(7);
  std::cout << cb << "\n";
  std::cout << cb.size() << "\n";

  return 0;
}

// Reflection:
// even the simple interface was tricky to implement correctly the first time
// general pointers to remember for circular buffers:
// - two pointers: read_pos and write_pos (don't use head and tail it's
// confusing)
// - empty is when they are equal
// - to distinguish empty from full, we set capacity equal to cap+1
// - therefore full is now when increment(write_pos) == read_pos
