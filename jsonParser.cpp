#include"jsonParser.hpp"

string getLastMessageId(string jsonResponse) {
    string stringId = between(jsonResponse,"\"last_message_id\": \"","\", \"last_pin_timestamp");
    return stringId;
}

std::string between(std::string const &in,
                    std::string const &before, std::string const &after) {
  int beg = in.find(before);
  beg += before.size();
  int end = in.find(after, beg);
  return in.substr(beg, end-beg);
}