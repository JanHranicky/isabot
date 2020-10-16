#include <stdio.h>
#include <string>
#include <map>
#include<vector>

using namespace std;

string getLastMessageId(string jsonResponse);
std::string between(std::string const &in,std::string const &before, std::string const &after);
std::map<string,std::vector<string>> parseMessages(string jsonMessages);
