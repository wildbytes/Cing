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

#pragma once

#include "Emotiv/lib/include/EmoStateDLL.h"
#include "Emotiv/lib/include/edk.h"
#include "Emotiv/lib/include/edkErrorCode.h"

#include "common/XMLSettings.h"


namespace Cing
{

	enum SignalQuality
	{
		E_DISCONNECTED = -1,	// no data or no user or the headset is off
		E_POOR_QUALITY = 0,		// have user and data is being received, but wireless signal is not acceptable
		E_MEDIUM_QUALITY,		// have user and data, but affective or engagement data are disabled due to noisy signals
		E_HIGH_QUALITY,			// all good. NOTE: this does not take into account the quality of each of the sensor contacts in the emotiv's headset.

		E_SIGNALQUALITY_COUNT
	};



/**
 * @brief Access to Emotiv's Epoc and EEG brain sensor interfaces
 */
class EmotivManager
{
public:

	// Info about each user (by default this means, each USB dongle)
	struct UserInfo
	{
		int				userID;
		SignalQuality	signal;
		double			lastEventTimestamp;

		UserInfo(): userID(-1), signal(E_DISCONNECTED) {}
	};

	// Constructor / Destructor
	EmotivManager();
	~EmotivManager();

	// Init / Release / Update
	bool	init	( const std::string& xmlConfigFile = "" );
	void    end     ();
	void	update	();

	// Getters
	double				getNormalizedFrustration()			const { return m_normalizedFrustration; }
	double				getNormalizedEngagement()			const { return m_normalizedEngagement; }
	double				getNormalizedMeditation()			const { return m_normalizedMeditation; }
	double				getNormalizedExcitementShortTerm()	const { return m_normalizedExcitementShortTerm; }

	EE_SignalStrength_t	getWirelessSignalStrength()			const { return m_wirelessSignalStatus; }
	SignalQuality		getSignalQuality()					const;
	SignalQuality		getSignalQualityForActiveUser()		const;

	// Does it have alpha, beta, etc waves data?
	bool				hasValidSpectralAnalysis()			const { return false; }

	void				setActiveUserID(unsigned int userID);
	int					getNumberOfUsers()					const { return m_userCount; }
	UserInfo			getUserInfo( unsigned int index )	const { return (index >= m_userInfo.size())? UserInfo(): m_userInfo[index]; }

private:
	// private methods
	void	updateConnectionState();
	bool	connectToEmotivEngine();
	void	updateAffectiveData();
	void	caculateScale(double& rawScore, double& maxScale, double& minScale, double& scaledScore);
	void	captureEEGData();

	// private attributes

	XMLSettings					m_xmlSettings;

	// General Emotiv 
	EmoEngineEventHandle		m_eEvent;
	EmoStateHandle				m_eState;
	bool						m_connectToEmoComposer;
	std::string					m_emoComposerIp;

	// Global status
	EE_SignalStrength_t			m_wirelessSignalStatus;
	bool						m_userConnected;	// we need an user to retrieve data...
	bool						m_headSetOn;		// true if the headset switch (physical switch) is on.
	int							m_nChannelsWithContact;
	float						m_secsToConsiderNoDataReception, m_lastValidTimestamp;
	bool						m_receivingData;	// true if we are receiving data from the headset.
	float						m_timeStampLastReceivedData;

	// Affectiv suite status (some of the parameters might be disabled if the signal is too noisy).
	bool						m_affectivEngagementEnabled;
	bool						m_affectivExcitementEnabled;
	bool						m_affectivFrustrationEnabled;
	bool						m_affectivMeditationEnabled;

	// Affective suite
	double						m_minFrustration, m_maxFrustration, m_rawFrustration, m_normalizedFrustration;
	double						m_minEngagement, m_maxEngagement, m_rawEngagement, m_normalizedEngagement; // Boredom would be the opposite of Engagement for the Emotive Affective suite
	double						m_minMeditation, m_maxMeditation, m_rawMeditation, m_normalizedMeditation;
	double						m_minExcitementShortTerm, m_maxExcitementShortTerm, m_rawExcitementShortTerm, m_normalizedExcitementShortTerm;
	double						m_normalizedExcitementLongTerm;
	
	// EEG Data capture
	DataHandle					m_hData;
	bool						m_readyToCollectEEGData;

	// User management
	int							m_activeUserID; // if set (not -1), only data from this user ID will be tracked.
	int							m_userCount; // by default, this is the number of dongles.
	std::vector<UserInfo>		m_userInfo;// info per user


	// Other
	const int					m_emoComposerPort;
	bool						m_isValid;
};

} // namespace Cing

