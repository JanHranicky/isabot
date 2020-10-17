
//============================================================================
// Name        : SSLClient.cpp
// Compiling   : g++ -c -o SSLClient.o SSLClient.cpp
//               g++ -o SSLClient SSLClient.o -lssl -lcrypto
//============================================================================
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <vector>
#include <string>
#include <regex>
#include <iostream>

#include "jsonParser.hpp"

using namespace std;

SSL *ssl;
int sock;
int timeout = 1000;
int finalLen = 0;

string lastMessgeId;
string stopMessage;

bool goodCode = true;

bool checkCR(char c)
{
    if (c == 13) //CR = 13 ASCII
    {
        return true;
    }
    return false;
}

bool checkLF(char c)
{
    if (c == 10) //LF = 10 ASCII
    {
        return true;
    }
    return false;
}

bool checkZero(char c)
{
    if (c == 48)
    {
        return true;
    }
    return false;
}

void checkForBegginingOfJson(char *buf, int len, string *jsonResponse)
{
    bool inJson = false;

    for (size_t i = 0; i < len; i++)
    {
        if (buf[i] == '[' || buf[i] == '{' || inJson)
        {
            jsonResponse->push_back(buf[i]);
            inJson = true;
        }
    }
}

//CRLFCRLF
bool checkEndOfHead(char *buf, int len, string *jsonResponse)
{
    for (size_t i = 0; i < len; i++)
    {
        if (checkCR(buf[i]) && i + 3 < len)
        {
            if (checkLF(buf[i + 1]))
            {
                if (checkCR(buf[i + 2]))
                {
                    if (checkLF(buf[i + 3]))
                    {
                        finalLen += len;
                        checkForBegginingOfJson(buf, len, jsonResponse);
                        //printf("%s",buf);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}
//0CRLFCRLF
bool checkEndOfMessage(char *buf, int len)
{
    for (size_t i = 0; i < len; i++)
    {
        if (checkZero(buf[i]) && i + 4 < len)
        {
            if (checkCR(buf[i + 1]))
            {
                if (checkLF(buf[i + 2]))
                {
                    if (checkCR(buf[i + 3]))
                    {
                        if (checkLF(buf[i + 4]))
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void addToJsonResonse(char *buf, int len, string *jsonResponse)
{
    for (size_t i = 0; i < len; i++)
    {
        jsonResponse->push_back(buf[i]);
    }
}

bool parseHead(char *buf, int len, string returnCode)
{ //9  10 11
    string returnCodeString = "";
    returnCodeString.push_back(buf[9]);
    returnCodeString.push_back(buf[10]);
    returnCodeString.push_back(buf[11]);

    if (returnCodeString == returnCode)
    {
        return true;
    }
    return false;
}

bool checkNewMessages(string jsonResponse)
{
    string newLast = getLastMessageId(jsonResponse);
    if (lastMessgeId.empty())
    {
        lastMessgeId = newLast;
    }
    else
    {
        if (lastMessgeId != newLast)
        {
            printf("GOING TO READ NEW MESSAGES \n\n");
            stopMessage = lastMessgeId;
            lastMessgeId = newLast;

            printf("last message : %s\n", lastMessgeId.c_str());
            printf("stop message : %s\n", stopMessage.c_str());
            return true; //get new messages
        }
    }
    return false;
}

bool checkCRLFCRLF(char *buf, int len)
{
    for (size_t i = 0; i < len; i++)
    {
        if (checkCR(buf[i + 1]))
        {
            if (checkLF(buf[i + 2]))
            {
                if (checkCR(buf[i + 3]))
                {
                    if (checkLF(buf[i + 4]))
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void readRestOfFailedResponse()
{
    char buf[150];
    int len = 100;

    bool readingHead = true;
    bool readLenght = false;
    string head;
    string contentLenght;
    string test;
    while (1)
    {
        len = SSL_read(ssl, buf, 100);
        buf[len] = 0;

        if (checkCRLFCRLF(buf, len))
        {
            if (checkCRLFCRLF(buf, len))
            {
                return;
            }
        }
    }
}

string RecvPacket()
{
    string jsonResponse;
    int len = 100;
    char buf[1000000];

    bool bodyFlag = false;
    bool parsedHead = false;
    while (1)
    {
        len = SSL_read(ssl, buf, 100);
        buf[len] = 0;
        //printf("%s",buf);

        if (!bodyFlag)
        {
            if (!parsedHead)
            {
                if (!parseHead(buf, len, "200"))
                {
                    goodCode = false;
                    //printf("%s",buf);
                    //printf("BREAKING CAUSE OF BAD CODE \n\n\n");
                    //TODO READ MSG ON SOCKET)
                    readRestOfFailedResponse();
                    break;
                }
                goodCode = true;
                parsedHead = true;
            }

            if (checkEndOfHead(buf, len, &jsonResponse))
            {
                bodyFlag = true;
            }
        }
        else
        {
            if (checkEndOfMessage(buf, len) || len < 0)
            {
                break;
            }
            else
            {
                finalLen += len;
                addToJsonResonse(buf, len, &jsonResponse);
                //printf("%s",jsonResponse.c_str());
            }
        }
    }
    //printf("finallen je : %i + velikost vectoru je : %i\n",finalLen,jsonResponse.size());
    //printf("\n\nfinished reading \nGoing to parseing \n");

    return jsonResponse;

    //printf("%s <- last message id \n",getLastMessageId(jsonResponse).c_str());

    /*
    if (len < 0) {
        int err = SSL_get_error(ssl, len);
    if (err == SSL_ERROR_WANT_READ)
            return 0;
        if (err == SSL_ERROR_WANT_WRITE)
            return 0;
        if (err == SSL_ERROR_ZERO_RETURN || err == SSL_ERROR_SYSCALL || err == SSL_ERROR_SSL)
            return -1;
    }
*/
}

int SendPacket(string request)
{
    const void *a = request.c_str();
    int len = SSL_write(ssl, a, request.length());
    //int len = SSL_write(ssl, &request, request.length());
    if (len < 0)
    {
        int err = SSL_get_error(ssl, len);
        switch (err)
        {
        case SSL_ERROR_WANT_WRITE:
            return 0;
        case SSL_ERROR_WANT_READ:
            return 0;
        case SSL_ERROR_ZERO_RETURN:
        case SSL_ERROR_SYSCALL:
        case SSL_ERROR_SSL:
        default:
            return -1;
        }
    }
}

void log_ssl()
{
    int err;
    while (err = ERR_get_error())
    {
        char *str = ERR_error_string(err, 0);
        if (!str)
            return;
        printf("\n");
        fflush(stdout);
    }
}

void initSSL()
{
    int s;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        printf("Error creating socket.\n");
        exit(-1);
    }
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr("162.159.138.232"); // discord ip
    sa.sin_port = htons(443);
    socklen_t socklen = sizeof(sa);
    if (connect(s, (struct sockaddr *)&sa, socklen))
    {
        printf("Error connecting to server.\n");
        exit(-1);
    }
    SSL_library_init();
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();
    const SSL_METHOD *meth = TLSv1_2_client_method();
    SSL_CTX *ctx = SSL_CTX_new(meth);
    ssl = SSL_new(ctx);

    if (!ssl)
    {
        printf("Error creating SSL.\n");
        log_ssl();
        exit(-1);
    }
    sock = SSL_get_fd(ssl);
    SSL_set_fd(ssl, s);
    int err = SSL_connect(ssl);
    if (err <= 0)
    {
        printf("Error creating SSL connection.  err=%x\n", err);
        log_ssl();
        fflush(stdout);
        exit(-1);
    }
    printf("SSL connection using %s\n", SSL_get_cipher(ssl));
}

int countNewMessages(std::vector<string> messageVector)
{
    for (size_t i = 0; i < messageVector.size(); i++)
    {
        if (messageVector.at(i) == stopMessage)
        {
            return i;
        }
    }
    return messageVector.size();
}

void requestChannelInfo() {
    string request = "GET /api/channels/720745314209890379 HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I\r\nUser-Agent: DiscordBot ($url, $versionNumber)\r\n\r\n";
    SendPacket(request);
}

void requestChannelMessages() {
    string request = "GET /api/channels/720745314209890379/messages HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I\r\nUser-Agent: DiscordBot ($url, $versionNumber)\r\n\r\n";
    SendPacket(request);
}

string getFirstLine(char buf[]) {
    
    string s = "";
    int counter = 0;

    while (buf[counter] != 10)
    {
        s += buf[counter];
        counter ++;
    }

    return s;
}

bool check200OK(string s) {
     std::regex codeRegex("^HTTP/1.1 200 OK");

    if (std::regex_search(s,codeRegex))
    {
        return true;
    }

    return false;        
}

bool checkEndOfMessage(string s) {
    std::regex endRegex("^0\\r\\n\\r\\n");
    std::regex endRegex02("\n}\n");

    if (std::regex_search(s,endRegex) || std::regex_search(s,endRegex02))
    {
        return true;
    }
    
    return false;
    
}

string convertToString(char buf[],int len) {
    string converted;

    for (size_t i = 0; i < len; i++)
    {
        converted.push_back(buf[i]);
    }

    return converted;
}

std::vector<string> splitString(string s, string delimiter) {
    std::vector<string> returnVector;
    int pos = 0;
    string token;

    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        returnVector.push_back(token);
        s.erase(0, pos + delimiter.length());
    }

    return returnVector;
}

string parseChannelInfo() { //returns last message id
    char buf[101];
    int len = 100;

    bool parsedHead = false;
    string messageString;
    while (1)
    {   
        len = SSL_read(ssl, buf, 100);
        buf[len] = 0;
        //printf("%s",buf);
        if (!parsedHead)
        {
            if (!check200OK(convertToString(buf,len)));
            {
                //TODO 
            }
            
            parsedHead = true;
        }
        else {
            string converted = convertToString(buf,len); 
            messageString += converted;
            if (checkEndOfMessage(convertToString(buf,len)))
            {
                //printf("%s",messageString.c_str());
                break;
            }
        }
    }
    //printf("%s",messageString.c_str());

    std::vector<string> parsedResponse;
    parsedResponse = splitString(messageString,"\r\n\r\n");

    return getLastMessageId(parsedResponse.at(1));
}

string getChannelMessages() {
    string channelMessagesResponse;

    char buf[200];
    int len = 100;

    bool code200Ok = true;
    bool checkedHead = false;
    while (1)
    {   
        len = SSL_read(ssl, buf, 100);
        buf[len] = 0;
        //printf("%s",buf);

        if (!checkedHead)
        {
            if (!check200OK(convertToString(buf,len)))
            {
                code200Ok = false;
            }
            checkedHead = true;
        }
        else {
            if(code200Ok) {
                channelMessagesResponse += convertToString(buf,len);
            }
            if (checkEndOfMessage(convertToString(buf,len)))
            {
                if (!channelMessagesResponse.empty())
                {
                    return splitString(channelMessagesResponse,"\r\n\r\n").at(1).c_str();
                }
                
                return channelMessagesResponse;
            }
        }
    }   
}

bool postMessage(string message) {
    string request = "POST /api/channels/720745314209890379/messages HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I\r\nContent-Type: application/json\r\nContent-Length: 153\r\nCache-Control: no-cache\r\n\r\n{\"content\": \"Hello, World!\",\n\"tts\": false,\n\"embed\": {\n\"title\": \"Hello, Embed!\",\n\"description\": \"This is an embedded message.\"\n}\n}\n";
    
    string messageJson = "{\n"
    "  \"content\": \"";
    messageJson.append(message);
    messageJson.append("\",\n"
    " \"tts\": false,\n"
    " \"embed\": \"\"\n"
    "}");
    
    printf("%s",messageJson.c_str());

    int size = messageJson.length();
    string c = to_string(size);

    string req;
    req.append("POST /api/channels/720745314209890379/messages HTTP/1.1\r\n"
"Host: discord.com\r\n"
"Authorization: Bot NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I\r\n"
"Content-Length: ");
req.append(c);
req.append("\r\n"
"Content-Type: application/json\r\n"
"Cache-Control: no-cache\r\n"
"Postman-Token: f4b2e754-2b42-a134-185c-5a2cdc686c6f\r\n"
"\r\n");
    req.append(messageJson);

    SendPacket(req);

    char buf[200];
    int len = 100;

    bool code200OK = true;
    bool checkedHead = false;

    while (1)
    {
        len = SSL_read(ssl, buf, 100);
        buf[len] = 0;
        printf("%s",buf);

        if (!checkedHead)
        {
            if (!check200OK(convertToString(buf,len)))
            {
                code200OK = false;
            }
            checkedHead = true;
        }
        else {
            if (checkEndOfMessage(convertToString(buf,len)))
            {
                return code200OK;
            }      
        }
    }   
}   

bool isBot(string userName) {
    std::regex botRegex("bot");

    if (std::regex_search(userName,botRegex))
    {
        return true;
    }
    
    return false;
}

int main(int argc, char *argv[])
{
    initSSL();


    int counter = 0;
    int recvReturnCode = 0;
    while (1)
    {
        requestChannelInfo();
        string msId = parseChannelInfo();
         
        if (lastMessgeId.empty())
        {
            printf("seting new msid \n");
            requestChannelMessages();
            string messages = getChannelMessages();
            if (!messages.empty()) //react to all messages
            {
                printf("%s",messages.c_str());
                printf("seting new msid \n");
                std::map<string,std::vector<string>> parsedMsg = parseMessages(messages);

                for (auto const& pair : parsedMsg) {
                    auto key = pair.first;
                    printf("msgid : %s content : %s username : %s \n",key.c_str(),pair.second.at(0).c_str(),pair.second.at(1).c_str());
                    if (!isBot(pair.second.at(1).c_str()))
                    {
                        do
                        {
                        } while (!postMessage(pair.second.at(0)));
                    }
                }
                lastMessgeId = msId;     
            }

        }
        else if(lastMessgeId != msId){
            printf("new messages \n");
            requestChannelMessages();
            string messages = getChannelMessages();
            if (!messages.empty()) //react to all messages
            {
                printf("%s",messages.c_str());
                printf("seting new msid \n");
                std::map<string,std::vector<string>> parsedMsg = parseMessages(messages);

                bool startResponding = false;
                for (auto const& pair : parsedMsg) {
                    auto key = pair.first;
                    printf("msgid : %s content : %s username : %s \n",key.c_str(),pair.second.at(0).c_str(),pair.second.at(1).c_str());
                    if (lastMessgeId == key.c_str())
                    {
                        startResponding = true;
                    }
                    if (startResponding)
                    {
                        if (!isBot(pair.second.at(1).c_str()))
                        {
                            do
                            {
                            } while (!postMessage(pair.second.at(0)));
                        }
                    }
                }
                lastMessgeId = msId;     
            }
            //check new msg count
        }
        /*
        string jsonResponse = RecvPacket();

        if (goodCode && checkNewMessages(jsonResponse))
        {
            string request = "GET /api/channels/720745314209890379/messages HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I\r\nUser-Agent: DiscordBot ($url, $versionNumber)\r\n\r\n";
            SendPacket(request);
            string messages = RecvPacket();
            std::vector<string> messageVector = parseMessages(messages);
            printf("new messages : %i \n", countNewMessages(messageVector));
            //printf("%s", messages.c_str());
        }
        /*
        else if (recvReturnCode == 1)
        {
            string request = "GET /api/channels/720745314209890379/messages HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I\r\nUser-Agent: DiscordBot ($url, $versionNumber)\r\n\r\n"; 
            SendPacket(request);
            recvReturnCode = RecvPacket();
        }
        counter++;
        printf("end of loop %i\n", counter);
        */
    }

    return 0;
}