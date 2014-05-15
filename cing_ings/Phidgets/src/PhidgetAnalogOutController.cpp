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

#include "PhidgetAnalogOutController.h"
#include "common/LogManager.h"
#include "common/MathUtils.h"

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



	//Display the properties of the attached phidget to the screen.  We will be displaying the name, serial number and version of the attached device.
	//We will also display the total number of available Analogs
	int display_properties(CPhidgetAnalogHandle phid)
	{
		int serialNo, version, numAnalog;
		double max, min;
		const char* ptr;

		CPhidget_getDeviceType((CPhidgetHandle)phid, &ptr);
		CPhidget_getSerialNumber((CPhidgetHandle)phid, &serialNo);
		CPhidget_getDeviceVersion((CPhidgetHandle)phid, &version);
		CPhidgetAnalog_getOutputCount(phid, &numAnalog);
		CPhidgetAnalog_getVoltageMax(phid, 0, &max);
		CPhidgetAnalog_getVoltageMin(phid, 0, &min);

		printf("%s\n", ptr);
		printf("Serial Number: %10d\nVersion: %8d\n", serialNo, version);
		printf("# Analog Outputs: %d\n", numAnalog);
		printf("Output range: -%0.1lfV - %0.1lfV\n",min,max);

		return 0;
	}



	/** 
	 * Constructor, not much for now.
	 */
	PhidgetAnalogOutController::PhidgetAnalogOutController()
		: m_isValid(false), m_analogOut(NULL)
	{
	}

	/** 
	 * Destructor, releases resources
	 */
	PhidgetAnalogOutController::~PhidgetAnalogOutController()
	{
		end();
	}

	/** 
	 * Inits connection with the motor
	 */
	bool PhidgetAnalogOutController::init( int serialNumber /*= -1*/)
	{
		// If it has already been init, do nothing
		if ( m_isValid )
		{
			LOG( "PhidgetAnalogOutController::init(). This analog controller was init already --> Doing nothing" );
			return false;
		}


		int result;
		const char *err;

		//create the analog out control object
		CPhidgetAnalog_create(&m_analogOut);

		//Set the handlers to be run when the device is plugged in or opened from software, unplugged or closed from software, or generates an error.
		CPhidget_set_OnAttach_Handler((CPhidgetHandle)m_analogOut, AttachHandler, NULL);
		CPhidget_set_OnDetach_Handler((CPhidgetHandle)m_analogOut, DetachHandler, NULL);
		CPhidget_set_OnError_Handler((CPhidgetHandle)m_analogOut, ErrorHandler, NULL);


		//open the motor control for device connections
		CPhidget_open((CPhidgetHandle)m_analogOut, serialNumber);

		//get the program to wait for a motor control device to be attached
		LOG("Waiting for Analog to be attached....");
		if((result = CPhidget_waitForAttachment((CPhidgetHandle)m_analogOut, 10000)))
		{
			CPhidget_getErrorDescription(result, &err);
			LOG("Problem waiting for attachment: %s\n", err);
			return 0;
		}

		//Display the properties of the attached motor control device
		display_properties(m_analogOut);

		//read motor control event data
		LOG("Conection to motor(s) up and running...\n");


		// all good
		m_isValid = true;
		return true;
	}

	/** 
	 * Releases the connection with the motor and resources
	 */
	void PhidgetAnalogOutController::end()
	{
		// If it has already been init, do nothing
		if ( !m_isValid || !m_analogOut )
		{
			LOG( "PhidgetAnalogOutController::end(). This motor controller was released already --> Doing nothing" );
			return;
		}

		// Disable all outputs
		setEnabled(0, false);
		setEnabled(1, false);
		setEnabled(2, false);
		setEnabled(3, false);

		// Close motor connection
		LOG( "PhidgetAnalogOutController Closing connection with the controller" );
		CPhidget_close((CPhidgetHandle)m_analogOut);
		CPhidget_delete((CPhidgetHandle)m_analogOut);


		// all good
		m_isValid = false;
	}

	/** 
	 * Enables or disasbles a specific output
	 */
	void PhidgetAnalogOutController::setEnabled( int index, bool enabled )
	{
		if ( !m_isValid || !m_analogOut )
		{
			LOG_CRITICAL( "PhidgetAnalogOutController::setEnabled(). ERROR: Analog controller  is not initialized. Call init first." );
			return;
		}

		if ( enabled )
			CPhidgetAnalog_setEnabled(m_analogOut, index, PTRUE);
		else
			CPhidgetAnalog_setEnabled(m_analogOut, index, PFALSE);

	}

	/** 
	 * Sets the acceleration for a specific motor
	 */
	void PhidgetAnalogOutController::setVoltage( int index, double voltage )
	{
		if ( !m_isValid || !m_analogOut )
		{
			LOG_CRITICAL( "PhidgetAnalogOutController::setVoltage(). ERROR: Analog controller  is not initialized. Call init first." );
			return;
		}

		// clamp just in case
		double max, min;
		CPhidgetAnalog_getVoltageMax(m_analogOut, index, &max);
		CPhidgetAnalog_getVoltageMin(m_analogOut, index, &min);
		voltage = constrain(voltage, min, max);

		// set the voltage
		CPhidgetAnalog_setVoltage(m_analogOut, index, voltage);



	}


}