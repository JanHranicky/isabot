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
#include <unistd.h>
#include <iostream>
#include <getopt.h>

using namespace std;

int SendPacket(string request); //sends packet to discord
void log_ssl(); //logs ssl error
void initSSL(); //inits SSL socket
void requestChannelInfo(); //requests channel info 
void requestChannelMessages(); //requststs channel messages
bool check200OK(string s); //checks if req was succesful
bool checkEndOfMessage(string s); //checks end of message 
string convertToString(char buf[],int len); //converts char[] to std::string
std::vector<string> splitString(string s, string delimiter); // splits string into two with using a delimiter
string parseChannelInfo(); //returns last message id
string getChannelMessages(); //reads channle msg request response
bool postMessage(string message); //posts msg to channel
bool isBot(string userName); //check for bot substring in username 
void proccessArguments(int argc,char *argv[]); //proccessing arguments
string getGuildId(); //gets guild id from discord api
void setChannelId(); //gets isabot channel id using guild id
