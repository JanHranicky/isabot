#include"jsonParser.hpp"

string parseLastMessageId(string jsonResponse) {

    string id = executeRegex(std::regex("\"last_message_id\": \"[0-9]+\","),jsonResponse);
    id = executeRegex(std::regex("[0-9]+"),id);
    return id;
    
}

string parseGuildId(string jsonResponse) {

    std::vector<string> splitResponse = splitString(jsonResponse,"\r\n\r\n");
    if (splitResponse.size() == 1) //bot is in no guild
    {
      fprintf(stderr,"Bot isn't in any guild, please add bot to your guild and restart.\n");
      exit(EXIT_FAILURE);
    }
    
    string id = executeRegex(std::regex("\"id\": \"[0-9]+\","),jsonResponse);
    id = executeRegex(std::regex("[0-9]+"),id);
    return id;
}

std::vector<std::string> extractAllRegexPatterns(string s, regex r) {

    std::vector<std::string> returnVector;
    smatch match; 
    int i = 1; 
    while (regex_search(s, match, r)) { 
        returnVector.push_back(match.str(0));
        i++; 
  
        // suffix to find the rest of the string. 
        s = match.suffix().str(); 
    }

    return returnVector;
}

std::vector<std::string> parseMsgIds(string s ) {
  std::vector<std::string> msgIdVector = extractAllRegexPatterns(s,regex("\"id\": \"[0-9]+\", \"type"));
  for (size_t i = 0; i < msgIdVector.size(); i++)
  {
    msgIdVector.at(i) = executeRegex(regex("[0-9]+"),msgIdVector.at(i));
  }
  return msgIdVector;
}

std::string removePrefixAndLastChar(string s,string prefix) {
  s = s.substr(prefix.size(),s.size()-1);
  s = s.substr(0,s.size()-1);
  return s;
}

std::vector<std::string> parseMsgContent(string s ) {
  std::vector<std::string> msgContentVector = extractAllRegexPatterns(s,regex("\"content\"[ :]+(\\\"(?:\\\\.|[^\"])*\\\")"));
  for (size_t i = 0; i < msgContentVector.size(); i++)
  {
    msgContentVector.at(i) = removePrefixAndLastChar(msgContentVector.at(i),"\"content\": \"");
  }
  return msgContentVector;
}

std::vector<std::string> parseMsgUserName(string s) {
  std::vector<std::string> msgUserNameVector = extractAllRegexPatterns(s,regex("\"username\"[ :]+(\\\"(?:\\\\.|[^\"])*\\\")"));
  for (size_t i = 0; i < msgUserNameVector.size(); i++)
  {
    msgUserNameVector.at(i) = removePrefixAndLastChar(msgUserNameVector.at(i),"\"username\": \"");
  }
  return msgUserNameVector;
}

std::map<string,std::vector<string>> constructParsedMsgMap(std::vector<std::string> id,std::vector<std::string> content,std::vector<std::string> userName) {
  std::map<string,std::vector<string>>  returnMap;

  for (size_t i = 0; i < id.size(); i++)
  {
    std::vector<string> mapVector;
    mapVector.push_back(content.at(i));
    mapVector.push_back(userName.at(i));
    returnMap.emplace(id.at(i),mapVector);
  }
  
  return returnMap;
}

std::map<string,std::vector<string>> parseMessages(string jsonMessages) {
  std::map<string,std::vector<string>> parsedMessages;
  
  std::vector<std::string> msgIdVector = parseMsgIds(jsonMessages);
  std::vector<std::string> msgContentVector = parseMsgContent(jsonMessages);
  std::vector<std::string> msgUserNameVector = parseMsgUserName(jsonMessages);

  return constructParsedMsgMap(msgIdVector,msgContentVector,msgUserNameVector);
}

std::string executeRegex(std::regex regex, const std::string s) {
  std::smatch match;
    if (regex_search(s.begin(), s.end(), match, regex))
      return match[0];
  return "";
}

std::string parseChannels(string jsonResponse) {
  string idString =  executeRegex(std::regex("\\{.{0,100}isa-bot"),jsonResponse);
  
  if (idString.empty())
  {
    fprintf(stderr,"There is no channel named '#isa-bot' on your guild, please add one and restart \n");
    exit(EXIT_FAILURE);
  }
  
  idString = executeRegex(std::regex("\"id\": \"[0-9]*\""),idString);
  string id = executeRegex(std::regex("[0-9]+"),idString);
  return id;
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

string extractResponseCode(string s) {
    string code = executeRegex(regex("^HTTP\\/1.1\\s(\\d+)"),s);
    code = executeRegex(regex("\\d{3}"),code);
    return code;
}

bool isBot(string userName) {
    std::regex botRegex("bot");

    if (std::regex_search(userName,botRegex))
    {
        return true;
    }
    
    return false;
}