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
#include <iostream>
#include <getopt.h>
#include "jsonParser.hpp"
#include "isabot.hpp"

using namespace std;

int SendPacket(string request); //sends packet to discord
void initSSL(); //inits SSL socket
string sendRqAndGetResponse(string request);
void logResponseCode(string code);
