/*

Fixed-size ring buffer backed by std::array, no dynamic allocation.
Supports push (overwrites oldest when full), operator[] in insertion order, size() and iteration.
Where: stats/include/stats/ring_buffer.hpp

RingBuffer<float, 3> rb;
rb.push(1); rb.push(2); rb.push(3); rb.push(4);
// contents: 2, 3, 4  ->  rb[0] == 2

*/

#pragma once

#include <array>
#include <cstddef>

namespace stats {

template <typename T, std::size_t N>
class RingBuffer {
    static_assert(N > 0, "RingBuffer capacity must be > 0");

public:
    // Const-Iterator über Indizes 0..size()-1 (Einfügereihenfolge)
    class ConstIterator {
        // TODO: Zeiger auf RingBuffer + Index als Member
        // TODO: operator*, operator++ (prä/post), operator==, operator!=
        // TODO: iterator_traits-Typen (value_type, reference, ...)
    };

    // Überschreibt das älteste Element, wenn der Puffer voll ist
    void push(const T& value) {
        // TODO
    }

    // Index 0 = ältestes Element
    T& operator[](std::size_t i) {
        // TODO
    }

    const T& operator[](std::size_t i) const {
        // TODO
    }

    std::size_t size() const {
        // TODO
    }

    static constexpr std::size_t capacity() {
        // TODO
    }

    bool empty() const {
        // TODO
    }

    bool full() const {
        // TODO
    }

    ConstIterator begin() const {
        // TODO
    }

    ConstIterator end() const {
        // TODO
    }

private:
    std::array<T, N> data_{};
    std::size_t head_ = 0;   // Position für das nächste push
    std::size_t count_ = 0;  // aktuelle Anzahl Elemente (<= N)
};

}  // namespace stats
