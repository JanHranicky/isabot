#include <stdio.h>
#include <regex>
#include <iostream>

using namespace std;

string parseLastMessageId(string jsonResponse);
string parseGuildId(string jsonResponse);
std::map<string,std::vector<string>> parseMessages(string jsonMessages);
std::string parseChannels(string jsonResponse);
std::string executeRegex(std::regex regex, const std::string s);
std::vector<std::string> extractAllRegexPatterns(string s, regex r); 
std::map<string,std::vector<string>> constructParsedMsgMap(std::vector<std::string> id,std::vector<std::string> content,std::vector<std::string> userName);

string convertToString(char buf[],int len);
std::vector<string> splitString(string s, string delimiter); // splits string into two with using a delimiter
string extractResponseCode(string s);
bool isBot(string userName);