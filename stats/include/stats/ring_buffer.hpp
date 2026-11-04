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
#include <iterator>

namespace stats {

template <typename T, std::size_t N>
class RingBuffer {
	static_assert(N > 0, "RingBuffer capacity must be > 0");

public:
	// Const-Iterator over indices 0..size()-1 (inputorder)
	class ConstIterator {
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type        = T;
		using difference_type   = std::ptrdiff_t;
		using pointer           = const T*;
		using reference         = const T&;

		ConstIterator() = default;
		ConstIterator(const RingBuffer* rb, std::size_t index)
			: rb_(rb), index_(index) {}

		reference operator*() const { return (*rb_)[index_]; }
		pointer operator->() const { return &(*rb_)[index_]; }

		ConstIterator& operator++() {
			++index_;
			return *this;
		}

		ConstIterator operator++(int) {
			ConstIterator tmp = *this;
			++index_;
			return tmp;
		}

		bool operator==(const ConstIterator& other) const {
			return rb_ == other.rb_ && index_ == other.index_;
		}

		bool operator!=(const ConstIterator& other) const {
			return !(*this == other);
		}
	private:
		const RingBuffer* rb_ = nullptr;
		std::size_t index_ = 0;
	};

	// overwrite oldest element when buffer full
	void push(const T& value) {
		data_[head_] = value;		// write at the next free slot (or over the oldest)
		head_ = (head_ + 1) % N;	// advance, wrap to 0 after N-1
		if (count_ < N) {
			++count_;				// grow until full, then stay at N
		}
	}

	// Index 0 = oldest element
	T& operator[](std::size_t i) {
		return data_[(head_ + N - count_ + i) % N];
	}

	const T& operator[](std::size_t i) const {
		return data_[(head_ + N - count_ + i) % N];
	}

	std::size_t size() const {
		return count_;
	}

	static constexpr std::size_t capacity() {
		return N;
	}

	bool empty() const {
		return count_ == 0;
	}

	bool full() const {
		return count_ == N;
	}

	ConstIterator begin() const {
		return ConstIterator(this, 0);
	}

	ConstIterator end() const {
		return ConstIterator(this, count_);
	}

private:
	std::array<T, N> data_{};
	std::size_t head_ = 0;   // Position for next push
	std::size_t count_ = 0;  // actual number of elements(<= N)
};

}  // namespace stats
