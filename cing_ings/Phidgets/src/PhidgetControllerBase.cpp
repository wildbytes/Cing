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

#include "PhidgetControllerBase.h"
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

		// Call the handler
		if ( userptr )
			((PhidgetControllerBase*)userptr)->attachHandler(MC, userptr);

		return 0;
	}

	int CCONV DetachHandler(CPhidgetHandle MC, void *userptr)
	{
		int serialNo;
		const char *name;

		CPhidget_getDeviceName (MC, &name);
		CPhidget_getSerialNumber(MC, &serialNo);
		LOG("%s %10d detached!\n", name, serialNo);

		// Call the handler
		if ( userptr )
			((PhidgetControllerBase*)userptr)->detachHandler(MC, userptr);

		return 0;
	}

	int CCONV ErrorHandler(CPhidgetHandle MC, void *userptr, int errorCode, const char *description)
	{
		LOG("Error handled. %d - %s\n", errorCode, description);

		// Call the handler
		if ( userptr )
			((PhidgetControllerBase*)userptr)->errorHandler(MC, userptr, errorCode, description);

		return 0;
	}


	//int display_properties(CPhidgetMotorControlHandle phid)
	//{
	//	int serialNo, version, numInputs, numMotors;
	//	const char* ptr;

	//	CPhidget_getDeviceType((CPhidgetHandle)phid, &ptr);
	//	CPhidget_getSerialNumber((CPhidgetHandle)phid, &serialNo);
	//	CPhidget_getDeviceVersion((CPhidgetHandle)phid, &version);
	//
	//	CPhidgetMotorControl_getInputCount(phid, &numInputs);
	//	CPhidgetMotorControl_getMotorCount(phid, &numMotors);

	//	LOG("%s\n", ptr);
	//	LOG("Serial Number: %10d\nVersion: %8d\n", serialNo, version);
	//	LOG("# Inputs: %d\n# Motors: %d\n", numInputs, numMotors);

	//	return 0;
	//}




	/** 
	 * Constructor, not much for now.
	 */
	PhidgetControllerBase::PhidgetControllerBase()
		: m_isValid(false), m_serialNumber(-1)
	{
	}

	/** 
	 * Destructor, releases resources
	 */
	PhidgetControllerBase::~PhidgetControllerBase()
	{
		end();
	}

	/** 
	 * Inits connection with the motor
	 */
	bool PhidgetControllerBase::init( int serialNumber /*= -1*/, int connectionTimeOutMillis /*= 2000*/ )
	{
		// If it has already been init, do nothing
		if ( m_isValid )
		{
			LOG( "PhidgetControllerBase::init(). This board was init already --> Doing nothing" );
			return false;
		}


		m_serialNumber = serialNumber;

		int result;
		const char *err;

		//Set the handlers to be run when the device is plugged in or opened from software, unplugged or closed from software, or generates an error.
		CPhidget_set_OnAttach_Handler(getPhidgetHandle(), AttachHandler, NULL);
		CPhidget_set_OnDetach_Handler(getPhidgetHandle(), DetachHandler, NULL);
		CPhidget_set_OnError_Handler(getPhidgetHandle(), ErrorHandler, NULL);


		//open the motor control for device connections
		CPhidget_open(getPhidgetHandle(), m_serialNumber);

		//get the program to wait for a motor control device to be attached
		LOG("Waiting for Board [%d]to be attached.", serialNumber);
		if((result = CPhidget_waitForAttachment(getPhidgetHandle(), connectionTimeOutMillis)))
		{
			CPhidget_getErrorDescription(result, &err);
			LOG("Problem waiting for attachment: %s, for board with s/n: %d", err, serialNumber);
			return false;
		}

		//Display the properties of the device
		displayProperties();

		//read motor control event data
		LOG("Connection to phidget board with s/n: %d, up and running...", serialNumber);


		// all good
		m_isValid = true;

		return true;
	}

	/** 
	 * Releases the connection with the motor and resources
	 */
	void PhidgetControllerBase::end()
	{
		// If it has already been init, do nothing
		if ( !m_isValid )
		{
			LOG( "PhidgetControllerBase::end(). This board was released already --> Doing nothing" );
			return;
		}

		// Close motor connection
		LOG( "PhidgetControllerBase Closing connection with the board" );
		CPhidget_close(getPhidgetHandle());
		CPhidget_delete(getPhidgetHandle());


		// all good
		m_isValid = false;
	}



}