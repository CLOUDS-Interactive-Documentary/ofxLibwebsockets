//
//  Connection.cpp
//  ofxLibwebsockets
//
//  Created by Brett Renfer on 4/11/12.
//  Copyright (c) 2012 Robotconscience. All rights reserved.
//

#include "ofxLibwebsockets/Connection.h"
#include "ofxLibwebsockets/Reactor.h"
#include "ofxLibwebsockets/Protocol.h"

namespace ofxLibwebsockets {

    //--------------------------------------------------------------
    Connection::Connection(){
        reactor = NULL;
        protocol = NULL;
    }

    //--------------------------------------------------------------
    Connection::Connection(Reactor* const _reactor, Protocol* const _protocol, const bool _supportsBinary)
    : reactor(_reactor)
    , protocol(_protocol)
    , ws(NULL)
    , session(NULL)
    , supportsBinary(_supportsBinary)
    //, buf(LWS_SEND_BUFFER_PRE_PADDING+1024+LWS_SEND_BUFFER_POST_PADDING)
    {
        if (_protocol != NULL){
            binary = _protocol->binary;
            bufsize = 1024;
            binaryBufsize = 1024;
            buf = (unsigned char*)calloc(LWS_SEND_BUFFER_PRE_PADDING+bufsize+LWS_SEND_BUFFER_POST_PADDING, sizeof(unsigned char));
            binaryBuf = (unsigned char*)calloc(LWS_SEND_BUFFER_PRE_PADDING+bufsize+LWS_SEND_BUFFER_POST_PADDING, sizeof(unsigned char));
        }
        
    }

    //--------------------------------------------------------------
    Connection::~Connection(){
        delete buf;
    }
    //--------------------------------------------------------------
    void Connection::close() {
        if (reactor != NULL){
            reactor->close(this);
        }
    }
    
    //--------------------------------------------------------------
    std::string Connection::getClientIP(){
        return client_ip;
    }
    
    //--------------------------------------------------------------
    std::string Connection::getClientName(){
        return client_name;
    }
    
    //--------------------------------------------------------------
    void Connection::setupAddress(){
        int fd = libwebsocket_get_socket_fd( ws );
        
        client_ip.resize(128);
        client_name.resize(128);
        
        libwebsockets_get_peer_addresses(fd, &client_name[0], client_name.size(),
                                         &client_ip[0], client_ip.size());
    }

    //--------------------------------------------------------------
    void Connection::send(const std::string& message)
    {
        int n = 0;
        if(message.size() > 4096){
            return;
        }
        if(message.size() > bufsize){
            bufsize = bufsize+1024;
            buf = (unsigned char*)realloc(buf, bufsize + LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING*sizeof(unsigned char));
            cout << "Connection -- received large message, resizing buffer to " << bufsize << endl;
        }
        unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
        
        if (binary)
        {
            //TODO: when libwebsockets has an API supporting something this, we should use it
            if (supportsBinary)
            {
                memcpy(p, message.c_str(), message.size());
                n = libwebsocket_write(ws, p, message.size(), LWS_WRITE_BINARY);
            }
            else {
                int encoded_len;
                //encoded_len = b64_encode_string(message.c_str(), message.size(), (char*)p, buf.size());
                cout<<"encode "<<message<<endl;
                cout<<bufsize<<endl;
                encoded_len = lws_b64_encode_string(message.c_str(), message.size(), (char*)p, bufsize-LWS_SEND_BUFFER_PRE_PADDING-LWS_SEND_BUFFER_POST_PADDING);
                if (encoded_len > 0){
                    n = libwebsocket_write(ws, p, encoded_len, LWS_WRITE_TEXT);
                }
            }
        }
        else {
            memcpy(p, message.c_str(), message.size());
            n = libwebsocket_write(ws, p, message.size(), LWS_WRITE_TEXT);
        }
        
        if (n < 0)
            std::cout << "ERROR writing to socket" << std::endl;
    }
        
    //--------------------------------------------------------------
    void Connection::sendBinary( char * data, unsigned int size ){
        //memset( binaryBuf, 0, binaryBufsize );
        binaryBufsize = size;
        free( binaryBuf );
        binaryBuf = (unsigned char*)calloc(LWS_SEND_BUFFER_PRE_PADDING+bufsize+LWS_SEND_BUFFER_POST_PADDING, sizeof(unsigned char));
        //(unsigned char*)realloc(binaryBuf, binaryBufsize + LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING*sizeof(unsigned char));
        
        ofSleepMillis(50);
        
        memcpy(&binaryBuf[LWS_SEND_BUFFER_PRE_PADDING], data, size );
        
        int n = -1;
        if ( ws != NULL ){
            n = libwebsocket_write(ws, binaryBuf, size, LWS_WRITE_BINARY);
        }
        
        if (n < 0){
            std::cout << "ERROR writing to socket" << std::endl;
        }
    }
    
    //--------------------------------------------------------------
    const std::string Connection::recv(const std::string& message) {
        std::string decoded = message;
        
        //TODO: when libwebsockets has an API
        // to detect binary support, we should use it
        if (binary && !supportsBinary)
        {
            //TODO: libwebsockets base64 decode is broken @2011-06-19
            //len = lws_b64_decode_string(message, decoded, len);
            int decoded_len = lws_b64_decode_string(message.c_str(), &decoded[0], message.size());
            decoded.resize(decoded_len);
        }
        
        return decoded;
    }
}


