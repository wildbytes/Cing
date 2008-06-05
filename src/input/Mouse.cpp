/*
  This source file is part of the Vision project
  For the latest info, see http://www.XXX.org

  Copyright (c) 2008 XXX

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "Mouse.h"

// Framework
#include "framework/Application.h"

// Graphics
#include "graphics/GraphicsManager.h"

// OIS
#include "OISInputManager.h"

namespace Input
{

/**
 * @internal
 * @brief Constructor. Initializes class attributes.
 */
Mouse::Mouse():
  BaseInputDevice(),
  m_pOISMouse( NULL ),
  m_bIsValid( false )
{
}

/**
 * @internal
 * @brief Destructor. Class release.
 */
Mouse::~Mouse()
{
  // Release resources
  end();
}

/**
 * @internal
 * @brief Initializes the class so it becomes valid.
 *
 * @param[in] pOISInputManager OIS Input manager. Allows to create and destroy the OIS Mouse
 * @return true if the initialization was ok | false otherwise
 */
bool Mouse::init( OIS::InputManager* pOISInputManager )
{
  // Check if the class is already initialized
  if ( isValid() )
    return true;

  // Create the buffered mouse
  if ( pOISInputManager && ( pOISInputManager->numMice() > 0 ) )
  {
      m_pOISMouse = static_cast< OIS::Mouse* >( pOISInputManager->createInputObject( OIS::OISMouse, true ) );

      // Activate call backs on mouse events
      m_pOISMouse->setEventCallback( this );

      // Get window size
      Graphics::Window::TWindowMetrics windowMetrics;
      Graphics::GraphicsManager::getSingleton().getMainWindow().getMetrics( windowMetrics );

      // Set mouse window limits
      const OIS::MouseState &mouseState = m_pOISMouse->getMouseState();
      mouseState.width  = windowMetrics.width;
      mouseState.height = windowMetrics.height;
  }

  // Store OIS input manager
  m_pOISInputManager = pOISInputManager;

	// The class is now initialized
	m_bIsValid = true;

	return true;
}

/**
 * @internal
 * @brief Releases the class resources. 
 * After this method is called the class is not valid anymore.
 */
void Mouse::end()
{
  // Check if the class is already released
  if ( !isValid() )
    return;

  // Release mouse
  m_pOISInputManager->destroyInputObject( m_pOISMouse );
  m_pOISMouse         = NULL;
  m_pOISInputManager  = NULL;

	// The class is not valid anymore
	m_bIsValid = false;
}

/**
 * @internal
 * @brief Captures the mouse events occurred in the frame
 */
void Mouse::update()
{
  if ( !isValid() )
    return;

  m_pOISMouse->capture();
}

// TEMP
bool Mouse::isButtonPressed( int n ) const
{
  const OIS::MouseState& mouseState = m_pOISMouse->getMouseState();
  return mouseState.buttonDown( OIS::MouseButtonID( n ) );
}

int Mouse::getXAxisRelative() const
{
  const OIS::MouseState& mouseState = m_pOISMouse->getMouseState();
  return mouseState.X.rel;
}
int Mouse::getYAxisRelative() const
{
  const OIS::MouseState& mouseState = m_pOISMouse->getMouseState();
  return mouseState.Y.rel;
}
int Mouse::getZAxisRelative() const
{
  const OIS::MouseState& mouseState = m_pOISMouse->getMouseState();
  return mouseState.Z.rel;
}


//// OIS Mouse interface

/**
 * @internal
 * @brief Receives an event of mouse moved and informs all registered listeners
 *
 * @param[in] event Received event. Contains the information about the mouse state
 */
bool Mouse::mouseMoved( const OIS::MouseEvent &event )
{
  //TODO: m_listeners->mouseMoved()
  return true;
}



/**
 * @internal
 * @brief Receives an event of mouse pressed and informs all registered listeners
 *
 * @param[in] event Received event. Contains the information about the mouse state
 * @param[in] id    Id of the mouse that has been pressed
 */
bool Mouse::mousePressed ( const OIS::MouseEvent &event, OIS::MouseButtonID id )
{
  // Call user event function
  // TODO esto se deber�a hacer registrando listeners, y que applicatino estuviera registrado
  Framework::Application::getSingleton().mousePressedEvent();
  return true;
}

/**
 * @internal
 * @brief Receives an event of mouse released and informs all registered listeners
 *
 * @param[in] event Received event. Contains the information about the mouse state
 * @param[in] id    Id of the mouse that has been released
 */
bool Mouse::mouseReleased( const OIS::MouseEvent &event, OIS::MouseButtonID id )
{
  return true;
}


} // namespace Input