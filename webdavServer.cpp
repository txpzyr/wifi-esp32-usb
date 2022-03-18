// 
// 
// 
#include "webdavServer.h"
#include "sdusb.h"

String UrlDecode(char* SRC)
{
    String ret;
    char ch;
    int ii;
    for (size_t i = 0; i < strlen(SRC); i++) {
        if (SRC[i] == 37) {
            char tc[2];
            tc[0] = SRC[i + 1];
            tc[1] = SRC[i + 2];
            sscanf(tc, "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + 2;
        }
        else {
            ret += SRC[i];
        }
    }
    return (ret);
}


void WebdavServerClass::begin()
{
	server.begin(80);
    server.setNoDelay(true);
    //String dirname = "/";
    //Serial.printf("Listing directory: %s\n", dirname);

    //File root = SD.open(dirname);
    //Serial.println(dirname);
    //Serial.println(root);
    //root.close();
}

void WebdavServerClass::handleClient()
{
    if (!client.connected())
    {
        client = server.available();
        client.setNoDelay(true);
        client.setTimeout(5);
    }
    if (client && client.connected()) {
        if (client.available()) 
        {
            Serial.println("new Data......");
            parseHeader(client);
            Serial.println("method: " + method);
            String resp = "";
            if (method.compareTo("OPTIONS") == 0)
            {
                handleOPTIONS(client, resp);
            }
            else if (method.compareTo("PROPFIND") == 0)
            {
                handlePROPFIND(client, resp);
            }
            else if (method.compareTo("PUT") == 0)
            {
                handlePUT(client, resp);
            }
            else if (method.compareTo("LOCK") == 0)
            {
                handleLOCK(client, resp);
            }
            else if (method.compareTo("UNLOCK") == 0)
            {
                handleUNLOCK(client, resp);
            }
            else if (method.compareTo("PROPPATCH") == 0)
            {
                handlePROPPATCH(client, resp);
            }
            else if (method.compareTo("MOVE") == 0)
            {
                handleMOVE(client, resp);
            }
            else if (method.compareTo("DELETE") == 0)
            {
                handleDELETE(client, resp);
            }
            else if (method.compareTo("MKCOL") == 0)
            {
                handleMKCOL(client, resp);
            }
            else if (method.compareTo("GET") == 0)
            {
                handleGET(client, resp);
            }
            else if (method.compareTo("HEAD") == 0)
            {
                handleHEAD(client, resp);
            }
            else
            {
                resp += "HTTP/1.1 200 OK\r\n";
                resp += "Content-type:text/html\r\n";
                resp += "Content-Length: 0\r\n";
                resp += "\r\n";
                client.print(resp);
            }
        }
        else
        {
            yield();
        }
        client.stop();
        //
        //
    }
    else
    {
        if(client)  client.stop();
    }
}

void WebdavServerClass::clearReceive(WiFiClient client)
{
    long dataSize = getLength();
    if (dataSize != 0)
    {
        uint8_t datas[1436];
        while (dataSize > 0)
        {
            int rlen = client.readBytes(datas, 1436);
            dataSize -= rlen;
        }
    }
}

long WebdavServerClass::getLength()
{
    String sLength = getHead("Content-Length");
    if (sLength.isEmpty() || sLength == "") return 0;
    else return sLength.toInt();
}

String WebdavServerClass::getHead(String name)
{
    for (int i = 0; i < headLen; ++i)
    {
        if (headName[i].compareTo(name) == 0)
        {
            return headArg[i];
        }
    }
    return "";
}


void WebdavServerClass::parseHeader(WiFiClient client)
{
    headLen = 0;
    String line = client.readStringUntil('\r');
    client.read();

    int mEnd = line.indexOf(' ');
    int pEnd = line.indexOf(' ', mEnd + 1);

    method = line.substring(0, mEnd);
    String surl = line.substring(mEnd + 1, pEnd);
    strcpy(url, surl.c_str());
    while (true)
    {
        line = client.readStringUntil('\r');
        client.read();
        if (line == NULL || line.length() == 0 || line.isEmpty() || line.charAt(0) == '\r')  break;

        int spIndex = line.indexOf(':');
        if (headLen < MAXHEADLEN - 1)
        {
            headName[headLen] = line.substring(0, spIndex);
            headArg[headLen] = line.substring(line.charAt(spIndex +1) == ' '? spIndex + 2 : spIndex + 1);
            headLen++;
        }
        else
        {
            log_v("head array is full, please change the MAXHEADLEN");
        }
    }

}


void WebdavServerClass::handleGET(WiFiClient client, String& resp)
{
    Serial.printf("handleGET: %s\n", url);

    if (!SD.exists(url)) {
        //Serial.println("no file");
        resp += "HTTP/1.1 404 Not Found\r\n";
        resp += "Content-Length: 0\r\n";
        resp += "\r\n";   
        client.print(resp);
        client.flush();
        return;
    }

    File f = SD.open(url, "rb");

    int dataSize = f.size();
    
    resp += "HTTP/1.1 200 OK\r\n";
    resp += "Content-type:application/octet-stream\r\n";
    resp += "Content-Length: " + String(dataSize) + "\r\n";
    resp += "\r\n";
    client.print(resp);

    char datas[1436];
    while (dataSize > 0)
    {
        int rlen = f.readBytes(datas, 1436);
        client.write(datas, rlen);
        dataSize -= rlen;
    }
    client.flush();
    f.close();
}

void WebdavServerClass::handleMKCOL(WiFiClient client, String& resp)
{
    Serial.printf("handleMKCOL: %s\n", url);

    SD.mkdir(url);
    resp += "HTTP/1.1 201 OK\r\n";
    resp += "Content-type:text/html\r\n";
    resp += "Content-Length: 0\r\n";
    resp += "\r\n";
    client.print(resp);
    client.flush();
}


void WebdavServerClass::handleMOVE(WiFiClient client, String& resp)
{
    Serial.printf("handleMOVE: %s\n", url);

    String dest = getHead("Destination");
    String host = getHead("Host");

    if (!SD.exists(url)) {
        //Serial.println("no file");
        resp += "HTTP/1.1 404 Not Found\r\n";
        resp += "Content-Length: 0\r\n";
        resp += "\r\n";
        client.print(resp);
        client.flush();
        return;
    }

    int start = dest.indexOf(host);
    String dpath = dest.substring(start + host.length());
    //Serial.println(dpath);

    SD.rename(url, dpath);
    resp += "HTTP/1.1 201 OK\r\n";
    resp += "Content-type:text/html\r\n";
    resp += "Content-Length: 0\r\n";
    resp += "\r\n";
    client.print(resp);
    client.flush();
}


void WebdavServerClass::handleDELETE(WiFiClient client, String& resp)
{
    Serial.printf("handleDELETE: %s\n", url);

    if (!SD.exists(url)) {
        //Serial.println("no file");
        resp += "HTTP/1.1 404 Not Found\r\n";
        resp += "Content-Length: 0\r\n";
        resp += "\r\n";
        client.print(resp);
        client.flush();
        return;
    }

    SD.remove(url);
    resp += "HTTP/1.1 201 OK\r\n";
    resp += "Content-type:text/html\r\n";
    resp += "Content-Length: 0\r\n";
    resp += "\r\n";
    client.print(resp);
    client.flush();
}


void WebdavServerClass::handlePROPPATCH(WiFiClient client, String& resp)
{
    Serial.printf("handlePROPPATCH: %s\n", url);
    clearReceive(client);

    String xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?><D:multistatus xmlns:D=\"DAV:\"><D:response xmlns:n3=\"urn:schemas-microsoft-com:\">";
    xml += "<D:href>http://" + WiFi.localIP().toString() + url + "</D:href>";
    xml += "<D:propstat><D:status>HTTP/1.1 424 Failed Dependency</D:status><D:prop><n3:Win32CreationTime/><n3:Win32LastAccessTime/><n3:Win32LastModifiedTime/></D:prop></D:propstat>";
    xml += "</D:response></D:multistatus>";
    resp += "HTTP/1.1 207 Multi-Status\r\n";
    resp += "Content-type:text/xml\r\n";
    resp += "Content-Length: " + String(xml.length()) + "\r\n";
    resp += "\r\n";
    resp += xml;
    client.print(resp);
    client.flush();
}

void WebdavServerClass::handleHEAD(WiFiClient client, String& resp)
{
    Serial.printf("handleHEAD: %s\n", url);

    resp += "HTTP/1.1 200 OK\r\n";
    resp += "Content-type:text/html\r\n";
    resp += "Content-Length: 0\r\n";
    resp += "\r\n";
    client.print(resp);
    client.flush();
}


void WebdavServerClass::handleUNLOCK(WiFiClient client, String& resp)
{
    Serial.printf("handleUNLOCK: %s\n", url);

    resp += "HTTP/1.1 204 No Content\r\n";
    resp += "Content-type:text/html\r\n";
    resp += "Content-Length: 0\r\n";
    resp += "\r\n";
    client.print(resp);
    client.flush();
}


void WebdavServerClass::handleLOCK(WiFiClient client, String& resp)
{
    Serial.printf("handleLOCK: %s\n", url);

    String xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?><D:prop xmlns:D=\"DAV:\"><D:lockdiscovery><D:activelock>";
    xml += "<D:locktype><D:write/></D:locktype>";
    xml += "<D:lockscope><D:exclusive/></D:lockscope>";
    xml += "<D:depth>0</D:depth><D:owner><D:href>TPC\tp</D:href></D:owner>";
    xml += "<D:timeout>Second-3600</D:timeout><D:locktoken><D:href>opaquelocktoken:d2750004-58ac-4375-9e06-228bcf005cbd.bf7901d83458fb64</D:href></D:locktoken>";
    xml += "<D:lockroot><D:href>";
    xml += "http://" + WiFi.localIP().toString() + url;
    xml += "</D:href></D:lockroot>";
    xml += "</D:activelock></D:lockdiscovery></D:prop>";
    resp += "HTTP/1.1 200 OK\r\n";
    resp += "Content-type:text/xml\r\n";
    resp += "Content-Length: " + String(xml.length()) + "\r\n";
    resp += "\r\n";
    resp += xml;
    client.print(resp);
    client.flush();
}



void WebdavServerClass::handlePUT(WiFiClient client, String& resp)
{
    Serial.printf("handlePUT: %s\n", url);

    int dataSize = getLength();

    File f = SD.open(url, "w");
    uint8_t datas[1436];
    while (dataSize>0)
    {
        int rlen = client.readBytes(datas, 1436);
        f.write(datas, rlen);
        dataSize -= rlen;
    }
    f.close();
    resp += "HTTP/1.1 201 OK\r\n";
    resp += "Content-type:text/html\r\n";
    resp += "Content-Length: 0\r\n";
    resp += "\r\n";
    client.print(resp);
}

void WebdavServerClass::handleOPTIONS(WiFiClient client, String &resp)
{
    Serial.println("handleOPTIONS");

    resp += "HTTP/1.1 200 OK\r\n";
    resp += "Allow: OPTIONS, TRACE, GET, HEAD, POST, COPY, PROPFIND, DELETE, MOVE, PROPPATCH, MKCOL, LOCK, UNLOCK\r\n";
    resp += "Publi: OPTIONS, TRACE, GET, HEAD, POST, PROPFIND, PROPPATCH, MKCOL, PUT, DELETE, COPY, MOVE, LOCK, UNLOCK\r\n";
    resp += "DAV: 1,2,3\r\n";
    resp += "Content-type:text/html\r\n";
    resp += "Content-Length: 0\r\n";
    resp += "\r\n";
    client.print(resp);
}


void WebdavServerClass::handlePROPFIND(WiFiClient client, String &resp)
{
    Serial.printf("handlePROPFIND: %s\n", url);
    clearReceive(client);

    if (!SD.exists(url)) {
        //Serial.println("no file");
        resp += "HTTP/1.1 404 Not Found\r\n";
        resp += "Content-Length: 0\r\n";
        resp += "\r\n";   
        client.print(resp);
        return;
    }

    //Serial.println("file found");
    File root = SD.open(url);
    String xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?><D:multistatus xmlns:D=\"DAV:\">";

    //time_t lastTime = root.getLastWrite();
    String filePath = url;
    String fileName = url;
    if (url != "/")
    {
        int last = filePath.lastIndexOf("/");
        fileName = filePath.substring(last+1);
    }
    bool isdir = root.isDirectory();

    xml += "<D:response>";
    xml += "<D:href>http://" + WiFi.localIP().toString() + filePath + "</D:href>";
    xml += "<D:propstat>";
    xml += "<D:status>HTTP/1.1 200 OK</D:status>";
    xml += "<D:prop>";
    xml += isdir ? "<D:getcontenttype/>" : "<D:getcontenttype>text/xml</D:getcontenttype>";
    xml += "<D:getlastmodified>";
    xml += "Mon, 07 Mar 2022 09:30:16 GMT";
    xml += "</D:getlastmodified>";
    xml += "<D:lockdiscovery/>";
    xml += "<D:ishidden>0</D:ishidden>";
    xml += "<D:supportedlock><D:lockentry><D:lockscope><D:exclusive/></D:lockscope><D:locktype><D:write/></D:locktype></D:lockentry><D:lockentry><D:lockscope><D:shared/></D:lockscope><D:locktype><D:write/></D:locktype></D:lockentry></D:supportedlock>";
    xml += "<D:getetag/>";
    xml += "<D:displayname>" + fileName + "</D:displayname>";
    xml += "<D:getcontentlanguage/>";
    xml += "<D:getcontentlength>";
    xml += isdir ? "0" : String(root.size());
    xml += "</D:getcontentlength>";
    xml += "<D:iscollection>";
    xml += isdir;
    xml += "</D:iscollection>";
    xml += "<D:creationdate>2022-03-06T13:28:31.286Z</D:creationdate>";
    xml += isdir ? "<D:resourcetype><D:collection/></D:resourcetype>" : "<D:resourcetype/>";
    xml += "</D:prop>";
    xml += "</D:propstat>";
    xml += "</D:response>";


    if (getHead("Depth").charAt(0) == '1') {
        File file = root.openNextFile();

        while (file) {
            //Serial.println("in file.....");
            fileName = file.name();
            //Serial.println(fileName);
            String surl = String(url);
            if(surl[surl.length()-1] == '/')    filePath = surl + fileName;
            else filePath = surl + "/" + fileName;

            isdir = file.isDirectory();
            xml += "<D:response>";
            xml += "<D:href>http://" + WiFi.localIP().toString() + filePath + "</D:href>";
            xml += "<D:propstat>";
            xml += "<D:status>HTTP/1.1 200 OK</D:status>";
            xml += "<D:prop>";
            xml += isdir ? "<D:getcontenttype/>" : "<D:getcontenttype>text/xml</D:getcontenttype>";
            xml += "<D:getlastmodified>";
            xml += "Mon, 07 Mar 2022 09:30:16 GMT";
            xml += "</D:getlastmodified>";
            xml += "<D:lockdiscovery/>";
            xml += "<D:ishidden>0</D:ishidden>";
            xml += "<D:supportedlock><D:lockentry><D:lockscope><D:exclusive/></D:lockscope><D:locktype><D:write/></D:locktype></D:lockentry><D:lockentry><D:lockscope><D:shared/></D:lockscope><D:locktype><D:write/></D:locktype></D:lockentry></D:supportedlock>";
            xml += "<D:getetag/>";
            xml += "<D:displayname>" + fileName + "</D:displayname>";
            xml += "<D:getcontentlanguage/>";
            xml += "<D:getcontentlength>";
            xml += isdir ? "0" : String(file.size());
            xml += "</D:getcontentlength>";
            xml += "<D:iscollection>";
            xml += "0";
            xml += "</D:iscollection>";
            xml += "<D:creationdate>2022-03-06T13:28:31.286Z</D:creationdate>";
            xml += isdir ? "<D:resourcetype><D:collection/></D:resourcetype>" : "<D:resourcetype/>";
            xml += "</D:prop>";
            xml += "</D:propstat>";
            xml += "</D:response>";
            file.close();
            file = root.openNextFile();
        }

        //Serial.println("end dir.....");
    }

    xml += "</D:multistatus>";
    resp += "HTTP/1.1 207 Multi-Status\r\n";
    resp += "Content-type:text/xml\r\n";
    resp += "Content-Length: " + String(xml.length()) + "\r\n";
    resp += "\r\n";
    resp += xml;
    client.print(resp);

}
