#pragma once

#include "RingBuffer.h"

namespace Cing
{

	//-------------------------------------------------------------------------------------------------
	// This class generates a fake (or backup) signal for the different brain wave meditions. 
	// It could be used for testing as it simulates the creation of brainwaves.
	//
	// NOTE: [Barkley] I've been studying the distribution of the signal and I believe it looks more like
	//  a gamma distribution (http://es.wikipedia.org/wiki/Distribuci%C3%B3n_Gamma). Actually, it's not
	//  so much the values as their square roots that look like that. I've decided to use that function
	//  to calculate the random values for it.
	// 
	//  I don't know the best (and fastest) way to produce random values in a gamma distribution, thus I'll
	//  still use the same algorithm than the normal distribution but I'll have different values for 
	//  below the average and above it. For that we will determine an average and a maximum value. The
	//  minimum value is always 0.
	//
	// Generates values for Alpha, Beta, Delta, Theta y Gamma
	class BackupSignalGenerator
	{
	public:

		const static int NumValuesToGenerate = 6;		//< Alpha, Beta, Delta, Theta y Gamma / Attention
		const static int BufferSize = 5;
		static float DefaultRandomDiffMultiplier;

	public:
		BackupSignalGenerator();
		~BackupSignalGenerator();

		void setUpdateFrequency( float timeBetweenGenerations ) { m_timeBetweenGenerations = timeBetweenGenerations; }

		// Updates the values with new generated values, but only once every X seconds
		void	update(float weights[NumValuesToGenerate]);

		void	setAverageValue(int index, float value)	{ m_averageValue[index] = value; }
		float	getAverageValue(int index)	{ return m_averageValue[index]; }
		void	setMaxValue(int index, float value)		{ m_maxValue[index] = value; }

		// Getters for the different buffers
		float	getAlphaValue() const { return	m_alpha.GetAverage(); }
		float	getBetaValue() const { return	m_beta.GetAverage(); }
		float	getGammaValue() const { return	m_gamma.GetAverage(); }
		float	getDeltaValue() const { return	m_delta.GetAverage(); }
		float	getThetaValue() const { return	m_theta.GetAverage(); }
		float	getAttentionValue() const { return	m_attention.GetAverage(); }

	private:
		// Inits the average and max values used for the calculations
		void initValues();

		// Generates and returns a new fake value given a set of parameters and the last value
		float generateValue(float lastVal, float avgVal, float maxOffset, float multiplier);

	private:

		float m_timer;
		float m_timeBetweenGenerations;

		float m_averageValue[NumValuesToGenerate];
		float m_maxValue[NumValuesToGenerate];
		float m_randomDiffMultiplier[NumValuesToGenerate];
		float m_lastValue[NumValuesToGenerate];

		// Buffers
		RingBuffer m_alpha;
		RingBuffer m_beta;
		RingBuffer m_gamma;
		RingBuffer m_delta;
		RingBuffer m_theta;
		RingBuffer m_attention;

	};

}
