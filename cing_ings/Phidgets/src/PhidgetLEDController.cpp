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

#include "PhidgetLEDController.h"
#include "common/LogManager.h"

namespace Cing
{
	/**
		PHIDGETS HANDLERS (specific to motor controller)
	**/

	/** 
	 * Constructor, not much for now.
	 */
	PhidgetLEDController::PhidgetLEDController()
		: m_LEDControl(NULL)
	{
	}

	/** 
	 * Destructor, releases resources
	 */
	PhidgetLEDController::~PhidgetLEDController()
	{
		end();
	}

	/** 
	 * Inits connection with the motor
	 */
	bool PhidgetLEDController::init( int serialNumber /*= -1*/)
	{
		// If it has already been init, do nothing
		if ( m_isValid )
		{
			LOG( "PhidgetLEDController::init(). This motor controller was init already --> Doing nothing" );
			return false;
		}

		//create the LED object
		LOG( "Connecting to Phidget LED board" );
		CPhidgetLED_create(&m_LEDControl);

		// Open the connection with the board
		return PhidgetControllerBase::init(serialNumber);
	}

	/** 
	 * Releases the connection with the motor and resources
	 */
	void PhidgetLEDController::end()
	{
		// If it has already been init, do nothing
		if ( !m_isValid || !m_LEDControl )
		{
			LOG( "PhidgetLEDController::end(). This LED controller was released already --> Doing nothing" );
			return;
		}

		// Turn off all the LEDs
		int numLED = 0;
		CPhidgetLED_getLEDCount(m_LEDControl, &numLED);
		for( int i = 0; i < numLED; ++i )
		{
			setBrightness(i, 0);
		}

		PhidgetControllerBase::end();
	}


	/** 
	 * Sets the brightness for a specific LED output
	 * @note maximum brightness is 100, 0 is off.  Can set this value to anything including and inbetween these values.
	 */
	void PhidgetLEDController::setBrightness( int index, double brightness )
	{
		if ( !m_isValid || !m_LEDControl)
		{
			LOG_CRITICAL( "PhidgetLEDController::setBrightness(). ERROR LED board is not initialized. Call init first." );
			return;
		}

		CPhidgetLED_setBrightness (m_LEDControl, index, brightness);
	}

	/** 
	 * Returns the brightness for a specific LED output
	 * @note maximum brightness is 100, 0 is off.  Can set this value to anything including and inbetween these values.
	 */
	double PhidgetLEDController::getBrightness( int index )
	{
		if ( !m_isValid || !m_LEDControl)
		{
			LOG_CRITICAL( "PhidgetLEDController::getBrightness(). ERROR LED board is not initialized. Call init first." );
			return 0.0;
		}

		double brightness;
		CPhidgetLED_getBrightness (m_LEDControl, index, &brightness);
		return brightness;
	}


	/** 
	  * Prints info about this motor
	  */
	void PhidgetLEDController::displayProperties()
	{
		int serialNo, version, numLED;
		const char* ptr;

		CPhidget_getDeviceType(getPhidgetHandle(), &ptr);
		CPhidget_getSerialNumber(getPhidgetHandle(), &serialNo);
		CPhidget_getDeviceVersion(getPhidgetHandle(), &version);
		CPhidgetLED_getLEDCount(m_LEDControl, &numLED);

		printf("%s\n", ptr);
		printf("Serial Number: %10d\nVersion: %8d\n", serialNo, version);
		printf("# LEDs: %d\n", numLED);
	}


}