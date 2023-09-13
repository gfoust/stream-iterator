#include <iostream>
#include <string>
#include <vector>
#include "input.hpp"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;

using cgf::scan;
using cgf::untilCount;
using cgf::untilSentinel;
using cgf::untilEof;

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
    cgf::IstreamIterator<string> instr(cin);

    // read three strings
    cout << sum1(instr, instr.untilCount(3)) << '\n';

    // read strings until "a"
    cout << sum1(instr, instr.untilSentinel("a")) << '\n';


    // alternative way to define a variable
    auto indub = scan<double>(cin);

    // read doubles until end of file
    cout << sum1(indub, indub.untilEof()) << '\n';
  }
  catch (std::exception& err) {
    // throws std::istream::failure if input fails
    // throws std::bad_variant_access if you try to increment or dereference a stopping iterator
    cerr << err.what() << endl;
  }
}
