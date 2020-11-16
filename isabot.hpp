#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <vector>
#include <string>
#include <regex>
#include <iostream>
#include <getopt.h>

using namespace std;

string getLastMessageId(); //returns last message id
string getChannelMessages(); //reads channle msg request response
void postMessage(string message); //posts msg to channel
bool isBot(string userName); //check for bot substring in username 
void setGuildId(); //gets guild id from discord api
void setChannelId(); //gets isabot channel id using guild id
string createGetRequest(string req); //creates GET response 
