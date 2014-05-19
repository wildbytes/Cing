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

#include "PhidgetInterfaceKitController.h"
#include "common/LogManager.h"
#include "common/MathUtils.h"

namespace Cing
{
	
	//callback that will run if an input changes.
	//Index - Index of the input that generated the event, State - boolean (0 or 1) representing the input state (on or off)
	int CCONV InputChangeHandler(CPhidgetInterfaceKitHandle IFK, void *usrptr, int Index, int State)
	{
		LOG("Digital Input: %d > State: %d\n", Index, State);
		return 0;
	}

	//callback that will run if an output changes.
	//Index - Index of the output that generated the event, State - boolean (0 or 1) representing the output state (on or off)
	int CCONV OutputChangeHandler(CPhidgetInterfaceKitHandle IFK, void *usrptr, int Index, int State)
	{
		LOG("Digital Output: %d > State: %d\n", Index, State);
		return 0;
	}

	//callback that will run if the sensor value changes by more than the OnSensorChange trigger.
	//Index - Index of the sensor that generated the event, Value - the sensor read value
	int CCONV SensorChangeHandler(CPhidgetInterfaceKitHandle IFK, void *usrptr, int Index, int Value)
	{
		LOG("Sensor: %d > Value: %d\n", Index, Value);
		return 0;
	}

	/** 
	 * Constructor, not much for now.
	 */
	PhidgetInterfaceKitController::PhidgetInterfaceKitController()
		: m_interfaceKit(NULL)
	{
	}

	/** 
	 * Destructor, releases resources
	 */
	PhidgetInterfaceKitController::~PhidgetInterfaceKitController()
	{
		end();
	}

	/** 
	 * Inits connection with the motor
	 */
	bool PhidgetInterfaceKitController::init( int serialNumber /*= -1*/)
	{
		// If it has already been init, do nothing
		if ( m_isValid )
		{
			LOG( "PhidgetInterfaceKitController::init(). This analog controller was init already --> Doing nothing" );
			return false;
		}

		LOG( "Connecting to Phidget Interface Kit board" );

		//create the interface kit control object
		CPhidgetInterfaceKit_create(&m_interfaceKit);

		//Registers a callback that will run if an input changes.
		//Requires the handle for the Phidget, the function that will be called, and an arbitrary pointer that will be supplied to the callback function (may be NULL).
		CPhidgetInterfaceKit_set_OnInputChange_Handler (m_interfaceKit, InputChangeHandler, NULL);

		//Registers a callback that will run if the sensor value changes by more than the OnSensorChange trig-ger.
		//Requires the handle for the IntefaceKit, the function that will be called, and an arbitrary pointer that will be supplied to the callback function (may be NULL).
		CPhidgetInterfaceKit_set_OnSensorChange_Handler (m_interfaceKit, SensorChangeHandler, NULL);

		//Registers a callback that will run if an output changes.
		//Requires the handle for the Phidget, the function that will be called, and an arbitrary pointer that will be supplied to the callback function (may be NULL).
		CPhidgetInterfaceKit_set_OnOutputChange_Handler (m_interfaceKit, OutputChangeHandler, NULL);


		// Open the connection with the board
		return PhidgetControllerBase::init(serialNumber);
	}

	/** 
	 * Releases the connection with the motor and resources
	 */
	void PhidgetInterfaceKitController::end()
	{
		// If it has already been init, do nothing
		if ( !m_isValid || !m_interfaceKit )
		{
			LOG( "PhidgetInterfaceKitController::end(). This controller was released already --> Doing nothing" );
			return;
		}

		// Disable all outputs
		setDigitalOutEnabled(0, false);
		setDigitalOutEnabled(1, false);
		setDigitalOutEnabled(2, false);
		setDigitalOutEnabled(3, false);

		PhidgetControllerBase::end();
	}

	/** 
	 * Enables or disasbles a specific digital output
	 */
	void PhidgetInterfaceKitController::setDigitalOutEnabled( int index, bool enabled )
	{
		if ( !m_isValid || !m_interfaceKit )
		{
			LOG_CRITICAL( "PhidgetInterfaceKitController::setEnabled(). ERROR: Interface kit controller  is not initialized. Call init first." );
			return;
		}

		if ( enabled )
			CPhidgetInterfaceKit_setOutputState(m_interfaceKit, index, PTRUE);
		else
			CPhidgetInterfaceKit_setOutputState(m_interfaceKit, index, PFALSE);
	}

	/** 
	 * Returns the state of a digital output. 
	 * @return true if the output is enabled, false otherwise
	 */
	bool PhidgetInterfaceKitController::getDigitalOutEnabled( int index )
	{
		if ( !m_isValid || !m_interfaceKit )
		{
			LOG_CRITICAL( "PhidgetInterfaceKitController::getDigitalOutEnabled(). ERROR Interface kit controller  is not initialized. Call init first." );
			return PFALSE;
		}

		int state;
		CPhidgetInterfaceKit_getOutputState(m_interfaceKit, index, &state);
		return state == PTRUE;
	}

	/** 
	  * Prints info about this controller
	  */
	void PhidgetInterfaceKitController::displayProperties()
	{
		int serialNo, version, numInputs, numOutputs, numSensors, triggerVal, ratiometric, i;
		const char* ptr;

		CPhidget_getDeviceType(getPhidgetHandle(), &ptr);
		CPhidget_getSerialNumber(getPhidgetHandle(), &serialNo);
		CPhidget_getDeviceVersion(getPhidgetHandle(), &version);

		CPhidgetInterfaceKit_getInputCount(m_interfaceKit, &numInputs);
		CPhidgetInterfaceKit_getOutputCount(m_interfaceKit, &numOutputs);
		CPhidgetInterfaceKit_getSensorCount(m_interfaceKit, &numSensors);
		CPhidgetInterfaceKit_getRatiometric(m_interfaceKit, &ratiometric);

		LOG("%s\n", ptr);
		LOG("Serial Number: %10d\nVersion: %8d\n", serialNo, version);
		LOG("# Digital Inputs: %d\n# Digital Outputs: %d\n", numInputs, numOutputs);
		LOG("# Sensors: %d\n", numSensors);
		LOG("Ratiometric: %d\n", ratiometric);

		for(i = 0; i < numSensors; i++)
		{
			CPhidgetInterfaceKit_getSensorChangeTrigger (m_interfaceKit, i, &triggerVal);

			LOG("Sensor#: %d > Sensitivity Trigger: %d\n", i, triggerVal);
		}
	}
}