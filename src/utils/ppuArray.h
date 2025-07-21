#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <algorithm>

namespace Utils
{

template <typename T, uint32_t n>
class PPUArray
{
  private:
	std::array<T, n> m_array{};

	uint32_t m_head = 0;
	uint32_t m_tail = 0;

  public:
	// Push item to end of array, only n pushes are allowed before needing to call clear()
	// to reset array
	void push(const T &value)
	{
		assert((m_tail != m_array.size()) && "Exceeded array size!");
		m_array[m_tail++] = value;
	}

	// remove item from array head by simply incrementing the head, no shifting
	void pop()
	{
		assert((m_tail != 0) && "Popping from empty array!");
		++m_head;
	}

	// retrieve item from array head
	T &peak()
	{
		assert((m_head - m_tail != 0) && "Reading from empty array!");
		return m_array[m_head];
	}

	bool isEmpty() const
	{
		return m_tail - m_head == 0;
	}

	void clear()
	{
		m_head = 0;
		m_tail = 0;
	}

	// return max capacity of array
	constexpr std::size_t size() const
	{
		return m_array.size();
	}

	// return number of items currenty in array
	uint32_t length() const
	{
		return m_tail - m_head;
	}

	T &operator[](std::size_t index)
	{
		return m_array[index];
	}

	const T &operator[](std::size_t index) const
	{
		return m_array[index];
	}

	void fill(const T &value)
	{
		m_array.fill(value);
	}

	// removes items in range of head and tail by shifting items towards head. Tail is decremented afterwards
	// if the size changed
	template <typename Predicate>
	void remove(Predicate pr)
	{
		auto newEnd = std::remove_if(m_array.begin() + m_head, m_array.begin() + m_tail, pr);
		auto dist = std::distance(m_array.begin() + m_head, newEnd);
		m_tail -= static_cast<uint32_t>(length() - dist);
	}
};

} // namespace Utils