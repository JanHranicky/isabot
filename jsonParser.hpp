#include <stdio.h>
#include <string>
#include <map>
#include<vector>
#include <regex>
#include <iostream>

using namespace std;

string getLastMessageId(string jsonResponse);
string parseGuildId(string jsonResponse);
std::string between(std::string const &in,std::string const &before, std::string const &after);
std::map<string,std::vector<string>> parseMessages(string jsonMessages);
std::string parseChannels(string jsonResponse);
std::string executeRegex(std::regex regex, const std::string s);