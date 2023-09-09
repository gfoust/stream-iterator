#include <iostream>
#include <string>
#include <vector>
#include "input.hpp"
using namespace std;
using input::scan;
using input::untilCount;
using input::untilSentinel;
using input::untilEof;

// Adds all elements between two iterators (assumes at least 1 element)
template<typename ItT>
auto sum1(ItT begin, ItT end) {
  auto total = *begin++;
  for (auto p = begin; p != end; ++p) {
    total += *p;
  }
  return total;
}

int main() {
  // observe: function works with vector iterators
  std::vector<int> numbers{ 2, 4, 6, 8 };
  cout << sum1(numbers.begin(), numbers.end()) << '\n';

  try {
    // read three integers
    cout << sum1(scan<int>(cin), untilCount<int>(3)) << '\n';

    // read integers until -1
    cout << sum1(scan<int>(cin), untilSentinel<int>(-1)) << '\n';


    // reusable variable
    input::StreamIterator<string> instr(cin);

    // read three strings
    cout << sum1(instr, instr.untilCount(3)) << '\n';

    // read strings until "a"
    cout << sum1(instr, instr.untilSentinel("a")) << '\n';


    // read doubles until end of file
    cout << sum1(scan<double>(cin), untilEof<double>()) << '\n';
  }
  catch (std::exception& err) {
    // throws std::istream::failure if input fails *and* you try to use the value
    // throws std::bad_variant_access if you try to increment or dereference a stopping iterator
    cerr << err.what() << endl;
  }
}
