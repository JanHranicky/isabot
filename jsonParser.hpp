#include <stdio.h>
#include <regex>
#include <iostream>
#include "isabot.hpp"

using namespace std;

string getLastMessageId(string jsonResponse);
string parseGuildId(string jsonResponse);
std::map<string,std::vector<string>> parseMessages(string jsonMessages);
std::string parseChannels(string jsonResponse);
std::string executeRegex(std::regex regex, const std::string s);
std::vector<std::string> extractAllRegexPatterns(string s, regex r); 
std::map<string,std::vector<string>> constructParsedMsgMap(std::vector<std::string> id,std::vector<std::string> content,std::vector<std::string> userName);
