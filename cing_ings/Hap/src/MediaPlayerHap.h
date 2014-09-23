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


// Graphics
#include "graphics/Image.h"

// Common
#include "PTypes/include/pasync.h"
#include "boost/smart_ptr/shared_ptr.hpp"


namespace Cing
{
	/**
	* Media player that uses Hap to decode and play video/audio
	*/
	class MediaPlayerHap
	{
	public:

		// Constructor / Destructor
		MediaPlayerHap();
		MediaPlayerHap( const char* filename, float fps = -1 );
		~MediaPlayerHap();

		// Init / Release / Update
		bool    init    ();
		bool    load    ( std::string const& fileName );
		void	close	();
		void    end     ();
		void    update  ( unsigned int forceFrame = -1, bool updateTexture = false );
		Image&  getImage();

		// Query methods
		bool    			isValid   			() const { return m_bIsValid; }
		bool    			isPlaying 			();
		bool				isPaused  			();
		float   			duration  			() const { return (float)m_videoDuration; }
		float   			time      			();
		unsigned int		currentFrameNumber	() { return (unsigned int)floor(time() * m_videoFps); }
		unsigned int    	getWidth  			() const { return m_videoWidth; }
		unsigned int    	getHeight 			() const { return m_videoHeight; }
		float   			fps		  			() const { return m_videoFps; }
		float   			frameRate 			() const { return m_videoFps; }
		unsigned int		frameCount			() const { return m_nFrames; } 
		const std::string&	getFilePath			() const { return m_fileName; }
		bool				hasNewFrame			() const { return true; }
		GraphicsType		getPixelFormat		() const { return m_pixelFormat; }

		// Media control
		void    play    ();
		void    loop    ();
		void    stop    ();
		void    pause   ();
		void    setLoop ( bool loop );
		void    noLoop  () { setLoop( false  ); }
		void    jump		( float whereInSecs );		///< Note: For accurate frame level positioning, videos should have keyframes in all frames.
		void    jumpToFrame	( unsigned int frameNumber );
		void    speed   ( float rate );

		// Audio Specific Control
		void	toggleMute();
		void	setVolume ( float volume );
		float	getVolume () const;

		virtual bool                isFrameNew();
		virtual Ogre::TexturePtr    getTexture();
		//virtual ofShader *          getShader();
    
		//virtual ofPixelFormat       getPixelFormat();
		//virtual bool				getHapAvailable();
	
		//should implement!
		virtual float               getPosition();
		virtual float               getSpeed();
		virtual float               getDuration();
	
		/*
		virtual bool                getIsMovieDone();
		*/
		virtual void                setPaused(bool pause);
		virtual void                setPosition(float percentage);
		virtual void                setSpeed(float speed);
		virtual void                setFrame(int frame);  // frame 0 = first frame...
		virtual int                 getCurrentFrame();
		virtual int                 getTotalNumFrames();
		/*
		virtual void                firstFrame();
		virtual void                nextFrame();
		virtual void                previousFrame();
		*/
		//
		virtual void                draw(float x, float y);
		virtual void                draw(float x, float y, float width, float height);
	private:
		// Internal methods
		bool			buildPathToFile			( const String& path );

		// Media info
		String			m_fileName;			///< Name of the loaded video
		String			m_filePath;			///< Full path of the loaded video
		double         	m_videoDuration;	///< Duration in seconds
		float           m_videoFps;			///< Frames per second of the loaded video
		int				m_nFrames;			///< Total number of frames of the video
		int             m_videoWidth;		///< Width in pixels of the loaded video
		int             m_videoHeight;		///< Height in pixels of the loaded video

		// Attributes copied from ofHapPlayer
		void*           m_movie;
		void*           m_gWorld;
		unsigned char*  m_buffer;
		Ogre::MaterialPtr m_shader;
		Ogre::TexturePtr  m_texture;
		bool            m_shaderLoaded;
		bool            m_wantsUpdate;
		bool			m_hapAvailable;
		int             m_totalNumFrames;
		int             m_lastKnownFrameNumber;
		int             m_lastKnownFrameTime;

		// Playback Settings/Info
		float           m_speed;
		bool            m_loop;				///< If true, the video will loop
		bool			m_loopPending;		///< Loop pending 
		bool			m_endOfFileReached; ///< True when the playback head reached the end of the file
		bool			m_playing;			///< True if the stream is playing (the user called play() or loop() not necessarily that the pipeline is already in playing state)
		bool			m_paused;			///< True if the stream is paused (tue user called pause(), not that the pipeline state is paused)

		// Audio settings	
		bool			m_mute;				///< True if volume is mutted (0)
		float			m_volume;			///< Current audio volume (0..1)

		GraphicsType	m_pixelFormat;		///< Pixel format in which new image frames will be stored
		Image			m_frameImg;			///< Image containing the buffer of the current video frame

		// Internal stuff
		bool            m_bIsValid;	      ///< Indicates whether the class is valid or not. If invalid none of its methods except init should be called.

	};

} // namespace
