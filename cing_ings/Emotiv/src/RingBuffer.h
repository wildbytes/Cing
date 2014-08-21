#ifndef _EMOTIV_RINGBUFFER_H__
#define _EMOTIV_RINGBUFFER_H__

namespace Cing
{
	//-------------------------------------------------------------------------------------------------
	// Simple circular buffer of float values used in to store the headset readings
	class RingBuffer
	{
	public:
		RingBuffer(int size);
		~RingBuffer();

		// Resets the buffer to all 0's
		void Reset();

		// Adds the given value at the end of the buffer.
		void Add(float value);

		// Returns the last value added to the buffer
		float GetLast() const { return (m_pos > 0) ? m_buffer[m_pos - 1] : m_buffer[m_size - 1]; }

		// Returns the value X positions in the history of the buffer. If x is 0 then this is the same as GetLast()
		float GetValue(int x) const;

		// sets the last value to the given value. It's useful sometimes when modifying the readings to
		// make sure they go according to the video
		void SetLast(float val);

		// The size of the buffer
		int GetSize() const { return m_size; }

		// Returns the position in the ring buffer. May be useful at times to know when it changes
		int GetPos() const { return m_pos; }

		// Several statistical functions

		float GetAverage() const;
		float GetMax() const;
		float GetMin() const;
		float GetStandardDeviation(float avg) const;

		float GetAverage(int numElems) const;

	private:
		float* m_buffer;
		int m_size;
		int m_pos;
	};

}

#endif	// __THINKGEAR_RINGBUFFER_H__
