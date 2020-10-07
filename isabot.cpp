
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
#include<vector>
#include<string>

#include"jsonParser.hpp"

using namespace std;
    
SSL *ssl;
int sock;
int timeout = 1000;
int finalLen = 0;


string jsonResponse;
string lastMessgeId;
string stopMessage;

bool checkCR(char c) {
    if (c == 13) //CR = 13 ASCII
    {
        return true;
    }
    return false;
}

bool checkLF(char c) {
    if (c == 10) //LF = 10 ASCII
    {
        return true;
    }
    return false;
}

bool checkZero(char c) {
    if (c == 48)
    {
        return true;
    }
    return false;
}

void checkForBegginingOfJson(char *buf,int len) {
    bool inJson = false;

    for (size_t i = 0; i < len; i++)
    {
        if (buf[i] == '[' || buf[i] == '{' || inJson)
        {
            jsonResponse.push_back(buf[i]);
            inJson = true;
        }
        
    }    
}

//CRLFCRLF
bool checkEndOfHead(char *buf,int len) {
    for (size_t i = 0; i < len; i++)
    {
        if (checkCR(buf[i]) && i+3 < len)
        {
            if (checkLF(buf[i+1]))
            {
                if (checkCR(buf[i+2]))
                {
                    if (checkLF(buf[i+3]))   
                    {
                        finalLen += len;
                        checkForBegginingOfJson(buf,len);
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
bool checkEndOfMessage(char *buf,int len) {
    for (size_t i = 0; i < len; i++)
    {
        if (checkZero(buf[i]) && i+4 < len)
        {
            if (checkCR(buf[i+1]))
            {
                if (checkLF(buf[i+2]))
                {
                    if (checkCR(buf[i+3]))
                    {
                        if (checkLF(buf[i+4]))
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

void addToJsonResonse(char *buf,int len) {
    for (size_t i = 0; i < len; i++)
    {
        jsonResponse.push_back(buf[i]);
    }
    
}

bool parseHead(char *buf,int len,string returnCode) { //9  10 11
    printf("inside parse head buf: %s",buf);
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

int RecvPacket()
{
    int len=100;
    char buf[1000000];

   bool bodyFlag = false;
   bool parsedHead = false;
   while (1)
   {
        len=SSL_read(ssl, buf, 100);
        buf[len]=0;

        if (!bodyFlag)
        {           
            if (!parsedHead)
            {
                if (!parseHead(buf,len,"200"))
                {
                    break;
                }
                parsedHead = true;
            }
            
            if (checkEndOfHead(buf,len))
            {
                bodyFlag = true;
            }
        }
        else {
            if (checkEndOfMessage(buf,len) || len < 0) {
                break;
            } 
            else {
                finalLen += len;
                addToJsonResonse(buf,len);
                //printf("%s",jsonResponse.c_str());                
            }
        }        
   }
    //printf("finallen je : %i + velikost vectoru je : %i\n",finalLen,jsonResponse.size());
    //printf("\n\nfinished reading \nGoing to parseing \n");

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
            printf("last message : %s\n",lastMessgeId.c_str());
            printf("stop message : %s\n",stopMessage.c_str());
            return 1; //get new messages
        }
    }
    
    
    //printf("%s <- last message id \n",getLastMessageId(jsonResponse).c_str());

    jsonResponse.clear();

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
    return 0;
}
   
int SendPacket(string request)
{
    const void * a = request.c_str();
    int len = SSL_write(ssl, a, request.length());
    //int len = SSL_write(ssl, &request, request.length());
    if (len < 0) {
        int err = SSL_get_error(ssl, len);
        switch (err) {
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
    while (err = ERR_get_error()) {
        char *str = ERR_error_string(err, 0);
        if (!str)
            return;
        printf("\n");
        fflush(stdout);
    }
}
    
int main(int argc, char *argv[])
{
    int s;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        printf("Error creating socket.\n");
        return -1;
    }
    struct sockaddr_in sa;
    memset (&sa, 0, sizeof(sa));
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr = inet_addr("162.159.138.232"); // discord ip
    sa.sin_port        = htons (443); 
    socklen_t socklen = sizeof(sa);
    if (connect(s, (struct sockaddr *)&sa, socklen)) {
        printf("Error connecting to server.\n");
        return -1;
    }
    SSL_library_init();
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();
    const SSL_METHOD *meth = TLSv1_2_client_method();
    SSL_CTX *ctx = SSL_CTX_new (meth);
    ssl = SSL_new (ctx);


    if (!ssl) {
        printf("Error creating SSL.\n");
        log_ssl();
        return -1;
    }
    sock = SSL_get_fd(ssl);
    SSL_set_fd(ssl, s);
    int err = SSL_connect(ssl);
    if (err <= 0) {
        printf("Error creating SSL connection.  err=%x\n", err);
        log_ssl();
        fflush(stdout);
        return -1;
    }
    printf ("SSL connection using %s\n", SSL_get_cipher (ssl));

    int counter = 0;
    int recvReturnCode = 0;
    while (1)
    {
        if (recvReturnCode == 0)
        {
            string request = "GET /api/channels/720745314209890379 HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I\r\nUser-Agent: DiscordBot ($url, $versionNumber)\r\n\r\n"; 
            SendPacket(request);
            recvReturnCode = RecvPacket();      
        }
        else if (recvReturnCode == 1)
        {
            string request = "GET /api/channels/720745314209890379/messages HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I\r\nUser-Agent: DiscordBot ($url, $versionNumber)\r\n\r\n"; 
            SendPacket(request);
            recvReturnCode = RecvPacket();
        }
        
        counter ++;
        printf("end of loop %i\n",counter);  
    }
    
    return 0;
}