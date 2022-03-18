// webdavServer.h

#ifndef _WEBDAVSERVER_h
#define _WEBDAVSERVER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <WiFi.h>
#include <map>

#define MAXHEADLEN 16

class WebdavServerClass
{
 protected:
	 String method;
	 char url[128];
	 int headLen = 0;
	 String headName[MAXHEADLEN];
	 String headArg[MAXHEADLEN];
	 WiFiServer server;
	 WiFiClient client;
 public:
	void begin();
	void handleClient();
	void handleOPTIONS(WiFiClient client, String &resp);
	void clearReceive(WiFiClient client);
	String getHead(String name);
	long getLength();
	void parseHeader(WiFiClient client);
	void handlePROPFIND(WiFiClient client, String &resp);
	void handlePUT(WiFiClient client, String& resp);
	void handleLOCK(WiFiClient client, String& resp);
	void handleUNLOCK(WiFiClient client, String& resp);
	void handlePROPPATCH(WiFiClient client, String& resp);
	void handleMOVE(WiFiClient client, String& resp);
	void handleDELETE(WiFiClient client, String& resp);
	void handleMKCOL(WiFiClient client, String& resp);
	void handleGET(WiFiClient client, String& resp);
	void handleHEAD(WiFiClient client, String &resp);
};

extern WebdavServerClass WebdavServer;

#endif

