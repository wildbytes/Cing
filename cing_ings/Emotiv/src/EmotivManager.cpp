/*
  This source file is part of the Cing project
  For the latest info, see http://www.cing.cc

  License: MIT License (http://www.opensource.org/licenses/mit-license.php)
 
  Copyright (c) 2006- Julio Obelleiro, Jorge Cano and the Cing community 

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

// Precompiled headers
#include "Cing-Precompiled.h"

#include "EmotivManager.h"
#include "common/LogManager.h"

namespace Cing
{
	
	/** 
	 * Constructor, not much for now.
	 */
	EmotivManager::EmotivManager()
		: m_isValid(false)
	{
	}

	/** 
	 * Destructor, releases resources
	 */
	EmotivManager::~EmotivManager()
	{
		end();
	}

	/** 
	 * Inits connection with the brain sensor
	 */
	bool EmotivManager::init()
	{
		// If it has already been init, do nothing
		if ( m_isValid )
		{
			LOG( "EmotivManager::init(). This Emotiv manager was init already --> Doing nothing" );
			return false;
		}

		// Create handlers for Emotiv's SDK
		m_eEvent	= EE_EmoEngineEventCreate();
		m_eState	= EE_EmoStateCreate();

		// Conect to emotiv engine
		if (EE_EngineConnect() != EDK_OK) 
		{
			LOG_ERROR("EmotivManager::init. ERROR: Emotiv Engine start up failed.");
		}


		//read motor control event data
		LOG("Conection to Emotive's engine up and running...\n");


		// all good
		m_isValid = true;
		return true;
	}

	/** 
	 * Releases the connection with the motor and resources
	 */
	void EmotivManager::end()
	{
		// If it has already been init, do nothing
		if ( !m_isValid )
		{
			LOG( "EmotivManager::end(). This Emotiv manager was released already --> Doing nothing" );
			return;
		}

		// all good
		m_isValid = false;
	}

	// General Emotive's update. For now it just updates Affective suite data
	void EmotivManager::update()
	{
		unsigned int userID = 0;

		// Get next event
		int state = EE_EngineGetNextEvent(m_eEvent);

		// New event needs to be handled?
		if (state == EDK_OK) 
		{

			EE_Event_t eventType = EE_EmoEngineEventGetType(m_eEvent);
			EE_EmoEngineEventGetUserId(m_eEvent, &userID);

			// Log the EmoState if it has been updated
			if (eventType == EE_EmoStateUpdated) 
			{

				EE_EmoEngineEventGetEmoState(m_eEvent, m_eState);
				const float timestamp = ES_GetTimeFromStart(m_eState);

				printf("%10.3fs : New Affectiv score from user %d ...\r", timestamp, userID);

				// read affective data
				updateAffectiveData();
			}
		}
	}

	// Updates Affective suit data (emotions)
	void EmotivManager::updateAffectiveData()
	{
		// Log the time stamp and user ID
		m_timmeStapLastAffectiveData = ES_GetTimeFromStart(m_eState);

		// Affectiv results

		// Frustration
		ES_AffectivGetFrustrationModelParams(m_eState, &m_rawFrustration, &m_minFrustration, &m_maxFrustration);
		caculateScale( m_rawFrustration, m_minFrustration, m_maxFrustration, m_normalizedFrustration );
	
		// Engagement / Boredom
		ES_AffectivGetEngagementBoredomModelParams(m_eState, &m_rawEngagement, &m_minEngagement, &m_maxEngagement);
		caculateScale( m_rawEngagement, m_minEngagement, m_maxEngagement, m_normalizedEngagement );

		// Meditation
		ES_AffectivGetMeditationModelParams(m_eState, &m_rawMeditation, &m_minMeditation, &m_maxMeditation);
		caculateScale( m_rawMeditation, m_minMeditation, m_maxMeditation, m_normalizedMeditation );

		// Short term excitement
		ES_AffectivGetExcitementShortTermModelParams(m_eState, &m_rawExcitementShortTerm, &m_minExcitementShortTerm, &m_maxExcitementShortTerm);
		caculateScale( m_rawExcitementShortTerm, m_minExcitementShortTerm, m_maxExcitementShortTerm, m_normalizedExcitementShortTerm );
	}


	// Calculates the normalized value (scaled) for an affective suit parameter based on the raw, min and max data values
	void EmotivManager::caculateScale(double& rawScore, double& minScale, double& maxScale, double& scaledScore)
	{
		// Wrong... this should not happen
		if (minScale == maxScale)
		{
			scaledScore = -1;
		}
		else if (rawScore < minScale)
		{
			scaledScore = 0;
		}
		else if (rawScore > maxScale)
		{
			scaledScore = 1;
		}
		else
		{
			scaledScore = (rawScore-minScale)/(maxScale-minScale);
		}
	}
}