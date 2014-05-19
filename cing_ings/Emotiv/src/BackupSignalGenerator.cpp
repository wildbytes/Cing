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
		m_averageValue[0] = 0.5f;
		m_averageValue[1] = 0.5f;
		m_averageValue[2] = 0.5f;
		m_averageValue[3] = 0.5f;
		m_averageValue[4] = 0.5f;

		m_maxValue[0] = 1.0f;
		m_maxValue[1] = 1.0f;
		m_maxValue[2] = 1.0f;
		m_maxValue[3] = 1.0f;
		m_maxValue[4] = 1.0f;

		for (int i = 0; i < NumValuesToGenerate; ++i)
			m_randomDiffMultiplier[i] = DefaultRandomDiffMultiplier;

	}

}