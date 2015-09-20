/*
 *  VideoInput.h
 *  msaCamView
 *
 *  Created by Mehmet Akten on 04/07/2010.
 *  Copyright 2010 MSA Visuals Ltd. All rights reserved.
 *
 */

#pragma once

//#define USE_OFXLIBDC

#include "ofMain.h"
#include "MSACore.h"
#ifdef USE_OFXLIBDC
#include "ofxLibdc.h"
#endif

#include "ofxSimpleGuiToo.h"

namespace msa {
    
	class VideoInput {//: public ofBaseVideo {
	public:
		VideoInput();
		~VideoInput();
        
        enum {
            kVideoGrabber,
            kVideoPlayer,
            kofxLibdc,
        } inputType;
        
		struct {
			ofBaseVideo			*current;				// points to one of below
            
			ofVideoPlayer       *player;
			ofVideoGrabber		*ofGrabber;
#ifdef USE_OFXLIBDC
            ofxLibdc::Grabber    *libdcGrabber;
#endif
		} inputDevice;
        
		bool	enabled;
		bool	doDraw;
		bool	doDrawGrid;
//		bool	useVideo;
//		bool	useLibDC;
		bool	videoSyncFrame;
		bool	useFormat7;
		int		format7Mode;
		int		width;
		int		height;
		bool	captureColor;					// whether we should try to capture Y8 or RGB
		bool	restart;
		bool	showSettings;
        
		struct {
			bool	togglePlay;
			bool	reset;
			bool	forward;
			bool	rewind;
			float	position;
			int		frame;
		} videoControls;
        
		struct {
			Vec2f	size;
			Vec2f	invSize;
            
			float	aspectRatio;
			float	invAspectRatio;
            
			bool	inputIsColor;			// whether capture is Y8 or RGB
			bool	hasNewFrame;
			
			float	captureFPS;
			float	lastCaptureTime;
			
		} info;
        
        struct {
            float brightness;
            float gamma;
            float exposure;
            float shutter;
            float gain;
        } libdcSettings, oldLibdcSettings;   // FIX: crappy way of doing it
        
        
		void reset();
		void setup();
		void update();
		void draw();
        
		void drawUI();
        
		void loadMovie(string s);
		
		void setInputVideofilename(string s);
        
		void clear();
		void initInput();
        
		bool isVideo();
		bool isOfGrabber();
		bool isOfxGrabber();
        
        bool isReady();
		unsigned char *getPixelsData() const;
        ofPixelsRef getPixels() const;
		float getWidth() const;
		float getHeight() const;
        
		void draw(float x, float y, float w, float h);
        
        
        bool isFrameNew() {
            return inputDevice.current && info.hasNewFrame;
        }
        
	protected:
		void			setupUI();
		void			keyPressed(ofKeyEventArgs &e);
		string			inputVideoFilename;
	};
    
}

