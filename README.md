start by talking through the general thought process
  - explain everything first

simple implementation first that works

define interface, think about expected output, write down simple examples in
main and THEN implement

next steps:
  - memory management + destructor
  - ownership
    - move semantics
  - const correctness, const member functions
  - explicit constructors, [[nodiscard]]

general:
  - don't use smart ptrs for simplicity sake -> though mention that you would
  use them in production code
  - implement friend std::ostream& operator<<(std::ostream &os, const Type& type); for
  easier debugging
