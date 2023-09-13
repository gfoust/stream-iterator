// Copyright 2023, Gabriel Foust, All rights reserved
#pragma once
#include <cstdint>
#include <istream>
#include <iterator>
#include <type_traits>
#include <utility>
#include <variant>
//#include <concepts>

namespace cgf {

  /*========================================================
   * IstreamIterator
   */
  template<typename T>
  class IstreamIterator {
  public:
    /*------------------------------------------------------
     * Iterator traits
     */
    using difference_type = ptrdiff_t;
    using value_type = std::remove_cv_t<T>;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::input_iterator_tag;

    /*------------------------------------------------------
     * Strong types to represent stopping conditions
     */
    struct Eof {
      bool operator ==(Eof) const {
        return true;
      }
      bool operator !=(Eof) const {
        return true;
      }
    };

    struct Count {
      size_t value;

      bool operator ==(Count rhs) const {
        return value == rhs.value;
      }
      bool operator !=(Count rhs) const {
        return value != rhs.value;
      }
    };

    struct Sentinel {
      value_type value;

      bool operator ==(Sentinel const& rhs) const {
        return value == rhs.value;
      }
      bool operator !=(Sentinel const& rhs) const {
        return value != rhs.value;
      }
    };

  private:
    /*------------------------------------------------------
     * Implementation
     */

    // State of an iterator being used for input
    struct InputState {
      std::istream* in;
      size_t count;
      mutable bool valid;
      mutable value_type value;
    };

    // Possible object states
    std::variant<Eof, Count, Sentinel, InputState> _impl;

    // Commit to reading the next value
    static
    void softCommit(InputState const& state) {
      if (!state.valid) {
        *state.in >> state.value;
        state.valid = true;
      }
    }

    // Commit to reading the next value and throw exception on failure
    static
    reference hardCommit(InputState const& state) {
      softCommit(state);
      if (!*state.in) {
        throw std::istream::failure("input failure");
      }
      return state.value;
    }

    /*------------------------------------------------------
     * Function object to compare all possible combinations
     * of iterator and stopping conditions
     */
    struct Equivalent {

      bool operator ()(InputState const& lhs, InputState const& rhs) {
        return lhs.in == rhs.in;
      }

      bool operator ()(InputState const& lhs, Sentinel const& rhs) {
        return hardCommit(lhs) == rhs.value;
      }

      bool operator ()(Sentinel const& lhs, InputState const& rhs) {
        return lhs.value == hardCommit(rhs);
      }

      bool operator ()(InputState const& lhs, Count rhs) {
        return lhs.count == rhs.value;
      }

      bool operator ()(Count lhs, InputState const& rhs) {
        return lhs.value == rhs.count;
      }

      bool operator ()(InputState const& lhs, Eof) {
        softCommit(lhs);
        return lhs.in->eof();
      }

      bool operator ()(Eof, InputState const& rhs) {
        softCommit(rhs);
        return rhs.in->eof();
      }

      template<typename U>
      bool operator ()(U const& lhs, U const& rhs) {
        return lhs == rhs;
      }

      template<typename U, typename V>
      bool operator ()(U const& lhs, V const& rhs) {
        return false;
      }
    };

  public:
    /*------------------------------------------------------
     * Constructors
     */

    explicit
    IstreamIterator(std::istream& in) : _impl{ InputState{ &in, 0, false, {} } } {
    }

    explicit
    IstreamIterator(Count count) : _impl{ count } {
    }

    explicit
    IstreamIterator(Sentinel value) : _impl{ std::move(value) } {
    }

    explicit
    IstreamIterator(Eof eof = {}) : _impl{ eof } {
    }

    /*------------------------------------------------------
     * Iterator operators
     */

    reference operator *() const {
      return hardCommit(std::get<InputState>(_impl));
    }

    pointer operator ->() const {
      return &hardCommit(std::get<InputState>(_impl));
    }

    IstreamIterator& operator ++() {
      auto& state = std::get<InputState>(_impl);
      hardCommit(state);
      ++state.count;
      state.valid = false;
      return *this;
    }

    IstreamIterator operator ++(int) {
      auto& state = std::get<InputState>(_impl);
      hardCommit(state);
      auto copy = std::move(*this);
      ++state.count;
      state.valid = false;
      return copy;
    }

    bool operator ==(IstreamIterator const& rhs) {
      return std::visit(Equivalent{}, _impl, rhs._impl);
    }

    bool operator !=(IstreamIterator const& rhs) {
      return !std::visit(Equivalent{}, _impl, rhs._impl);
    }

    /*------------------------------------------------------
     * Convenience factory methods
     */

    static
    IstreamIterator untilCount(size_t count) {
      return IstreamIterator(Count{ count });
    }

    static
    IstreamIterator untilSentinel(value_type value) {
      return IstreamIterator(Sentinel{ std::move(value) });
    }

    static
    IstreamIterator untilEof() {
      return IstreamIterator(Eof{});
    }
  };

  /*========================================================
   * Convenience factory functions
   */

  template<typename T> inline
  IstreamIterator<T> scan(std::istream& in) {
    return IstreamIterator<T>(in);
  }

  template<typename T> inline
  IstreamIterator<T> untilCount(size_t count) {
    return IstreamIterator<T>(typename IstreamIterator<T>::Count{ count });
  }

  template<typename T> inline
  IstreamIterator<T> untilSentinel(T value) {
    return IstreamIterator<T>(typename IstreamIterator<T>::Sentinel{ std::move(value) });
  }

  template<typename T> inline
  IstreamIterator<T> untilEof() {
    return IstreamIterator<T>(typename IstreamIterator<T>::Eof{});
  }

  //static_assert(std::input_iterator<StreamIterator<double>>);

}