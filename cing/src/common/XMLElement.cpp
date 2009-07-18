/*
  This source file is part of the Cing project
  For the latest info, see http://www.cing.cc

    Copyright (c) 2006-2009 Julio Obelleiro and Jorge Cano

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

#include "XMLElement.h"
#include "LogManager.h"
#include "Release.h"
#include "XMLVisitor.h"

// Tiny xml
#include "TinyXML/include/tinyxml.h"

// Framework
#include "framework/UserAppGlobals.h"

#include <iostream>
#include <sstream>


namespace Cing
{

/**
 * @internal
 * @brief Constructor. Initializes class attributes.
 */
XMLElement::XMLElement():
  m_rootElem( NULL ), m_bIsValid  ( false )
{
}


/**
 * @internal
 * @brief Constructor. Loads an xml file
 * @param xmlFileName  Name of the xml file to load. It must be in the application data folder
 */
XMLElement::XMLElement( const std::string& xmlFileName ):
	m_rootElem( NULL ), m_bIsValid  ( false )
{
	load( xmlFileName );
}

/**
 * @internal
 * @brief Constructor. Builds an xml element from a parent xml element
 * @param root    root root element
 * @param xmlDoc  xmlDoc doc associated
 */
XMLElement::XMLElement( TiXmlElement* root, XMLDocSharedPtr &xmlDoc )
{
  m_xmlDoc    = xmlDoc;
  m_rootElem  = root;
  m_bIsValid = true;
}

/**
 * @internal
 * @brief Destructor. Class release.
 */
XMLElement::~XMLElement()
{
	// Release resources
	end();
}

/**
 * @internal
 * @brief Loads an xml file
 * @param xmlFileName  Name of the xml file to load. It must be in the application data folder
 */
void XMLElement::load( const std::string& xmlFileName )
{
  // Check if load has already been called
  if ( isValid() )
    end();

  // Load the xml file
  m_xmlDoc = XMLDocSharedPtr( new TiXmlDocument( dataFolder + xmlFileName.c_str() ) );
	m_xmlDoc->LoadFile();

	// Error loading file?
	if ( m_xmlDoc->Error() )
	{
		LOG_ERROR( "Error loading %s: %s", xmlFileName.c_str(), m_xmlDoc->ErrorDesc() );
		return;
	}

	// Get root element
	m_rootElem = m_xmlDoc->RootElement();
	if ( !m_rootElem )
		LOG_ERROR( "Error loading %s, There is no root element", xmlFileName.c_str() );


	m_bIsValid = true;
}

/**
 * @internal
 * @brief Releases the class resources.
 * After this call no method of this object can be called without calling init method again.
 */
void XMLElement::end()
{
  // @note The xmlDoc is not deleted because it is a shared pointer (as it can be shared among many XMLElement)
	m_bIsValid = false;
}

/**
 * @brief Returns the number of children for this element
 * @return the number of children for this element
 */
int XMLElement::getChildCount()
{
	// Check state
	if ( !isValid() )
	{
		LOG_ERROR( "Trying to call getChildCount() in a XMLElement no correctly initialized. You should call load() before using this object)" );
		return 0;
	}

	// TODO: check a proper way to do this (maybe tinyxml has something inside)
	int count = 0;
	for( TiXmlNode* node = m_rootElem->FirstChild(); node != NULL; node = node->NextSibling() )
			++count;

	return count;
}

/**
 * @brief Returns a children of the XML element (which will be another element)
 * @return index index of the child that will be returned
 */
XMLElement XMLElement::getChild( int index )
{
	// Check state
	if ( !isValid() )
	{
		LOG_ERROR( "Trying to call getChild() in a XMLElement no correctly initialized. You should call load() before using this object)" );
		return XMLElement();
	}

  // Check number of children
  if ( ( index < 0 ) || ( index >= getChildCount()) )
  {
    LOG_ERROR( "Error in XMLElement::getChild. Child index requested out of bounds" );
    return XMLElement();
  }

  // get child
  int count = 0;
  TiXmlElement *child = m_rootElem->FirstChild()->ToElement();
  while ( count < index )
  {
    // Go to next child
    child = child->NextSibling()->ToElement();

    // Check if child is ok
    if ( child == NULL )
    {
      LOG_ERROR( "Error in XMLElement::getChild. Requested index is out of range" );
      return XMLElement();
    }

    count++;
  }

  // Return found element
  return XMLElement( child, m_xmlDoc );
}

/**
 * @brief Returns a children of the XML element referred by its path. Path element separator is /
 * @return path path to the child that will be returned
 */
XMLElement XMLElement::getChild( const std::string& path )
{
	// Check state
	if ( !isValid() )
	{
		LOG_ERROR( "Trying to call getChild() in a XMLElement no correctly initialized. You should call load() before using this object)" );
		return XMLElement();
	}

  // Find the requested element
  bool found = false;
  TiXmlElement* child = NULL;
  std::string token;
  std::istringstream pathToTokenize( path );
  std::vector< std::string > tokens;

  // Split the path
  while ( getline(pathToTokenize, token, '/') )
    tokens.push_back( token );

  // Check more than one token
  if ( tokens.empty() )
  {
    LOG_ERROR( "Error in XMLElement::getChild. path received is empty" );
    return XMLElement();
  }

  // Get first child
  child = m_rootElem->FirstChild( tokens[0] )->ToElement();
  if ( !child )
  {
    LOG_ERROR( "Error in XMLElement::getChild. path received does not correspond to any child in the XML file. %s not found", tokens[0].c_str() );
    return XMLElement();
  }

  // Iterate to find the rest if there is more than one leve in the path
  for ( size_t i = 1; i < tokens.size(); ++i )
  {
    child = child->FirstChild( tokens[i] )->ToElement();
    if ( !child )
    {
      LOG_ERROR( "Error in XMLElement::getChild. path received does not correspond to any child in the XML file. %s not found", tokens[i].c_str() );
      return XMLElement();
    }
  }

  // Return found element
  return XMLElement( child, m_xmlDoc );
}

/**
 * @brief Fills an array of XMLElemnt with all the xml elements of the file
 * @param children Array where the children will be stored
 * @param path If path is specified, only children that match that path will be stored (optional parameter)
 */
void XMLElement::getChildren( XMLElementArray& children, const String& path /*= "NO_PATH"*/ )
{
  // Check state
  if ( !isValid() )
  {
    LOG_ERROR( "Trying to call getChildren() in a XMLElement no correctly initialized. You should call load() before using this object)" );
    return;
  }

  // Visit all the nodes to fill the children array
  XMLVisitor visitor( m_xmlDoc, children, path );
  m_rootElem->Accept( &visitor );
}

/**
 * @brief Returns the text content of an xml element
 * @return the text content of an xml element
 */
String XMLElement::getContent()
{
  // Check state
  if ( !isValid() )
  {
    LOG_ERROR( "Trying to call getContent() in a XMLElement no correctly initialized. You should call load() before using this object)" );
    return String();
  }

  return m_rootElem->GetText()? m_rootElem->GetText(): "";
}

/**
 * @brief Returns an int attribute of the xml Element.
 * @param name    Name of the attribute to be returned
 * @param defaultValue Default value that will be returned in case the attribute does not exist
 * @return        an int attribute of the xml Element.
 */
int XMLElement::getIntAttribute( const String& name, int defaultValue /*= 0 */)
{
  // Check state
  if ( !isValid() )
  {
    LOG_ERROR( "Trying to call getIntAttribute() in a XMLElement no correctly initialized. You should call load() before using this object)" );
    return defaultValue;
  }

  int value = defaultValue;
  m_rootElem->QueryIntAttribute( name, &value );
  return value;
}

/**
 * @brief Returns a float attribute of the xml Element.
 * @param name    Name of the attribute to be returned
 * @param defaultValue Default value that will be returned in case the attribute does not exist
 * @return        a float attribute of the xml Element.
 */
float XMLElement::getFloatAttribute( const String& name, float defaultValue /*= 0.0f*/ )
{
  // Check state
  if ( !isValid() )
  {
    LOG_ERROR( "Trying to call getIntAttribute() in a XMLElement no correctly initialized. You should call load() before using this object)" );
    return defaultValue;
  }

  float value = defaultValue;
  m_rootElem->QueryFloatAttribute( name.toChar(), &value );
  return value;
}

/**
 * @brief Returns a String attribute of the xml Element.
 * @param name    Name of the attribute to be returned
 * @param defaultValue Default value that will be returned in case the attribute does not exist
 * @return        a String attribute of the xml Element.
 */
String XMLElement::getStringAttribute( const String& name, String defaultValue /*= "0"*/ )
{
  // Check state
  if ( !isValid() )
  {
    LOG_ERROR( "Trying to call getIntAttribute() in a XMLElement no correctly initialized. You should call load() before using this object)" );
    return defaultValue;
  }

  return m_rootElem->Attribute( name.toChar() );
}

/**
 * @brief Returns the name of the elemtn
 * @return the name of the element
 */
String XMLElement::getName()
{
  // Check state
  if ( !isValid() )
  {
    LOG_ERROR( "Trying to call getName() in a XMLElement no correctly initialized. You should call load() before using this object)" );
    return String();
  }

  return m_rootElem->Value();
}

} // namespace Cing