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

#include "PhidgetControllerBase.h"

namespace Cing
{

/**
 * @brief Access and control to a Interface Kit 0/0/4 to control high power devices like a light bulb
 */
class PhidgetInterfaceKitController: public PhidgetControllerBase
{
public:

	// Constructor / Destructor
	PhidgetInterfaceKitController();
	~PhidgetInterfaceKitController();

	// Init / Release / Update
	bool	init	( int serialNumber = -1);
	void    end     ();

	// Control
	void	setDigitalOutEnabled	( int index, bool enabled );
	bool	getDigitalOutEnabled	( int index );

	// base required overwrides
	CPhidgetHandle	getPhidgetHandle	()	{ return (CPhidgetHandle)m_interfaceKit; };
	void			displayProperties	();

private:

	CPhidgetInterfaceKitHandle	m_interfaceKit;
};

} // namespace Cing
