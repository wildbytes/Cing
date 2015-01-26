/*
This source file is part of the Cing project
For the latest info, see http://www.cing.cc

Copyright (c) 2008-2010 Julio Obelleiro and Jorge Cano

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the im_mediaPlayerlied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Tem_mediaPlayerle Place, Suite 330, Boston, MA  02111-1307  USA
*/

// Precompiled headers
#include "Cing-Precompiled.h"

#include "MediaPlayerHap.h"

// QuickTime includes
#include <QTML.h>
#include <Movies.h>
#include <FixMath.h>
#define MediaPlayerHapFloatToFixed(x) X2Fix(x)

// Hap Includes
#include "MediaPlayerHap.h"
#include "HapSupport.h"

// Common
#include "common/CommonUtilsIncludes.h"
#include "common/XMLElement.h"
#include "common/LogManager.h"

// Framework
#include "framework/UserAppGlobals.h"

// Graphics
#include "graphics/Color.h"
#include "graphics/GraphicsUserAPI.h"


namespace Cing
{

/*
Utility to round up to a multiple of 4 for DXT dimensions
*/
static int roundUpToMultipleOf4( int n )
{
    if( 0 != ( n & 3 ) )
        n = ( n + 3 ) & ~3;
    return n;
}

/*
Utility to know if we are working with DXT data
*/
static bool isDXTPixelFormat(OSType fmt)
{
    switch (fmt)
    {
    case kHapPixelFormatTypeRGB_DXT1:
    case kHapPixelFormatTypeRGBA_DXT5:
    case kHapPixelFormatTypeYCoCg_DXT5:
        return true;
    default:
        return false;
    }
}

/*
Called by QuickTime when we have a new frame
*/
OSErr MediaPlayerHapDrawComplete(Movie theMovie, long refCon){

    *(bool *)refCon = true;
    return noErr;
}


/**
	* Default constructor.
	*/
MediaPlayerHap::MediaPlayerHap():
	m_movie			( NULL ),
	m_gWorld		( NULL ),
	m_buffer		( NULL ),
	m_texture		( NULL ),
	m_shader		( NULL ),
	m_videoWidth	( 0 ),
	m_videoHeight	( 0 ),
	m_videoFps		( 0 ),
	m_videoDuration	( 0 ),
	m_loop			( false ),
	m_loopPending	( false	),
	m_bIsValid		( false ),
	m_pixelFormat	( RGB	),
	m_mute			( false ),
	m_volume		( 1.0f  ),
	m_endOfFileReached	(false),
	m_paused			(false),
	m_playing			(false),
	m_speed			( 1.0f )
{
}

/**
	* Constructor to load a movie file.
	* @param filename	Name of the movie to load (it can be a local path relative to the data folder, or a network path)
	* @param fps		Desired Frames per Second for the playback. -1 means to use the fps of the movie file.
	*/
MediaPlayerHap::MediaPlayerHap( const char* filename, float fps /*= -1*/ ):
	m_movie			( NULL ),
	m_gWorld		( NULL ),
	m_buffer		( NULL ),
	m_texture		( NULL ),
	m_shader		( NULL ),
	m_videoWidth	( 0 ),
	m_videoHeight	( 0 ),
	m_videoFps		( 0 ),
	m_videoDuration	( 0 ),
	m_loop			( false ),
	m_loopPending	( false	),
	m_bIsValid		( false ),
	m_pixelFormat	( RGB	),
	m_mute			( false ),
	m_volume		( 1.0f  ),
	m_endOfFileReached	(false),
	m_paused			(false),
	m_playing			(false),
	m_speed			( 1.0f )
{
	// Load the movie
	load( filename );
}

/**
	* Default destructor.
	*/
MediaPlayerHap::~MediaPlayerHap()
{
	end();
}


/**
	* Inits the object to make it valid.
	*/
bool MediaPlayerHap::init()
{
	LOG_ENTER_FUNCTION;

	// http://developer.apple.com/library/mac/#documentation/QuickTime/Conceptual/QT_InitializingQT/InitializingQT/InitializingQTfinal.html
	// Calls to InitializeQTML() must be balanced by calls to TerminateQTML()
	InitializeQTML(0);

	// There is no harm or cost associated with calling EnterMovies() after the first call, and
	// it should NOT be balanced by calls to ExitMovies()
	EnterMovies();

	LOG_EXIT_FUNCTION;

	return true;
}

/**
	* Loads a movie file
	* @param filename		Name of the movie to load (it can be a local path relative to the data folder, or a network path)
	* @param requestedVideoFormat	Format in which the frames of the movie will be stored. Default RGB, which means that the
	*								movie file video format will be used (regarding alpha channel: RGB vs RGBA)
	* @param fps			Desired Frames per Second for the playback. -1 means to use the fps of the movie file.
	* @return true if the video was succesfully loaded
	*/
bool MediaPlayerHap::load( std::string const& fileName )
{
	// Init the player (to make sure GStreamer is initialized)
	bool resIni = init();
	if ( !resIni )
	{
		LOG_ERROR( "MediaPlayerHap::load. Error loading %s, GStreamer could not load it correctly.", fileName.c_str() );
		end();
		return false;
	}

	// Build path to file
	resIni = buildPathToFile( fileName );
	if ( !resIni )
	{
		LOG_ERROR( "MediaPlayerHap::load. File Not Found: %s", fileName.c_str() );
		end();
		return false;
	}

	LOG_ENTER_FUNCTION;
	
	OSStatus result = noErr;
	m_fileName = fileName;
	
	// Close any open movie
	close();

	// Opening a movie requires an active graphics world, so set a dummy one
	CGrafPtr previousGWorld;
	GDHandle previousGDH;
    
	GetGWorld(&previousGWorld, &previousGDH);

	GWorldPtr tempGWorld;
	const ::Rect bounds = {0, 0, 1, 1};
	QTNewGWorld(&tempGWorld, 32, &bounds, NULL, NULL, useTempMem | keepLocal);
    
	SetGWorld(tempGWorld, NULL);

	// try to open the file
	FSSpec fsSpec;
	short fileRefNum;
	result = NativePathNameToFSSpec((char *)fileName.c_str(), &fsSpec, 0);
	if (result == noErr)
	{
		result = OpenMovieFile(&fsSpec, &fileRefNum, fsRdPerm);
	}
	if (result == noErr)
	{
		short resID = 0;
		result = NewMovieFromFile((Movie *)&m_movie, fileRefNum, &resID, NULL, 0, NULL);
		CloseMovieFile(fileRefNum);
	}

	if (result != noErr)
	{
		LOG_ERROR("Error loading movie");
	}

	OSType wantedPixelFormat;
	::Rect renderRect;
	//size_t bitsPerPixel;
	
	if (result == noErr)
	{
		// Determine the movie rectangle
		GetMovieBox((Movie)m_movie, &renderRect);

		// Determine the pixel-format to use and adjust dimensions for Hap
		if (HapQTQuickTimeMovieHasHapTrackPlayable((Movie)m_movie))
		{
			m_hapAvailable = true;
			wantedPixelFormat = HapQTGetQuickTimeMovieHapPixelFormat((Movie)m_movie);
			renderRect.bottom = roundUpToMultipleOf4(renderRect.bottom);
			renderRect.right = roundUpToMultipleOf4(renderRect.right);
		}
		else
		{
			/*
				For now we fail for non-Hap movies
				*/
			result = 1;
		}
	}
    
	/*
	Check any existing GWorld is appropriate for this movie
	*/
	if (result == noErr && m_gWorld != NULL)
	{
		PixMapHandle pmap = GetGWorldPixMap((GWorldPtr)m_gWorld);
		if ((*pmap)->pixelFormat != wantedPixelFormat ||
			(*pmap)->bounds.bottom != renderRect.bottom ||
			(*pmap)->bounds.right != renderRect.right)
		{
			DisposeGWorld((GWorldPtr)m_gWorld);
			m_gWorld = NULL;
			delete[] m_buffer;
			m_buffer = NULL;
		}
	}

	if (result == noErr && m_gWorld == NULL)
	{
		// Create a GWorld to render into
		size_t bitsPerPixel;
		Ogre::PixelFormat internalFormat;
		switch (wantedPixelFormat)
		{
		case kHapPixelFormatTypeRGB_DXT1:
			bitsPerPixel = 4;
			internalFormat = Ogre::PF_DXT1;
			break;
		case kHapPixelFormatTypeRGBA_DXT5:
		case kHapPixelFormatTypeYCoCg_DXT5:
			bitsPerPixel = 8;
			internalFormat = Ogre::PF_DXT5;
			break;
		default:
			bitsPerPixel = 32;
			internalFormat = Ogre::PF_BYTE_RGBA;
			break;
		}
		size_t bytesPerRow = renderRect.right * bitsPerPixel / 8;
		m_buffer = new unsigned char[renderRect.bottom * bytesPerRow];
		OSErr newWorldError = QTNewGWorldFromPtr((GWorldPtr*)&m_gWorld, wantedPixelFormat, &renderRect, NULL, NULL, 0, (Ptr)m_buffer, bytesPerRow);
		if (newWorldError != noErr)
		{
			LOG_WARNING("Error creating the GWorld in the Hap Player (whatever that means): %d", newWorldError);
		}

		// calculate the width and height of the video
		m_videoWidth = (int)renderRect.right;
		m_videoHeight = (int)renderRect.bottom;

		// create the image 
		Cing::GraphicsType cingPixelFormat = (Cing::GraphicsType)internalFormat;
		m_frameImg.init(m_videoWidth, m_videoHeight, cingPixelFormat);

		// get the texture from the image
		m_texture = m_frameImg.m_quad.getOgreTexture();
	}

	// Associate the GWorld with the movie
	if (result == noErr)
	{
		SetMovieGWorld((Movie)m_movie, (GWorldPtr)m_gWorld, NULL);

		static MovieDrawingCompleteUPP drawCompleteProc = NewMovieDrawingCompleteUPP(MediaPlayerHapDrawComplete);

		SetMovieDrawingCompleteProc((Movie)m_movie, movieDrawingCallWhenChanged, drawCompleteProc, (long)&m_wantsUpdate);
	}

	// Restore the previous GWorld and destroy the temporary one
	SetGWorld(previousGWorld, previousGDH);
	DisposeGWorld(tempGWorld);
    
	bool retValue;
	if (result == noErr)
	{

		/*
		Apply our current state to the movie
		*/
		m_loop = false;
		m_loopPending = false;
		if (m_playing) 
			this->play();
		this->setSpeed(m_speed);

		retValue = true;
	}
	else
	{
		this->close();
		retValue = false;
	}

	LOG_EXIT_FUNCTION;

	return retValue;
}

void MediaPlayerHap::close()
{
    if (m_movie)
    {
        DisposeMovie((Movie)m_movie);
        m_movie = NULL;
        m_wantsUpdate = false;
        m_hapAvailable = false;
        m_totalNumFrames = -1;
        m_lastKnownFrameNumber = 0;
        m_lastKnownFrameTime = 0;
    }
}


/**
* Releases the object's resources
*/
void MediaPlayerHap::end()
{
	LOG_ENTER_FUNCTION;
	
	// Check if already released
	if ( !isValid() )
		return;

	// Close any loaded movie
	close();

#if defined(TARGET_WIN32)
	// Balance our call to InitializeQTML()
	TerminateQTML();
#endif

	// Dispose of our GWorld and buffer if we created them
	if (m_gWorld)
	{
		DisposeGWorld((GWorldPtr)m_gWorld);
		m_gWorld = NULL;
	}
	if (m_buffer)
	{
		delete[] m_buffer;
		m_buffer = NULL;
	}
			
	// Clear flags
	m_bIsValid			= false;
	m_loopPending		= false;
	m_paused			= false;
	m_playing			= false;

	LOG_EXIT_FUNCTION;
}

/**
	* Updates media playback state
	*/
void MediaPlayerHap::update( unsigned int forceFrame /* = -1 */, bool updateTexture /* = false */ )
{
	LOG_ENTER_FUNCTION;
	
	// Check if we have to loop
	if ( m_loopPending )
	{
		jump(0);
		m_loopPending = false;
	}

	if ( forceFrame > -1 )
		jump(forceFrame/m_nFrames*m_videoFps);

    if (m_movie) 
		MoviesTask((Movie)m_movie, 0);

	m_wantsUpdate = true;

	//if ( m_newBufferReady )
	//	copyBufferIntoImage();

	LOG_EXIT_FUNCTION;
}

/**
	* Returns the internal Image (that contains the last buffer coming from the stream)
	*/
Image& MediaPlayerHap::getImage()
{		
	LOG_ENTER_FUNCTION;
	
	// Check if video is ok
	if ( !isValid() )
	{
		LOG_ERROR_NTIMES( 5, "MediaPlayerHap not corretly initialized. No valid will be returned" );
	}
	else
	{
		// Update, just in case there are pending operations
		update();

		// Check if we have a new buffer to copy
		//if ( m_newBufferReady )
			//copyBufferIntoImage();
	}

	LOG_EXIT_FUNCTION;

	return m_frameImg;
}



/**
	* Returns true if the media is playing, false otherwise     
	**/
bool MediaPlayerHap::isPlaying()
{
	LOG_ENTER_FUNCTION;
	
	// Check if video is ok
	if ( !isValid() )
	{
		LOG_ERROR_NTIMES( 5, "MediaPlayerHap not corretly initialized" );
		return false;
	}

	LOG_EXIT_FUNCTION;

	return m_playing;
}

/**
	* Returns true if the media is paused, false otherwise     
	**/
bool MediaPlayerHap::isPaused()
{
	LOG_ENTER_FUNCTION;
	
	// Check if video is ok
	if ( !isValid() )
	{
		LOG_ERROR_NTIMES( 5, "MediaPlayerHap not corretly initialized" );
		return false;
}

	LOG_EXIT_FUNCTION;

	return m_paused;
}


/**
	* Returns the location of the play head in seconds (that is, the time it has played)     
	**/
float MediaPlayerHap::time()
{
	LOG_ENTER_FUNCTION;
	
	float pos = getPosition();

	LOG_EXIT_FUNCTION;

	return pos;
}

/**
	* Plays the media with no loop
	**/
void MediaPlayerHap::play()
{
	LOG_ENTER_FUNCTION;
	
	// Check if video is ok
	if ( !isValid() )
	{
		LOG_ERROR_NTIMES( 5, "MediaPlayerHap not corretly initialized. File will not play" );
		return;
	}

    if (m_movie)
    {
        if (!GetMovieActive((Movie)m_movie))
	        StartMovie((Movie)m_movie);
        SetMovieRate((Movie)m_movie, MediaPlayerHapFloatToFixed(m_speed));
    }

	// If we were not in pause, reset the playhead to the beginning
	if ( !m_paused )
		jump(0);

	// no loop mode
	m_loop		= false;
	m_paused	= false;
	m_playing	= true;
	m_endOfFileReached	= false;

	LOG_EXIT_FUNCTION;
}

/**
	* Plays the media with no loop
	**/
void MediaPlayerHap::loop()
{
	LOG_ENTER_FUNCTION;
	
	// play normally 
	play();

	// then set the loop flag
	setLoop(true);

	LOG_EXIT_FUNCTION;
}

/**
	* Plays the media with no loop
	**/
void MediaPlayerHap::stop()
{
	LOG_ENTER_FUNCTION;
	
	// Check if video is ok
	if ( !isValid() )
	{
		LOG_ERROR_NTIMES( 5, "MediaPlayerHap not corretly initialized. File will not stop (as it is not playing)" );
		return;
	}

    if (m_movie)
    {
        StopMovie((Movie)m_movie);
    }
		
	m_paused	= false;
	m_playing	= false;
	m_loop		= false;

	LOG_EXIT_FUNCTION;
}

/**
	* Plays the media with no loop
	**/
void MediaPlayerHap::pause()
{
	LOG_ENTER_FUNCTION;
	
	// Check if video is ok
	if ( !isValid() )
	{
		LOG_ERROR_NTIMES( 5, "MediaPlayerHap not corretly initialized. File will not pause (as it is not playing)" );
		return;
	}

    if (m_movie)
    {
        if (GetMovieActive((Movie)m_movie))
        {
            SetMovieRate((Movie)m_movie, MediaPlayerHapFloatToFixed(0.0));
        }
    }

	// pause the media
	m_paused	= true;
	m_playing	= false;

	LOG_EXIT_FUNCTION;
}

/**
	* Jumps to a specific location within a movie (specified in seconds)
	* @param whereInSecs Where to jump in the movie (in seconds)
	**/
void MediaPlayerHap::jump( float whereInSecs )
{
	LOG_ENTER_FUNCTION;
	
	// Check if video is ok
	if ( !isValid() )
	{
		LOG_ERROR_NTIMES( 5, "MediaPlayerHap not corretly initialized. File will not jump" );
		return;
	}

    if (m_movie)
    {
		// Clamp time position
		whereInSecs = constrain( whereInSecs, 0.0f, duration() );

		// set the time
        SetMovieTimeValue((Movie)m_movie, whereInSecs);
    }

	LOG_EXIT_FUNCTION;
}

/**
	* Jumps to a specific location within a movie (specified in frame number)
	* @param whereInSecs Where to jump in the movie (in seconds)
	**/
void MediaPlayerHap::jumpToFrame	( unsigned int frameNumber )
{
	LOG_ENTER_FUNCTION;
	
	// If we are already at that frame, don't jump
	// NOTE: this is cause otherwise Gstreamer throws this error: "(qtdemux10): This file contains no playable streams."
	unsigned int currentFrame = currentFrameNumber();
	if ( currentFrame ==	 frameNumber )
	{
		LOG_TRIVIAL( "MediaPlayerHap::jumpToFrame. Not jumping, already at frame %d", frameNumber );
		return;
	}

	// Clamp time position
	frameNumber = constrain( frameNumber, 0.0, frameCount()-1 );

	// Calculate time in seconds for this frame
	double whereInSecs = (double)frameNumber / fps();
		
	// Jump to that second
	jump( whereInSecs );

	LOG_EXIT_FUNCTION;

}

/**
	* Sets the relative playback speed of the movie (
	* Examples: 1.0 = normal speed, 2.0 = 2x speed, 0.5 = half speed
	* @param rate Speed rate to play the movie
	**/
void MediaPlayerHap::speed( float rate )
{
	LOG_ENTER_FUNCTION;
	
	// Check if video is ok
	if ( !isValid() )
	{
		LOG_ERROR_NTIMES( 5, "MediaPlayerHap not corretly initialized" );
		return;
	}
		
	setSpeed(rate);

	LOG_EXIT_FUNCTION;

}

/**
	* Toggles the audio mute                                                                    
	**/
void MediaPlayerHap::toggleMute()
{
	LOG_ENTER_FUNCTION;
	
	// Check if video is ok
	if ( !isValid() )
	{
		LOG_ERROR_NTIMES( 5, "MediaPlayerHap not corretly initialized" );
		return;
	}
		
	// Toggle mute
	m_mute = !m_mute;

	if ( m_mute )
	{
		// if mute -> set volume to 0
		setVolume( 0 );
	}
	else
	{
		// otherwise -> reset previous volume
		setVolume( m_volume );
	}

	LOG_EXIT_FUNCTION;
}	

/**
	* Builds an absolute path to the file that will be loaded into the player
	* @return true if there was no problem
	*/
bool MediaPlayerHap::buildPathToFile( const String& path )
{
	LOG_ENTER_FUNCTION;
	
	// Store incomming path as the filename (if it's a local file, the use would have passed the file path
	// relative to the data folder)
	m_fileName = path;

	// Check if it is a network path
	if ( m_fileName.find( "://" ) != std::string::npos )
		m_filePath = m_fileName;
		
	// It looks like a local path -> check if it exists
	else
	{		
		m_filePath = dataFolder + path;
		if ( !fileExists( m_filePath ) )
		{
			LOG_ERROR( "MediaPlayer: File %s not found in data folder.", path.c_str() );
			LOG_ERROR( "Absolute path to file: %s", m_filePath.c_str() );
			return false;
		}

		// Build uri in the formtat tha GStreamer expects it
		m_filePath = "file:///" + m_filePath;
	}

	LOG_EXIT_FUNCTION;

	return true;
}


/**
	* Copies the last buffer that came from the stream into the Image (drawable)
	*/
/*void MediaPlayerHap::copyBufferIntoImage()
{
	LOG_ENTER_FUNCTION;

	// Check if video is ok
	if ( !isValid() || !m_newBufferReady)
		return;

	// Set image data (and upload it to the texture)
	m_frameImg.setData( m_internalBuffer, m_videoWidth, m_videoHeight, m_frameImg.getFormat(), m_widthStep );
	m_frameImg.updateTexture();

	// Clear new buffer flag
	m_newBufferReady	= false;
		
	LOG_EXIT_FUNCTION;
}*/

void MediaPlayerHap::draw(float x, float y)
{
    draw(x,y, getWidth(), getHeight());
}

void MediaPlayerHap::draw(float x, float y, float w, float h)
{
	Ogre::TexturePtr t = getTexture();
	m_frameImg.draw(x, y, 0.0f, w, h);
/*    ofTexture *t = getTexture();
    ofShader *sh = getShader();
    if (sh)
    {
        sh->begin();
    }
    t->draw(x,y,w,h);
    if (sh)
    {
        sh->end();
    }*/
}

void MediaPlayerHap::setPaused(bool pause)
{
    if (m_movie)
    {
        if (pause)
        {
            if (GetMovieActive((Movie)m_movie))
            {
                SetMovieRate((Movie)m_movie, MediaPlayerHapFloatToFixed(0.0));
            }
        }
        else
        {
            if (!GetMovieActive((Movie)m_movie))
            {
                StartMovie((Movie)m_movie);
            }
            SetMovieRate((Movie)m_movie, MediaPlayerHapFloatToFixed(m_speed));
        }
    }
    m_paused = pause;
}

bool MediaPlayerHap::isFrameNew()
{
    return m_wantsUpdate;
}

void MediaPlayerHap::setLoop(bool loop)
{
    if (m_movie)
    {
        TimeBase movieTimeBase = GetMovieTimeBase((Movie)m_movie);
        long flags = GetTimeBaseFlags(movieTimeBase);

		if (loop)
		{
            SetMoviePlayHints((Movie)m_movie, 0, hintsPalindrome | hintsLoop);
            flags &= ~loopTimeBase;
            flags &= ~palindromeLoopTimeBase;
		}
		else
		{
            SetMoviePlayHints((Movie)m_movie, hintsLoop, hintsLoop);
            SetMoviePlayHints((Movie)m_movie, 0, hintsPalindrome);
            flags |= loopTimeBase;
            flags &= ~palindromeLoopTimeBase;
		}

        SetTimeBaseFlags(movieTimeBase, flags);
    }

	// always update the loop state too
	m_loop = loop;
}

float MediaPlayerHap::getSpeed()
{
    return m_speed;
}

void MediaPlayerHap::setSpeed(float speed)
{
    if (m_movie && m_playing)
    {
        SetMovieRate((Movie)m_movie, MediaPlayerHapFloatToFixed(speed));
    }
    m_speed = speed;
}

float MediaPlayerHap::getDuration()
{
    if (m_movie)
    {
        TimeValue duration = GetMovieDuration((Movie)m_movie);
        TimeScale timescale = GetMovieTimeScale((Movie)m_movie);
        return (float)duration / (float)timescale;
    }
    return 0.0;
}

float MediaPlayerHap::getPosition()
{
    if (m_movie)
    {
        TimeValue duration = GetMovieDuration((Movie)m_movie);
        if (duration != 0)
        {
            return (float)GetMovieTime((Movie)m_movie, NULL) / (float)duration;
        }
    }
    return 0.0;
}

void MediaPlayerHap::setPosition(float pct)
{
    if (m_movie)
    {
		pct = Cing::clamp<float>(pct, 0.0, 1.0);
        TimeValue duration = GetMovieDuration((Movie)m_movie);
        
        SetMovieTimeValue((Movie)m_movie, (float)duration * pct);
    }
}

void MediaPlayerHap::setVolume(float volume)
{
    if (m_movie)
    {
        SetMovieVolume((Movie)m_movie, (short) (volume * 256));
    }
}

void MediaPlayerHap::setFrame(int frame)
{
    if (m_movie)
    {
        TimeValue time = m_lastKnownFrameTime;
        int search = m_lastKnownFrameNumber;
        OSType mediaType = VideoMediaType;
        Fixed searchDirection = search > frame ? MediaPlayerHapFloatToFixed(-1.0) : MediaPlayerHapFloatToFixed(1.0);
        int searchIncrement = search > frame ? -1 : 1;
        while (search != frame && time != -1)
        {
            TimeValue nextTime = 0;
            GetMovieNextInterestingTime((Movie)m_movie,
                                        nextTimeMediaSample,
                                        1,
                                        &mediaType,
                                        time,
                                        searchDirection,
                                        &nextTime,
                                        NULL);
            if (nextTime == -1)
            {
                break;
            }
            time = nextTime;
            search += searchIncrement;
            m_lastKnownFrameTime = time;
            m_lastKnownFrameNumber = search;
        }
        SetMovieTimeValue((Movie)m_movie, time);
    }
}

int MediaPlayerHap::getCurrentFrame()
{
    int frameNumber = 0;
    if (m_movie)
    {
        TimeValue now = GetMovieTime((Movie)m_movie, NULL);
        OSType mediaType = VideoMediaType;
        /*
         Find the actual time of the current frame
         */
        GetMovieNextInterestingTime((Movie)m_movie,
                                    nextTimeMediaSample | nextTimeEdgeOK,
                                    1,
                                    &mediaType,
                                    now,
                                    (m_speed > 0.0 ? MediaPlayerHapFloatToFixed(-1.0) : MediaPlayerHapFloatToFixed(1.0)),
                                    &now,
                                    NULL);
        /*
         Step through frames until we get to the current frame
         */
        TimeValue searched = m_lastKnownFrameTime;
        frameNumber = m_lastKnownFrameNumber;
        Fixed searchDirection = searched > now ? MediaPlayerHapFloatToFixed(-1.0) : MediaPlayerHapFloatToFixed(1.0);
        int searchIncrement = searched > now ? -1 : 1;
        while (searched != now && searched != -1)
        {
            GetMovieNextInterestingTime((Movie)m_movie,
                                        nextTimeMediaSample,
                                        1,
                                        &mediaType,
                                        searched,
                                        searchDirection,
                                        &searched,
                                        NULL);
            if (searched != -1)
            {
                frameNumber += searchIncrement;
            }
        }
    }
    return frameNumber;
}

int MediaPlayerHap::getTotalNumFrames()
{
    int frameCount = 0;
    if (m_movie)
    {
        if (m_totalNumFrames == -1)
        {
            TimeValue searched = m_lastKnownFrameTime;
            frameCount = m_lastKnownFrameNumber;
            OSType mediaType = VideoMediaType;
            while (searched != -1)
            {
                GetMovieNextInterestingTime((Movie)m_movie,
                                            nextTimeMediaSample,
                                            1,
                                            &mediaType,
                                            searched,
                                            MediaPlayerHapFloatToFixed(1.0),
                                            &searched,
                                            NULL);
                frameCount++;
            }
            m_totalNumFrames = frameCount;
        }
        else
        {
            frameCount = m_totalNumFrames;
        }
    }
    return frameCount;
}

Ogre::TexturePtr MediaPlayerHap::getTexture()
{
    if (m_wantsUpdate && m_gWorld != NULL)
    {
        PixMapHandle pmap = GetGWorldPixMap((GWorldPtr)m_gWorld);
        LockPixels(pmap);

        if (isDXTPixelFormat((*pmap)->pixelFormat))
        {
			Ogre::PixelFormat internalFormat;
            size_t dataLength;
            switch ((*pmap)->pixelFormat)
            {
                case kHapPixelFormatTypeRGB_DXT1:
                    internalFormat = Ogre::PF_DXT1;
                    dataLength = (*pmap)->bounds.right * (*pmap)->bounds.bottom / 2;
                    break;
                case kHapPixelFormatTypeRGBA_DXT5:
                case kHapPixelFormatTypeYCoCg_DXT5:
                default:
                    internalFormat = Ogre::PF_DXT5;
                    dataLength = (*pmap)->bounds.right * (*pmap)->bounds.bottom;
                    break;
            }
            
			// get the pixel buffer for the texture
			m_texture = Ogre::TextureManager::getSingleton().getByName(m_frameImg.m_quad.getTextureName());
			Ogre::HardwarePixelBufferSharedPtr pixelBuffer = m_texture->getBuffer();

			// copy the data from the video
			pixelBuffer->blitFromMemory(Ogre::PixelBox((*pmap)->bounds.right, (*pmap)->bounds.bottom, 0, internalFormat, (*pmap)->baseAddr));
        }
        else
        {
            // TODO: regular upload using ofTexture's load stuff
        }

        UnlockPixels(pmap);
        m_wantsUpdate = false;
    }
    return m_texture;
}


} // namespace
