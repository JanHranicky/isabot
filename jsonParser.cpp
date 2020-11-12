#include"jsonParser.hpp"

string getLastMessageId(string jsonResponse) {

    string id = executeRegex(std::regex("\"last_message_id\": \"[0-9]+\","),jsonResponse);
    id = executeRegex(std::regex("[0-9]+"),id);
    return id;
    
}

string parseGuildId(string jsonResponse) {
    string stringId = between(jsonResponse,"\"id\": \"","\", \"name\":");
    return stringId;
}

std::string between(std::string const &in,
                    std::string const &before, std::string const &after) {
  int beg = in.find(before);
  beg += before.size();
  int end = in.find(after, beg);
  //printf("end : %i\n",end);
  return in.substr(beg, end-beg);
}

std::pair<int,string> betweenTest(std::string const &in,
                    std::string const &before, std::string const &after) {
  int beg = in.find(before);
  beg += before.size();
  int end = in.find(after, beg);
  std::pair<int,string> returnPair(end,in.substr(beg, end-beg));
  return returnPair;
}

  std::map<string,std::vector<string>> parseMessages(string jsonMessages) {
  std::map<string,std::vector<string>> parsedMessages;

  int beg = 0;
  string pomMessages = jsonMessages;

  string before = ", {\"id\": \"";
  string after = "\", \"type\":";
  string beforeFirst = "\"id\": \"";

  string beforeContent = "\"content\": \"";
  string afterContent = "\", \"channel_id\":";

  string beforeUserName = "\"username\": \"";
  string afterUserName = "\", \"avatar\":";

  std::pair<int,string> returnPair;
  std::vector<string> ids;

  int size = jsonMessages.size();

  while (beg < size)
  {
    if (size == jsonMessages.size())
    {
      returnPair = betweenTest(pomMessages,beforeFirst,after);
    }
    else {
      returnPair = betweenTest(pomMessages,before,after);
    }
    std::vector<string> msgContent;
    msgContent.push_back(between(pomMessages,beforeContent,afterContent));

    std::pair<int,string> returnUsername = betweenTest(pomMessages,beforeUserName,afterUserName);
    msgContent.push_back(returnUsername.second);
    
    //msgContent.push_back(between(pomMessages,beforeUserName,afterUserName));

    parsedMessages.emplace(std::make_pair(returnPair.second,msgContent));

    //ids.push_back(returnPair.second);

    pomMessages = pomMessages.substr(returnUsername.first,pomMessages.length());
    //printf("%s \n\n",pomMessages.c_str());
    //printf("vector size : %i\n",ids.size());
    beg = returnUsername.first;
    size = pomMessages.size();
  }
    
  return parsedMessages;
}

std::string executeRegex(std::regex regex, const std::string s) {
  std::smatch match;
    if (regex_search(s.begin(), s.end(), match, regex))
      return match[0];
  return NULL;
}

std::string parseChannels(string jsonResponse) {

  string idString =  executeRegex(std::regex("\\{.{0,100}isabot"),jsonResponse);
  idString = executeRegex(std::regex("\"id\": \"[0-9]*\""),idString);
  string id = executeRegex(std::regex("[0-9]+"),idString);
  return id;
}