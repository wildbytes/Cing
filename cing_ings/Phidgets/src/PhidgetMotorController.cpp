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
		PHIDGETS HANDLERS
	**/
	int CCONV AttachHandler(CPhidgetHandle MC, void *userptr)
	{
		int serialNo;
		const char *name;

		CPhidget_getDeviceName (MC, &name);
		CPhidget_getSerialNumber(MC, &serialNo);
		LOG("%s %10d attached!\n", name, serialNo);

		return 0;
	}

	int CCONV DetachHandler(CPhidgetHandle MC, void *userptr)
	{
		int serialNo;
		const char *name;

		CPhidget_getDeviceName (MC, &name);
		CPhidget_getSerialNumber(MC, &serialNo);
		LOG("%s %10d detached!\n", name, serialNo);

		return 0;
	}

	int CCONV ErrorHandler(CPhidgetHandle MC, void *userptr, int ErrorCode, const char *Description)
	{
		LOG("Error handled. %d - %s\n", ErrorCode, Description);
		return 0;
	}


	int CCONV InputChangeHandler(CPhidgetMotorControlHandle MC, void *usrptr, int Index, int State)
	{
		LOG("Input %d > State: %d\n", Index, State);
		return 0;
	}

	int CCONV VelocityChangeHandler(CPhidgetMotorControlHandle MC, void *usrptr, int Index, double Value)
	{
		LOG("Motor %d > Current Speed: %f\n", Index, Value);
		return 0;
	}

	int CCONV CurrentChangeHandler(CPhidgetMotorControlHandle MC, void *usrptr, int Index, double Value)
	{
		LOG("Motor: %d > Current Draw: %f\n", Index, Value);
		return 0;
	}

	int display_properties(CPhidgetMotorControlHandle phid)
	{
		int serialNo, version, numInputs, numMotors;
		const char* ptr;

		CPhidget_getDeviceType((CPhidgetHandle)phid, &ptr);
		CPhidget_getSerialNumber((CPhidgetHandle)phid, &serialNo);
		CPhidget_getDeviceVersion((CPhidgetHandle)phid, &version);
	
		CPhidgetMotorControl_getInputCount(phid, &numInputs);
		CPhidgetMotorControl_getMotorCount(phid, &numMotors);

		LOG("%s\n", ptr);
		LOG("Serial Number: %10d\nVersion: %8d\n", serialNo, version);
		LOG("# Inputs: %d\n# Motors: %d\n", numInputs, numMotors);

		return 0;
	}




	/** 
	 * Constructor, not much for now.
	 */
	PhidgetMotorController::PhidgetMotorController()
		: m_isValid(false), m_motoControl(NULL)
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


		int result;
		const char *err;

		//create the motor control object
		CPhidgetMotorControl_create(&m_motoControl);

		//Set the handlers to be run when the device is plugged in or opened from software, unplugged or closed from software, or generates an error.
		CPhidget_set_OnAttach_Handler((CPhidgetHandle)m_motoControl, AttachHandler, NULL);
		CPhidget_set_OnDetach_Handler((CPhidgetHandle)m_motoControl, DetachHandler, NULL);
		CPhidget_set_OnError_Handler((CPhidgetHandle)m_motoControl, ErrorHandler, NULL);

		//Registers a callback that will run if an input changes.
		//Requires the handle for the Phidget, the function that will be called, and a arbitrary pointer that will be supplied to the callback function (may be NULL).
		CPhidgetMotorControl_set_OnInputChange_Handler (m_motoControl, InputChangeHandler, NULL);

		//Registers a callback that will run if a motor changes.
		//Requires the handle for the Phidget, the function that will be called, and a arbitrary pointer that will be supplied to the callback function (may be NULL).
		CPhidgetMotorControl_set_OnVelocityChange_Handler (m_motoControl, VelocityChangeHandler, NULL);

		//Registers a callback that will run if the current draw changes.
		//Requires the handle for the Phidget, the function that will be called, and a arbitrary pointer that will be supplied to the callback function (may be NULL).
		CPhidgetMotorControl_set_OnCurrentChange_Handler (m_motoControl, CurrentChangeHandler, NULL);

		//open the motor control for device connections
		CPhidget_open((CPhidgetHandle)m_motoControl, serialNumber);

		//get the program to wait for a motor control device to be attached
		LOG("Waiting for MotorControl to be attached....");
		if((result = CPhidget_waitForAttachment((CPhidgetHandle)m_motoControl, 10000)))
		{
			CPhidget_getErrorDescription(result, &err);
			LOG("Problem waiting for attachment: %s\n", err);
			return 0;
		}

		//Display the properties of the attached motor control device
		display_properties(m_motoControl);

		//read motor control event data
		LOG("Conection to motor(s) up and running...\n");


		// all good
		m_isValid = true;
		return true;
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

		// Close motor connection
		LOG( "PhidgetMotorController Closing connection with the motors" );
		CPhidget_close((CPhidgetHandle)m_motoControl);
		CPhidget_delete((CPhidgetHandle)m_motoControl);


		// all good
		m_isValid = false;
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


}