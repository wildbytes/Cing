// Precompiled headers
#include "Cing-Precompiled.h"

#include "RingBuffer.h"

#include "Cing.h"


namespace Cing
{

	//-------------------------------------------------------------------------------------------------
	RingBuffer::RingBuffer(int size)
		: m_size(size)
		, m_pos(0)
	{
		m_buffer = new float[size];
		Reset();
	}

	//-------------------------------------------------------------------------------------------------
	RingBuffer::~RingBuffer()
	{
		delete[] m_buffer;
	}

	//-------------------------------------------------------------------------------------------------
	void RingBuffer::Reset()
	{
		for (int i = 0; i < m_size; ++i)
			m_buffer[i] = 0.0f;
		m_pos = 0;
	}

	//-------------------------------------------------------------------------------------------------
	void RingBuffer::Add(float value)
	{
		m_buffer[m_pos] = value;
		m_pos = (m_pos + 1) % m_size;
	}

	//-------------------------------------------------------------------------------------------------
	float RingBuffer::GetValue(int x) const
	{
		// first make sure X is not bigger than the size of the buffer, and if it is convert it
		x = x % m_size;

		// now calculate the index in the buffer and return the value
		int index = (x <= m_pos) ? m_pos - x : m_pos + m_size - x;
		return m_buffer[index];
	}

	//-------------------------------------------------------------------------------------------------
	void RingBuffer::SetLast(float val)
	{
		int index = (m_pos > 0) ? m_pos - 1 : m_size - 1;
		m_buffer[index] = val;
	}

	//-------------------------------------------------------------------------------------------------
	float RingBuffer::GetAverage() const
	{
		float avg = 0.0f;
		
		for (int i = 0; i < m_size; ++i)
			avg += m_buffer[i];

		avg /= (float)m_size;

		return avg;
	}

	//-------------------------------------------------------------------------------------------------
	float RingBuffer::GetMax() const
	{
		float maxVal = m_buffer[0];

		for (int i = 1; i < m_size; ++i)
			maxVal = Cing::max(maxVal, m_buffer[i]);

		return maxVal;
	}

	//-------------------------------------------------------------------------------------------------
	float RingBuffer::GetMin() const
	{
		float minVal = m_buffer[0];

		for (int i = 1; i < m_size; ++i)
			minVal = Cing::min(minVal, m_buffer[i]);

		return minVal;
	}

	//-------------------------------------------------------------------------------------------------
	float RingBuffer::GetStandardDeviation(float avg) const
	{
		// Calculated as explained here:
		// http://en.wikipedia.org/wiki/Standard_deviation

		float deviation = 0.0f;

		for (int i = 0; i < m_size; ++i)
		{
			float dist = (m_buffer[i] - avg);
			deviation += dist * dist;
		}

		deviation /= (float)m_size;
		deviation = sqrt(deviation);

		return deviation;
	}

	//-------------------------------------------------------------------------------------------------
	float RingBuffer::GetAverage(int numElems) const
	{
		if (numElems <= 0)
			return 0.0f;
		else if (numElems >= m_size)
			return GetAverage();

		// only do the average of the last X readings
		float avg = 0.0f;
		int index = m_pos;
		for (int i = 0; i < numElems; ++i)
		{
			// go to the previous index
			if (index == 0)
				index = m_size - 1;
			else
				--index;

			// then add the value of that position
			avg += m_buffer[index];
		}

		avg /= (float)numElems;

		return avg;
	}

}
