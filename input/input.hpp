// Copyright 2023, Gabriel Foust, All rights reserved
#pragma once
#include <istream>
#include <variant>
#include <utility>

namespace input {

  /*========================================================
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

  template<typename T>
  struct Sentinel {
    T value;

    bool operator ==(Sentinel const& rhs) const {
      return value == rhs.value;
    }
    bool operator !=(Sentinel const& rhs) const {
      return value != rhs.value;
    }
  };

  /*========================================================
   * StreamIterator
   */
  template<typename T>
  class StreamIterator {

    // State of an iterator that is used for input
    struct InputState {
      std::istream* in;
      size_t count;
      mutable bool valid;
      mutable T value;
    };

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
    T& commit(InputState const& state) {
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

      bool operator ()(InputState const& lhs, Sentinel<T> const& rhs) {
        return commit(lhs) == rhs.value;
      }

      bool operator ()(Sentinel<T> const& lhs, InputState const& rhs) {
        return lhs.value == commit(rhs);
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
    /* end of Equivalent
     *----------------------------------------------------*/

    // This one object can represent either a stopping iterator or an input iterator
    std::variant<Eof, Count, Sentinel<T>, InputState> _impl;

  public:

    /*------------------------------------------------------
     * Constructors
     */

    explicit
    StreamIterator(std::istream& in) : _impl{ InputState{ &in, 0, false, {} } } {
    }

    explicit
    StreamIterator(Count count) : _impl{ count } {
    }

    explicit
    StreamIterator(Sentinel<T> value) : _impl{ std::move(value) } {
    }

    explicit
    StreamIterator(Eof eof = {}) : _impl{ eof } {
    }

    /*------------------------------------------------------
     * Iterator operators
     */

    T& operator *() const {
      return commit(std::get<InputState>(_impl));
    }

    T* operator ->() const {
      return &commit(std::get<InputState>(_impl));
    }

    StreamIterator& operator ++() {
      auto& state = std::get<InputState>(_impl);
      softCommit(state);
      ++state.count;
      state.valid = false;
      return *this;
    }

    StreamIterator operator ++(int) {
      auto& state = std::get<InputState>(_impl);
      softCommit(state);
      auto copy = std::move(*this);
      ++state.count;
      state.valid = false;
      return copy;
    }

    bool operator ==(StreamIterator const& rhs) {
      return std::visit(Equivalent{}, _impl, rhs._impl);
    }

    bool operator !=(StreamIterator const& rhs) {
      return !std::visit(Equivalent{}, _impl, rhs._impl);
    }

    /*------------------------------------------------------
     * Convenience factory methods
     */

    static
    StreamIterator untilCount(size_t count) {
      return StreamIterator(Count{ count });
    }

    static
    StreamIterator untilSentinel(T value) {
      return StreamIterator(Sentinel<T>{ std::move(value) });
    }

    static
    StreamIterator untilEof() {
      return StreamIterator(Eof{});
    }
  };

  /*========================================================
   * Convenience factory functions
   */

  template<typename T> inline
  StreamIterator<T> scan(std::istream& in) {
    return StreamIterator<T>(in);
  }

  template<typename T> inline
  StreamIterator<T> untilCount(size_t count) {
    return StreamIterator<T>(Count{ count });
  }

  template<typename T> inline
  StreamIterator<T> untilSentinel(T value) {
    return StreamIterator<T>(Sentinel<T>{ std::move(value) });
  }

  template<typename T> inline
  StreamIterator<T> untilEof() {
    return StreamIterator<T>(Eof{});
  }

}