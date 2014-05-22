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
#include "common/MathUtils.h"

namespace Cing
{

	EE_DataChannel_t targetChannelList[] = {
		ED_AF3
		//ED_COUNTER,
		//ED_AF3, ED_F7, ED_F3, ED_FC5, ED_T7, 
		//ED_P7, ED_O1, ED_O2, ED_P8, ED_T8, 
		//ED_FC6, ED_F4, ED_F8, ED_AF4, ED_GYROX, ED_GYROY, ED_TIMESTAMP, 
		//ED_FUNC_ID, ED_FUNC_VALUE, ED_MARKER, ED_SYNC_SIGNAL
	};


	
	/** 
	 * Constructor, not much for now.
	 */
	EmotivManager::EmotivManager()
		:	m_isValid(false), 
			m_emoComposerPort(1726), 
			m_wirelessSignalStatus(NO_SIGNAL),
			m_normalizedFrustration(0.0),
			m_normalizedEngagement(0.0),
			m_normalizedMeditation(0.0),
			m_normalizedExcitementShortTerm(0.0),
			m_normalizedExcitementLongTerm(0.0),
			m_userConnected(false),
			m_headSetOn(false),
			m_receivingData(false),
			m_timeStampLastReceivedData(0.0f),
			m_affectivEngagementEnabled(false),
			m_affectivExcitementEnabled(false),
			m_affectivFrustrationEnabled(false),
			m_affectivMeditationEnabled(false),
			m_readyToCollectEEGData(false)
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
	 * @param & xmlConfigFile Path to the xml config file for the emotiv manager. If no path received, default parameters will be used.
	 */
	bool EmotivManager::init( const std::string& xmlConfigFile /*= ""*/ )
	{
		// If it has already been init, do nothing
		if ( m_isValid )
		{
			LOG( "EmotivManager::init(). This Emotiv manager was init already --> Doing nothing" );
			return false;
		}

		// Load parameters from xml or default if no xml received
		m_emoComposerIp	= "127.0.0.1";
		if ( xmlConfigFile != ""  )
		{
			if ( m_xmlSettings.load( xmlConfigFile ) )
			{
				m_connectToEmoComposer	= m_xmlSettings.getBool( "ConnectToEmoComposer", false );
				m_emoComposerIp			= m_xmlSettings.get( "EmoComposerIP", "127.0.0.1" );
			}
		}
		

		// Create handlers for Emotiv's SDK
		m_eEvent	= EE_EmoEngineEventCreate();
		m_eState	= EE_EmoStateCreate();

		// Connect to headsert or EmoComposer
		m_isValid = connectToEmotivEngine();

		// Prepare for EEG data capture
		//float secs = 1.0f;
		//m_hData = EE_DataCreate();
		//EE_DataSetBufferSizeInSec(secs);



		return m_isValid;
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

		// Release EEG data
		EE_DataFree(m_hData);

		// Disconnect from Emotiv SDK
		EE_EngineDisconnect();
		EE_EmoStateFree(m_eState);
		EE_EmoEngineEventFree(m_eEvent);

		// not connected anymore
		m_isValid = false;
	}

	// General Emotive's update. For now it just updates Affective suite data
	void EmotivManager::update()
	{
		unsigned int userID = 0;

		// If we are not connected, keep trying
		if ( !m_isValid )
		{
			m_isValid = connectToEmotivEngine();
			return;
		}

		// update general headset status
		m_headSetOn = ES_GetHeadsetOn(m_eState) == 1;
		m_wirelessSignalStatus = ES_GetWirelessSignalStatus(m_eState);
		float timestamp = ES_GetTimeFromStart(m_eState);
		m_receivingData = (timestamp > 0.001f) && !equal(timestamp, m_timeStampLastReceivedData); // we have new data if the timestamp of the last data is not zero and it is different from our last update.
		m_timeStampLastReceivedData = timestamp;

		// TEST
		m_receivingData = true;


		// update affectiv suite detection status
		m_affectivEngagementEnabled		= ES_AffectivIsActive(m_eState, AFF_ENGAGEMENT_BOREDOM) == 1;
		m_affectivExcitementEnabled		= ES_AffectivIsActive(m_eState, AFF_EXCITEMENT) == 1;
		m_affectivFrustrationEnabled	= ES_AffectivIsActive(m_eState, AFF_FRUSTRATION) == 1;
		m_affectivMeditationEnabled		= ES_AffectivIsActive(m_eState, AFF_MEDITATION) == 1;

		// Get next event
		int state = EE_EngineGetNextEvent(m_eEvent);

		// New event needs to be handled? 
		if (state == EDK_OK) 
		{

			EE_Event_t eventType = EE_EmoEngineEventGetType(m_eEvent);
			EE_EmoEngineEventGetUserId(m_eEvent, &userID);

			// Log the EmoState if it has been updated
			switch( eventType )
			{
				// New user connected: ready to start reading data
				case EE_UserAdded:
					m_userConnected = true;

					EE_DataAcquisitionEnable(userID, true); // this allows to capture EEG data for this user (i.e. different types of brain waves)
					m_readyToCollectEEGData = true;
					
					LOG( "EmotivManager: New User Added" );
					break;

				case EE_UserRemoved:
					LOG( "EmotivManager: User Removed" );
					m_userConnected = false;
					break;

				// New data from headset
				case EE_EmoStateUpdated:
					EE_EmoEngineEventGetEmoState(m_eEvent, m_eState);

					//LOG("%10.3fs : New Affectiv score from user %d ...\r", m_timeStampLastReceivedData, userID);
					
					// read affective data
					updateAffectiveData();
					break;
			}
		}

		// Capture and process EEG data
		//captureEEGData();
	}

	// Returns the signal quality of the connection with the headset
	SignalQuality EmotivManager::getSignalQuality() const
	{
		// all good: have user, headset is on, wireless signal is good and we have a user connected
		if ( m_headSetOn && m_receivingData && m_userConnected && (m_wirelessSignalStatus == GOOD_SIGNAL) && (m_affectivEngagementEnabled || m_affectivExcitementEnabled) )
			return E_HIGH_QUALITY;
		
		//if we have user and data, but affective or engagement data are disabled due to noisy signals
		else if ( m_headSetOn && m_receivingData && m_userConnected && (!m_affectivEngagementEnabled || !m_affectivExcitementEnabled ) )
			return E_MEDIUM_QUALITY;

		// if we have user and data is being received, but wireless signal is not acceptable
		else if ( m_headSetOn && m_receivingData && m_userConnected && (m_wirelessSignalStatus < GOOD_SIGNAL) )
			return E_POOR_QUALITY;

		//  no data or no user or the headset is off
		return E_DISCONNECTED;
	}


	// Connects to the Emotive Engine (real headset or EmoComposer)
	bool EmotivManager::connectToEmotivEngine()
	{
		if ( m_connectToEmoComposer )
		{
			// Conect to remote EmoComposer (no physical headset)
			int result = EE_EngineRemoteConnect( m_emoComposerIp.c_str(), m_emoComposerPort );
			if ( result != EDK_OK) 
			{
				LOG_ERROR( "EmotivManager::init. ERROR: Emotiv Engine failed to connect remotely (to EmoComposer). Error code: %d.", result);
				return false;
			}
			else
				LOG("Conection to Emotive's remote engine (EmoComposer) up and running...\n");

		}
		else
		{
			// Conect to emotiv engine
			int result = EE_EngineConnect();
			if (result  != EDK_OK) 
			{
				LOG_ERROR( "EmotivManager::init. ERROR: Emotiv Engine failed to connect to headset. Error code: %d.", result);
				return false;
			}
			else
				LOG("Conection to Emotive's engine up and running...\n");
		}

		// we are connected
		return true;
	}

	// Updates Affective suit data (emotions)
	void EmotivManager::updateAffectiveData()
	{
		// Affectiv results

		// This is how it was retrieved initially, but it does not work with EmoComposer. The lines below work well with physical headset and emocomposer as well

		// Frustration
		//ES_AffectivGetFrustrationModelParams(m_eState, &m_rawFrustration, &m_minFrustration, &m_maxFrustration);
		//caculateScale( m_rawFrustration, m_minFrustration, m_maxFrustration, m_normalizedFrustration );
	
		//// Engagement / Boredom
		//ES_AffectivGetEngagementBoredomModelParams(m_eState, &m_rawEngagement, &m_minEngagement, &m_maxEngagement);
		//caculateScale( m_rawEngagement, m_minEngagement, m_maxEngagement, m_normalizedEngagement );

		//// Meditation
		//ES_AffectivGetMeditationModelParams(m_eState, &m_rawMeditation, &m_minMeditation, &m_maxMeditation);
		//caculateScale( m_rawMeditation, m_minMeditation, m_maxMeditation, m_normalizedMeditation );

		//// Short term excitement
		//ES_AffectivGetExcitementShortTermModelParams(m_eState, &m_rawExcitementShortTerm, &m_minExcitementShortTerm, &m_maxExcitementShortTerm);
		//caculateScale( m_rawExcitementShortTerm, m_minExcitementShortTerm, m_maxExcitementShortTerm, m_normalizedExcitementShortTerm );

		m_normalizedFrustration = ES_AffectivGetFrustrationScore(m_eState);
		m_normalizedEngagement	= ES_AffectivGetEngagementBoredomScore(m_eState);
		m_normalizedMeditation	= ES_AffectivGetMeditationScore(m_eState);
		m_normalizedExcitementShortTerm = ES_AffectivGetExcitementShortTermScore(m_eState);
		m_normalizedExcitementLongTerm	= ES_AffectivGetExcitementLongTermScore(m_eState);


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

	// Capture and process EEG data
	void EmotivManager::captureEEGData()
	{
		// readu to collect data?
		if ( !m_readyToCollectEEGData )
			return;

		//// update data since our lasta data capture call
		//EE_DataUpdateHandle(0, m_hData);

		//// See how many samples have been collected
		//unsigned int nSamplesTaken=0;
		//EE_DataGetNumberOfSample(m_hData, &nSamplesTaken);
		//
		//LOG_TRIVIAL( "EmotivManager::captureEEGData: Number of new samples collected: %d", nSamplesTaken );
		//
		//// If we have samples to process
		//if (nSamplesTaken != 0) 
		//{			
		//	unsigned int channelCount = sizeof(targetChannelList)/sizeof(EE_DataChannel_t);
		//	double ** buffer = new double*[channelCount];
		//	for (size_t i=0; i<channelCount; i++)
		//		buffer[i] = new double[nSamplesTaken];

		//	// Get the data for all the channels at once
		//	EE_DataGetMultiChannels(m_hData, targetChannelList, channelCount, buffer, nSamplesTaken);
		//					
		//	for (int sampleIdx=0 ; sampleIdx<(int)nSamplesTaken ; ++ sampleIdx) {
		//		for (int i = 0 ; i<sizeof(targetChannelList)/sizeof(EE_DataChannel_t) ; i++) {

		//			double value = buffer[i][sampleIdx];
		//		}	
		//	}

		//	// delete buffers
		//	for (size_t i=0; i<channelCount; i++)
		//		delete buffer[i];
		//	delete buffer;
		//}

		EE_DataUpdateHandle(0, m_hData);

		unsigned int nSamplesTaken=0;
		EE_DataGetNumberOfSample(m_hData,&nSamplesTaken);
		
		//LOG( "EEG data Updated. Number of samples: %d", nSamplesTaken );
						

		if (nSamplesTaken != 0  ) {

			double* data = new double[nSamplesTaken];
			for (int sampleIdx=0 ; sampleIdx<(int)nSamplesTaken ; ++ sampleIdx) {
				for (int i = 0 ; i<sizeof(targetChannelList)/sizeof(EE_DataChannel_t) ; i++) {

					EE_DataGet(m_hData, targetChannelList[i], data, nSamplesTaken);
					//LOG( "Data: %f", data[sampleIdx] );
				}	
			}
			delete[] data;							
		}
						


	}

}