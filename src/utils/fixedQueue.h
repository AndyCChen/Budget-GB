#pragma once

#include <array>
#include <cstdint>
#include <cassert>
#include <cmath>

namespace Utils
{

// Simple and small fixed queue, emphasis on small
template <typename T, uint8_t n>
class FixedQueue
{
  private:
	std::array<T, n> m_queue{};

	uint8_t m_tail = 0;
	uint8_t m_head = 0;
	uint8_t m_size = 0;

  public:
	void push(const T &value)
	{
		assert((m_size != 10) && "Exceeded queue size!");
		m_queue[m_head] = value;

		m_tail = (m_tail + 1) % m_queue.size();
		++m_size;
	}

	T& get()
	{
		assert((m_size != 0) && "Reading from empty queue!");
		return m_queue[m_head];
	}

	void pop()
	{
		assert((m_size != 0) && "Popping from empty queue!");
		m_head = (m_head + 1) % m_queue.size();
		--m_size;
	}

	bool isEmpty() const
	{
		return m_size == 0;
	}

	void clear()
	{
		m_tail = 0;
		m_head = 0;
		m_size = 0;
	}

	uint8_t size() const
	{
		return m_size;
	}
};

}; // namespace Utils