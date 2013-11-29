//
//  AVFVideoReader.h
//  Cing
//
//  Created by Julio Obelleiro on 10/14/13.
//  Copyright (c) 2013 Cing Project. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

#include "graphics/Image.h"


@interface AVFVideoReader : NSObject {
 
    // Video reading
    AVURLAsset                  *_asset;
    AVAssetReader               *_assetReader;
    AVAssetReaderTrackOutput    *_assetReaderOutput;
    
    // Buffers
    uint8_t                     *_frameBuffer;
    Cing::Image                 _frameImg;
    
    // Extracted properties
    float                       _fps;
    float                       _duration;
    unsigned int                _frameCount;
    unsigned int                _width;
    unsigned int                _height;
}

// Load and frame retrieval 
- (Boolean) loadFile:(NSString *)   filename;
- (Cing::Image &)                   getNextFrame;

// Query
@property (assign) float        fps;
@property (assign) float        duration;
@property (assign) unsigned int frameCount;
@property (assign) unsigned int width;
@property (assign) unsigned int height;





@end