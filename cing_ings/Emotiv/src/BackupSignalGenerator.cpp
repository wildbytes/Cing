// Precompiled headers
#include "Cing-Precompiled.h"

#include "BackupSignalGenerator.h"

#include "Cing.h"


namespace Cing
{
	float BackupSignalGenerator::DefaultRandomDiffMultiplier = 0.5f;

	//-------------------------------------------------------------------------------------------------
	BackupSignalGenerator::BackupSignalGenerator()
		: m_delta(BufferSize)
		, m_theta(BufferSize)
		, m_alpha(BufferSize)
		, m_beta(BufferSize)
		, m_gamma(BufferSize)
		, m_attention(BufferSize)
		, m_timer(0.0f)
		, m_timeBetweenGenerations(0.3f)
	{
		initValues();

		for (int i = 0; i < NumValuesToGenerate; ++i)
			m_lastValue[i] = m_averageValue[i];
	}

	//-------------------------------------------------------------------------------------------------
	BackupSignalGenerator::~BackupSignalGenerator()
	{
	}

	//-------------------------------------------------------------------------------------------------
	void BackupSignalGenerator::update(float weights[NumValuesToGenerate])
	{
		m_timer += Cing::elapsedSec;
		if ( m_timer < m_timeBetweenGenerations )
			return;

		m_timer = 0.0f;


		// first generate the new random values
		for (int i = 0; i < NumValuesToGenerate; ++i)
		{
			m_lastValue[i] = generateValue(m_lastValue[i], m_averageValue[i], m_maxValue[i], m_randomDiffMultiplier[i]);
		}

		// now assign it to the readings
		m_alpha.Add( m_lastValue[0] * weights[0] );
		m_beta.Add( m_lastValue[1] * weights[1] );
		m_gamma.Add( m_lastValue[2] * weights[2] );
		m_delta.Add( m_lastValue[3] * weights[3] );
		m_theta.Add( m_lastValue[4] * weights[4] );
		m_attention.Add( m_lastValue[5] *  weights[5] );
	}

	//-------------------------------------------------------------------------------------------------
	float BackupSignalGenerator::generateValue(float lastVal, float avgVal, float maxValue, float multiplier)
	{
		// calculate a random value that tends toward the average
		float diffToAvg = lastVal - avgVal;
		float normPos;
		if (diffToAvg > 0.0f)
			normPos = diffToAvg / (maxValue - avgVal);
		else
			normPos = diffToAvg / avgVal;

		// calculate the new value
		float normRand = random(-1.0f - normPos, 1.0f - normPos);
		normPos += normRand;
		float newVal = (normPos <= 0.0f) ? (avgVal * -normPos) : (avgVal + (maxValue - avgVal) * normPos);
		float offset = newVal - lastVal;

		// then modulate that offset with the multiplier and add it to the last value
		offset *= multiplier;
		lastVal += offset;

		return lastVal;
	}

	//-------------------------------------------------------------------------------------------------
	void BackupSignalGenerator::initValues()
	{
		for (int i = 0; i < NumValuesToGenerate; ++i)
			m_averageValue[i] = 0.5f;

		for (int i = 0; i < NumValuesToGenerate; ++i)
			m_maxValue[i] = 1.0f;

		for (int i = 0; i < NumValuesToGenerate; ++i)
			m_randomDiffMultiplier[i] = DefaultRandomDiffMultiplier;

	}

}