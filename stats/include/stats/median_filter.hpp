#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>

namespace stats {

	// Sliding a median over a fiexd odd window. No heap allocation.
	template <typename T, std::size_t N>
	class MedianFilter {
		static_assert(N > 0, "window must not be empty");
		static_assert(N % 2 == 1, "window size must be odd");

	public:
		// Pushes a sample. Returns the median once the window is full,
		// std::nullopt before that.
		std::optional<T> push(T sample) {
			buf_[head_] = sample;
			head_ = (head_ + 1) % N;
			if (count_ < N) {
				++count_;
			}
			return full() ? std::optional<T>{median()} : std::nullopt;
		}

		bool full() const { return count_ == N; }
		std::size_t size() const {  return count_; }
		static constexpr std::size_t window() {  return N; }

		void reset() {
			head_ = 0;
			count_ = 0;
		}

	private:
		T median() const {
			std::array<T, N> tmp = buf_; // order inside the window is irrelevant
			auto mid = tmp.begin() + N / 2;
			std::nth_element(tmp.begin(), mid, tmp.end());
			return *mid;
		}

		std::array<T, N> buf_{};
		std::size_t head_ = 0;
		std::size_t count_ = 0;
	};

} // namespace