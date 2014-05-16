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

#include "PhidgetMotorController.h"
#include "common/LogManager.h"

namespace Cing
{
	/**
		PHIDGETS HANDLERS (specific to motor controller)
	**/
	int CCONV InputChangeHandler(CPhidgetMotorControlHandle MC, void *usrptr, int Index, int State)
	{
		LOG("Input %d > State: %d\n", Index, State);
		return 0;
	}

	int CCONV VelocityChangeHandler(CPhidgetMotorControlHandle MC, void *usrptr, int Index, double Value)
	{
		LOG_TRIVIAL("Motor %d > Current Speed: %f\n", Index, Value);
		return 0;
	}

	int CCONV CurrentChangeHandler(CPhidgetMotorControlHandle MC, void *usrptr, int Index, double Value)
	{
		LOG_TRIVIAL("Motor: %d > Current Draw: %f\n", Index, Value);
		return 0;
	}

	/** 
	 * Constructor, not much for now.
	 */
	PhidgetMotorController::PhidgetMotorController()
		: m_motoControl(NULL)
	{
	}

	/** 
	 * Destructor, releases resources
	 */
	PhidgetMotorController::~PhidgetMotorController()
	{
		end();
	}

	/** 
	 * Inits connection with the motor
	 */
	bool PhidgetMotorController::init( int serialNumber /*= -1*/)
	{
		// If it has already been init, do nothing
		if ( m_isValid )
		{
			LOG( "PhidgetMotorController::init(). This motor controller was init already --> Doing nothing" );
			return false;
		}

		//create the motor control object
		CPhidgetMotorControl_create(&m_motoControl);

		//Registers a callback that will run if an input changes.
		//Requires the handle for the Phidget, the function that will be called, and a arbitrary pointer that will be supplied to the callback function (may be NULL).
		CPhidgetMotorControl_set_OnInputChange_Handler (m_motoControl, InputChangeHandler, NULL);

		//Registers a callback that will run if a motor changes.
		//Requires the handle for the Phidget, the function that will be called, and a arbitrary pointer that will be supplied to the callback function (may be NULL).
		CPhidgetMotorControl_set_OnVelocityChange_Handler (m_motoControl, VelocityChangeHandler, NULL);

		//Registers a callback that will run if the current draw changes.
		//Requires the handle for the Phidget, the function that will be called, and a arbitrary pointer that will be supplied to the callback function (may be NULL).
		CPhidgetMotorControl_set_OnCurrentChange_Handler (m_motoControl, CurrentChangeHandler, NULL);


		// Open the connection with the board
		return PhidgetControllerBase::init(serialNumber);
	}

	/** 
	 * Releases the connection with the motor and resources
	 */
	void PhidgetMotorController::end()
	{
		// If it has already been init, do nothing
		if ( !m_isValid || !m_motoControl )
		{
			LOG( "PhidgetMotorController::end(). This motor controller was released already --> Doing nothing" );
			return;
		}

		// Set motors to 0 velocity
		setVelocity(0, 0);
		setVelocity(1, 0);


		PhidgetControllerBase::end();
	}


	/** 
	 * Sets the velocity for a specific motor. How fast it reaches it depends on the set acceleration. Use setAcceleration to control it.
	 */
	void PhidgetMotorController::setVelocity( int motorIndex, double velocity )
	{
		if ( !m_isValid || !m_motoControl )
		{
			LOG_CRITICAL( "PhidgetMotorController::setVelocity(). ERROR: Motor is not initialized. Call init first." );
			return;
		}

		CPhidgetMotorControl_setVelocity (m_motoControl, motorIndex, velocity);

	}

	/** 
	 * Sets the acceleration for a specific motor
	 */
	void PhidgetMotorController::setAcceleration( int motorIndex, double acc )
	{
		if ( !m_isValid || !m_motoControl )
		{
			LOG_CRITICAL( "PhidgetMotorController::setAcceleration(). ERROR: Motor is not initialized. Call init first." );
			return;
		}

		CPhidgetMotorControl_setAcceleration (m_motoControl, motorIndex, acc);
	}

	/** 
	  * Prints info about this motor
	  */
	void PhidgetMotorController::displayProperties()
	{
		int serialNo, version, numInputs, numMotors;
		const char* ptr;

		CPhidget_getDeviceType(getPhidgetHandle(), &ptr);
		CPhidget_getSerialNumber(getPhidgetHandle(), &serialNo);
		CPhidget_getDeviceVersion(getPhidgetHandle(), &version);
	
		CPhidgetMotorControl_getInputCount(m_motoControl, &numInputs);
		CPhidgetMotorControl_getMotorCount(m_motoControl, &numMotors);

		LOG("%s\n", ptr);
		LOG("Serial Number: %10d\nVersion: %8d\n", serialNo, version);
		LOG("# Inputs: %d\n# Motors: %d\n", numInputs, numMotors);
	}


}