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


namespace Cing
{

/**
 * @brief Access to Emotiv's Epoc and EEG brain sensor interfaces
 */
class EmotivManager
{
public:

	// Constructor / Destructor
	EmotivManager();
	~EmotivManager();

	// Init / Release / Update
	bool	init	();
	void    end     ();
	void	update	();

	// Getters
	double	getNormalizedFrustration()			const { return m_normalizedFrustration; }
	double	getNormalizedEngagement()			const { return m_normalizedEngagement; }
	double	getNormalizedMeditation()			const { return m_normalizedMeditation; }
	double	getNormalizedExcitementShortTerm()	const { return m_normalizedExcitementShortTerm; }

private:
	// private methods
	void	updateAffectiveData();
	void	caculateScale(double& rawScore, double& maxScale, double& minScale, double& scaledScore);

	// private attributes

	// General Emotiv 
	EmoEngineEventHandle		m_eEvent;
	EmoStateHandle				m_eState;

	// Affective suite
	float						m_timmeStapLastAffectiveData;
	double						m_minFrustration, m_maxFrustration, m_rawFrustration, m_normalizedFrustration;
	double						m_minEngagement, m_maxEngagement, m_rawEngagement, m_normalizedEngagement; // Boredom would be the opposite of Engagement for the Emotive Affective suite
	double						m_minMeditation, m_maxMeditation, m_rawMeditation, m_normalizedMeditation;
	double						m_minExcitementShortTerm, m_maxExcitementShortTerm, m_rawExcitementShortTerm, m_normalizedExcitementShortTerm;
	
	bool						m_isValid;
};

} // namespace Cing

