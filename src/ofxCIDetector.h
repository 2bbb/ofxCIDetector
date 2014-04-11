//
//  ofxCIDetector.h
//
//  Created by ISHII Tsuubito on 2013/07/17.
//
//

#ifndef __ofxCIDetector__
#define __ofxCIDetector__

#include "ofMain.h"
#ifdef IPHONE
#   import <CoreImage/CoreImage.h>
#else
#   import <AppKit/AppKit.h>
#   import <QuartzCore/QuartzCore.h>
#endif

struct ofxFace {
    ofRectangle rectangle;
    
    bool hasLeftEyePoint;
    ofPoint leftEyePoint;
    
    bool hasRightEyePoint;
    ofPoint rightEyePoint;
    
    bool hasMouthPoint;
    ofPoint mouthPoint;
};

class ofxCIDetector {
public:
    static vector<ofxFace> getFaces(ofImage &image) {
        return getFaces(image.getPixelsRef());
    }
    static vector<ofxFace> getFaces(ofPixelsRef pixels) {
        vector<ofxFace> faces;
        
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        
        int width = pixels.getWidth();
        int height = pixels.getHeight();
        ofImageType imageType = pixels.getImageType();
        
#ifdef IPHONE
        CGColorSpaceRef colorSpace;
        CGContextRef cgContext;
        
        colorSpace = CGColorSpaceCreateDeviceRGB();
        
        if(imageType == OF_IMAGE_COLOR_ALPHA) {
            cgContext = CGBitmapContextCreate(pixels.getPixels(), width, height, 8, width * 4, colorSpace, kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst);
        } else if(imageType == OF_IMAGE_COLOR) {
            unsigned char *buf = pixels.getPixels();
            unsigned char rgba[width * height * 4];
            for(int i = 0; i < width * height; ++i) {
                rgba[4 * i]     = buf[3 * i];
                rgba[4 * i + 1] = buf[3 * i + 1];
                rgba[4 * i + 2] = buf[3 * i + 2];
                rgba[4 * i + 3] = 255;
            }
            cgContext = CGBitmapContextCreate(rgba, width, height, 8, width * 4, colorSpace, kCGImageAlphaNoneSkipLast);
        }
        CGColorSpaceRelease(colorSpace);
        
        // 画像の作成
        CGImageRef cgImage = CGBitmapContextCreateImage(cgContext);
        CIImage *ciImage = [CIImage imageWithCGImage:cgImage];
        CGImageRelease(cgImage);
        CGContextRelease(cgContext);
#else
        NSBitmapImageRep *rep;
        unsigned char *pix = pixels.getPixels();
        if(imageType == OF_IMAGE_COLOR_ALPHA) {
            rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:&pix
                                                          pixelsWide:pixels.getWidth()
                                                          pixelsHigh:pixels.getHeight()
                                                       bitsPerSample:8
                                                     samplesPerPixel:4
                                                            hasAlpha:YES
                                                            isPlanar:NO
                                                      colorSpaceName:NSDeviceRGBColorSpace
                                                        bitmapFormat:NSAlphaNonpremultipliedBitmapFormat
                                                         bytesPerRow:4 * pixels.getWidth()
                                                        bitsPerPixel:32];
        } else if(imageType == OF_IMAGE_COLOR) {
            rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:&pix
                                                          pixelsWide:pixels.getWidth()
                                                          pixelsHigh:pixels.getHeight()
                                                       bitsPerSample:8
                                                     samplesPerPixel:3
                                                            hasAlpha:NO
                                                            isPlanar:NO
                                                      colorSpaceName:NSDeviceRGBColorSpace
                                                        bitmapFormat:NSAlphaNonpremultipliedBitmapFormat
                                                         bytesPerRow:3 * pixels.getWidth()
                                                        bitsPerPixel:24];
        }
        
        // 画像の作成
        CIImage *ciImage = [[CIImage alloc] initWithBitmapImageRep:rep];
#endif
        NSDictionary *options = [NSDictionary dictionaryWithObject:CIDetectorAccuracyHigh
                                                            forKey:CIDetectorAccuracy];
        CIDetector *detector = [CIDetector detectorOfType:CIDetectorTypeFace
                                                  context:nil
                                                  options:options];
        
        
        NSDictionary *fOptions = [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:1]
                                                             forKey:CIDetectorImageOrientation];
        NSArray *features = [detector featuresInImage:ciImage options:fOptions];
        
        for(CIFaceFeature *f in features) {
            ofxFace face;
            
            face.rectangle.set(f.bounds.origin.x, height - f.bounds.origin.y - f.bounds.size.height, f.bounds.size.width, f.bounds.size.height);
            
            face.hasLeftEyePoint = f.hasLeftEyePosition;
            if(f.hasLeftEyePosition) {
                if(isVerbose()) ofLogVerbose() << "has left eye : " << f.leftEyePosition.x << ", " << f.rightEyePosition.y;
                face.leftEyePoint.set(f.leftEyePosition.x, height - f.leftEyePosition.y);
            }
            
            face.hasRightEyePoint = f.hasRightEyePosition;
            if(f.hasRightEyePosition) {
                if(isVerbose()) ofLogVerbose() << "has right eye : " << f.rightEyePosition.x << ", " << f.rightEyePosition.y;
                face.rightEyePoint.set(f.rightEyePosition.x, height - f.rightEyePosition.y);
            }
            
            face.hasMouthPoint = f.hasMouthPosition;
            if(f.hasMouthPosition) {
                if(isVerbose()) ofLogVerbose() << "has mouth : " << f.mouthPosition.x << ", " << f.mouthPosition.y;
                face.mouthPoint.set(f.mouthPosition.x, height - f.mouthPosition.y);
            }
            
            faces.push_back(face);
        }
        
        [pool release];

        
        return faces;
    }
    static void setVerbose(bool b) { bVerbose = b; }
    static bool isVerbose() { return bVerbose; };
private:
    static bool bVerbose;
};

bool ofxCIDetector::bVerbose = false;

#endif /* defined(__emptyExample__ofxCIDetector__) */
