/*
 *  VideoInput.cpp
 *  msaCamView
 *
 *  Created by Mehmet Akten on 04/07/2010.
 *  Copyright 2010 MSA Visuals Ltd. All rights reserved.
 *
 */

#include "MSACore.h"
#include "MSAVideoInput.h"

namespace msa {
    
    //--------------------------------------------------------------
	VideoInput::VideoInput() {
		ofLog(OF_LOG_VERBOSE, "VideoInput::VideoInput");
        
		memset(&inputDevice, 0, sizeof(inputDevice));
		memset(&info, 0, sizeof(info));
		memset(&videoControls, 0, sizeof(videoControls));
		
		setInputVideofilename("inputVideo.mov");
        
        inputType = kVideoGrabber;
	}
    
    
    //--------------------------------------------------------------
    VideoInput::~VideoInput() {
		ofLog(OF_LOG_VERBOSE, "VideoInput::~VideoInput");
	}
    
    
    //--------------------------------------------------------------
	void VideoInput::setupUI() {
		gui.saveToXML();
        
		gui.addPage("VIDEO INPUT").setXMLName("settings/VideoInput/VideoInput.xml");
		gui.addToggle("enabled", enabled);
		gui.addToggle("doDraw", doDraw);
		gui.addToggle("doDrawGrid", doDrawGrid);
		
		int maxWidth = 1280;
		int maxHeight = maxWidth * 3.0f / 4;
        
        string typeNames[] = {  "VideoGrabber", "VideoPlayer", "ofxLibDc"};
		gui.addComboBox("inputType", (int&)inputType, 3, (string*)typeNames);
        
		gui.addToggle("videoSyncFrame", videoSyncFrame);
		gui.addTitle("FORMAT7");
		gui.addToggle("useFormat7", useFormat7);
		gui.addSlider("format7Mode", format7Mode, 0, 2);
        
		gui.addTitle("DIMENSIONS");
		gui.addToggle("captureColor", captureColor);
		gui.addSlider("width", width, 320, maxWidth);
		gui.addSlider("height", height, 180, maxHeight);
		gui.addButton("restart", restart);
        
		gui.addTitle("INFO");
		gui.addSlider("captureFPS", info.captureFPS, 0, 120);
        
        
		gui.loadFromXML();
        
		if(width == 0) width = 640;
		if(height == 0) height = 480;
		restart = false;
        
		gui.saveToXML();
        
        
	}
    
    
    //--------------------------------------------------------------
	void VideoInput::clear() {
		ofLog(OF_LOG_VERBOSE, "VideoInput::clear");
        
		DelPointer(inputDevice.player);
		DelPointer(inputDevice.ofGrabber);
#ifdef USE_OFXLIBDC
        DelPointer(inputDevice.libdcGrabber);
#endif
        memset(&info, 0, sizeof(info));
        
	}
    
    
    //--------------------------------------------------------------
	void VideoInput::initInput() {
		ofLog(OF_LOG_VERBOSE, "VideoInput::initInput");
        
		clear();
		
		bool useOFGrabber = false;
		
		switch(inputType) {
            case kofxLibdc: {
#ifdef USE_OFXLIBDC
                ofLog(OF_LOG_VERBOSE, "   using ofxLibdc");
                inputDevice.current = inputDevice.libdcGrabber = new ofxLibdc::Grabber();
                
                inputDevice.libdcGrabber->setImageType(captureColor ? OF_IMAGE_COLOR : OF_IMAGE_GRAYSCALE);
                inputDevice.libdcGrabber->setFormat7(useFormat7, format7Mode);
                //            inputDevice.libdcGrabber->set
                inputDevice.libdcGrabber->setup();
                
                oldLibdcSettings.brightness = -1;
                oldLibdcSettings.gamma = -1;
                oldLibdcSettings.exposure = -1;
                oldLibdcSettings.shutter = -1;
                oldLibdcSettings.gain = -1;
                
                gui.saveToXML();
                gui.addPage("ofxLibdc").setXMLName("settings/ofxLibdc.xml");
                gui.addSlider("brightness", libdcSettings.brightness, 0, 1);
                gui.addSlider("gamma", libdcSettings.gamma, 0, 1);
                gui.addSlider("exposure", libdcSettings.exposure, 0, 1);
                gui.addSlider("shutter", libdcSettings.shutter, 0, 1);
                gui.addSlider("gain", libdcSettings.gain, 0, 1);
                gui.loadFromXML();
#endif
            }
                break;
                
            case kVideoPlayer: {
                ofLog(OF_LOG_VERBOSE, "   using ofVideoPlayer");
                
                inputDevice.current = inputDevice.player = new ofVideoPlayer;
                
                if(inputDevice.player->isLoaded() == false) {
                    //				setDataPathToBundle();
                    if(inputDevice.player->load(inputVideoFilename) == false) {
                        printf("Could not find inputVideo.mov\n");
                        string videoFilename = "inputVideo" + ofToString(width, 0) + "x" + ofToString(height, 0) + ".mov";
                        printf("trying %s\n", videoFilename.c_str());
                        bool bLoaded = inputDevice.player->load(videoFilename);
                        if(bLoaded == false) useOFGrabber = true;
                        printf("%s\n", bLoaded ? "SUCCESS" : "FAIL");
                    }
                    //				restoreDataPath();
                }
                width = inputDevice.player->getWidth();
                height = inputDevice.player->getHeight();
                inputDevice.player->play();
            }
                break;
                
            default:
                useOFGrabber = true;
        }
        
        if(useOFGrabber) {
            ofLog(OF_LOG_VERBOSE, "   using ofVideoGrabber");
            if(width == 0) width = 640;
            if(height == 0) height = 480;
            
            inputDevice.current = inputDevice.ofGrabber = new ofVideoGrabber;
            inputDevice.ofGrabber->listDevices();
            inputDevice.ofGrabber->initGrabber(width, height);
        }
    }
    
    //--------------------------------------------------------------
    void VideoInput::setup() {
        ofLog(OF_LOG_VERBOSE, "VideoInput::setup");
        setupUI();
        initInput();
//        ofAddListener(ofEvents().keyPressed, this, &VideoInput::keyPressed);
    }
    
    //--------------------------------------------------------------
    void VideoInput::reset() {
        ofLog(OF_LOG_VERBOSE, "VideoInput::reset");
        if(inputType == kVideoPlayer) {
            inputDevice.player->firstFrame();
            inputDevice.player->play();
            videoControls.reset = true;
        }
    }
    
    //--------------------------------------------------------------
    void VideoInput::update() {
        if(!enabled) return;
        
        if(inputDevice.current == NULL) {
            ofLogWarning() << "VideoInput::update - inputDevice.current == NULL";
            return;
        }
        
        if(isReady()) {
#ifdef USE_OFXLIBDC
            if(inputType == kofxLibdc) {
                if(oldLibdcSettings.brightness!=libdcSettings.brightness) {
                    ofLogNotice() << "VideoInput::update - setBrightness " << libdcSettings.brightness << " at frame " << ofGetFrameNum();
                    inputDevice.libdcGrabber->setBrightness(oldLibdcSettings.brightness=libdcSettings.brightness);
                }
                
                if(oldLibdcSettings.gamma!=libdcSettings.gamma) {
                    ofLogNotice() << "VideoInput::update - setGamma " << libdcSettings.gamma << " at frame " << ofGetFrameNum();
                    inputDevice.libdcGrabber->setGamma(oldLibdcSettings.gamma=libdcSettings.gamma);
                }
                
                if(oldLibdcSettings.exposure!=libdcSettings.exposure) {
                    ofLogNotice() << "VideoInput::update - setExposure " << libdcSettings.exposure << " at frame " << ofGetFrameNum();
                    inputDevice.libdcGrabber->setExposure(oldLibdcSettings.exposure=libdcSettings.exposure);
                }
                
                if(oldLibdcSettings.shutter!=libdcSettings.shutter) {
                    ofLogNotice() << "VideoInput::update - setShutter " << libdcSettings.shutter << " at frame " << ofGetFrameNum();
                    inputDevice.libdcGrabber->setShutter(oldLibdcSettings.shutter=libdcSettings.shutter);
                }
                
                if(oldLibdcSettings.gain!=libdcSettings.gain) {
                    ofLogNotice() << "VideoInput::update - setGain " << libdcSettings.gain << " at frame " << ofGetFrameNum();
                    inputDevice.libdcGrabber->setGain(oldLibdcSettings.gain=libdcSettings.gain);
                }
            }
#endif
        }
        
        inputDevice.current->update();
        info.hasNewFrame = inputDevice.current->isFrameNew();// | useVideo;
        
        if(info.hasNewFrame) {
            float nowTime = ofGetElapsedTimef();
            if(nowTime > info.lastCaptureTime) info.captureFPS = 1.0f/(nowTime - info.lastCaptureTime);
            info.lastCaptureTime = nowTime;
        }
        
        if(info.size.x == 0) {
            info.size.x			= inputDevice.current->getPixels().getWidth();
            info.size.y			= inputDevice.current->getPixels().getHeight();
            info.invSize.x		= 1.0f/info.size.x;
            info.invSize.y		= 1.0f/info.size.y;
            info.aspectRatio	= info.size.x/info.size.y;
            info.invAspectRatio	= 1.0f/info.aspectRatio;
            
            if(inputType == kVideoPlayer) info.inputIsColor = true;
            else info.inputIsColor = captureColor;
            
            showSettings = false;
            
            printf("\n Real capture: %f x %f \n inputIsColor: %i \n", info.size.x, info.size.y, info.inputIsColor);
        }
        
        if(info.size.x == 0) {
            return;
        }
        
        if(restart) {
            printf("************\n");
            restart = false;
            initInput();
        }
        
        if(inputDevice.player) {
            static int videoFrameCounter = 0;
            static bool isPlaying = true;
            
            if(videoControls.togglePlay) {
                videoControls.togglePlay = false;
                isPlaying ^= true;
                if(isPlaying) inputDevice.player->play();
                else inputDevice.player->stop();
            }
            if(videoControls.reset) {
                videoControls.reset = false;
                inputDevice.player->firstFrame();
                videoFrameCounter = 0;
            }
            if(videoControls.forward) {
                videoControls.forward = false;
                inputDevice.player->setFrame(inputDevice.player->getCurrentFrame() + 1);
                videoFrameCounter += 1;
            }
            if(videoControls.rewind) {
                videoControls.rewind = false;
                inputDevice.player->setFrame(inputDevice.player->getCurrentFrame() - 1);
                videoFrameCounter -= 1;
            }
            
            if(videoSyncFrame) {
                if(isPlaying) videoFrameCounter = (videoFrameCounter+1) % inputDevice.player->getTotalNumFrames();
                inputDevice.player->setFrame(videoFrameCounter);
            }
            
            videoControls.position = inputDevice.player->getPosition();
            videoControls.frame = inputDevice.player->getCurrentFrame();
        }
        
        if(showSettings) {
            gui.setDraw(false);
            showSettings = false;
            if(inputDevice.ofGrabber) inputDevice.ofGrabber->videoSettings();
        }
        
    }
    
    
    //--------------------------------------------------------------
    void VideoInput::drawUI() {
        if(!enabled) return;
    }
    
    
    
    //--------------------------------------------------------------
    void VideoInput::loadMovie(string s) {
        if(inputDevice.player) inputDevice.player->load(s);
    }
    
    //--------------------------------------------------------------
    void VideoInput::setInputVideofilename(string s) {
        inputVideoFilename = s;
    }
    
    
    
    //--------------------------------------------------------------
//    void VideoInput::keyPressed(ofKeyEventArgs &e) {
//        switch(e.key) {
//            case 's':
//                showSettings = true;
//                break;
//
//            case 'i':
//                doDraw ^= true;
//                break;
//
//        }
//
//        if(inputType == kVideoPlayer) {
//            switch(e.key) {
//                case 'w': videoControls.reset = true; break;
//                case 'v': videoControls.togglePlay = true; break;
//                case '.': videoControls.forward = true; break;
//                case ',': videoControls.rewind = true; break;
//            }
//        }
//    }
    
    
    //--------------------------------------------------------------
    bool VideoInput::isVideo() {
        return inputDevice.player && inputDevice.player == inputDevice.current;
    }
    
    //--------------------------------------------------------------
    bool VideoInput::isOfGrabber() {
        return inputDevice.ofGrabber && inputDevice.ofGrabber == inputDevice.current;
    }
    
    //--------------------------------------------------------------
    bool VideoInput::isOfxGrabber() {
        return false;
    }
    
    
    //--------------------------------------------------------------
    bool VideoInput::isReady() {
        if(inputDevice.current == NULL) return false;
        switch(inputType) {
            case kVideoPlayer: return true;
            case kVideoGrabber: return inputDevice.ofGrabber->isInitialized();
#ifdef USE_OFXLIBDC
            case kofxLibdc: return inputDevice.libdcGrabber->isReady();
#endif
            default:
                return false;
                break;
        }
    }
    
    
    //--------------------------------------------------------------
    unsigned char *VideoInput::getPixelsData() const {
        return inputDevice.current->getPixels().getData();
    }
    
    //--------------------------------------------------------------
    ofPixelsRef VideoInput::getPixels() const {
        return inputDevice.current->getPixels();
    }
    
    //--------------------------------------------------------------
    float VideoInput::getWidth() const {
        return info.size.x;
    }
    
    //--------------------------------------------------------------
    float VideoInput::getHeight() const {
        return info.size.y;
    }
    
    //--------------------------------------------------------------
    void VideoInput::draw(float x, float y, float w, float h) {
        if(doDraw && isReady()) {
            ofPushStyle();
            ofSetColor(255);
            //            inputDevice.current->draw(x, y, w, h);// FIX
            switch(inputType) {
                case kVideoPlayer:  inputDevice.player->draw(x, y, w, h); break;
                case kVideoGrabber: inputDevice.ofGrabber->draw(x, y, w, h); break;
#ifdef USE_OFXLIBDC
                case kofxLibdc: inputDevice.libdcGrabber->draw(x, y, w, h); break;
#endif
                default:
                    break;

            }
            
            
            if(doDrawGrid) {
                ofSetColor(255, 0, 0);
                ofSetLineWidth(3);
                ofPushMatrix();
                ofTranslate(x, y, 0);
                ofDrawLine(w/2, 0, w/2, h);
                ofDrawLine(0, h/2, w, h/2);
                ofSetLineWidth(1);
                ofDrawLine(w/4, 0, w/4, h);
                ofDrawLine(3 * w/4, 0, 3 * w/4, h);
                ofDrawLine(0, h/4, w, h/4);
                ofDrawLine(0, 3 * h/4, w, 3 * h/4);
                ofPopMatrix();
            }
            ofPopStyle();
        }
    }
    
}
