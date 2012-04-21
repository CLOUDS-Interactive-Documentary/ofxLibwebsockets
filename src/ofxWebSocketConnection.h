#pragma once

#include <libwebsockets.h>

#include <vector>
#include <string>

class ofxWebSocketReactor;
class ofxWebSocketProtocol;

class ofxWebSocketSession {
    public:
};

class ofxWebSocketConnection {
    friend class ofxWebSocketReactor;
public:
    ofxWebSocketConnection();
    
    ofxWebSocketConnection(ofxWebSocketReactor* const _reactor=NULL,
                           ofxWebSocketProtocol* const _protocol=NULL,
                           const bool supportsBinary=false);
    
    ~ofxWebSocketConnection();
    void close();
    void send(const std::string& message);
    const std::string recv(const std::string& message);  
    
    libwebsocket* ws;
    ofxWebSocketReactor*  reactor;
    ofxWebSocketProtocol* protocol;
    
protected:
    ofxWebSocketSession*  session;
    
    bool binary;
    bool supportsBinary;
    int buffersize;
    unsigned char* buf;
    int bufsize;
    //std::vector<unsigned char> buf;
};

