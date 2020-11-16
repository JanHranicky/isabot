#include "isabot.hpp"
#include "jsonParser.hpp"
#include "httpsClient.hpp"
#include "argumentParser.hpp"

string guildId; //id of guild that isa-bot is ont
string channelId; //id of isa-bot channel

bool vFlag = false; // -v parameter flag
string botToken; //test bot token = NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I

int reqFrequency = 1; //1 second, request frequency

string lastMessgeId; //id of last message that processed

/**
 * Returns last message in isa-bot channel
 */
string getLastMessageId() { //returns last message id
    string messageString = sendRqAndGetResponse(createGetRequest("/api/channels/"+channelId));

    std::vector<string> parsedResponse;
    parsedResponse = splitString(messageString,"\r\n\r\n");

    return parseLastMessageId(parsedResponse.at(1));
}

/**
 * Returns messages of isa-bot channel from discord AOI 
 */
string getChannelMessages() {
    string messageString = sendRqAndGetResponse(createGetRequest("/api/channels/"+channelId+"/messages"));
    
    std::vector<string> splitResponse = splitString(messageString,"\r\n\r\n");

    if (splitResponse.size() == 1) //no msgs in channel
    {
        return "";
    }
    
    return splitString(messageString,"\r\n\r\n").at(1);
}

/**
 * Creates POST request with message to post a sends it to discord api
*/
void postMessage(string message) {

    stringstream ss;

    ss << "{\n";
    ss << "  \"content\": \"";
    ss << message;
    ss << "\",\n";
    ss << " \"tts\": false,\n";
    ss << " \"embed\": \"\"\n";
    ss << "}";

    string messageJson = ss.str();

    int contentLenght = messageJson.length();
    string contentLenghtString = to_string(contentLenght);

    ss.str("");
    ss << "POST /api/channels/"+channelId+"/messages HTTP/1.1\r\n";
    ss << "Host: discord.com\r\n";
    ss << "Authorization: Bot "+botToken+"\r\n";
    ss << "Connection: close\r\n";
    ss << "Content-Length: " << contentLenghtString << "\r\n";
    ss << "Content-Type: application/json\r\n";
    ss << "Cache-Control: no-cache\r\n\r\n";
    ss << messageJson;

    string req = ss.str();

    string response = sendRqAndGetResponse(req); 
}   

/**
 * Creates GET request for Discord API
 * req -> specific request for example /api/users/@me/guilds
*/
string createGetRequest(string req) {
    stringstream ss;

    ss << "GET " << req << " HTTP/1.1\r\n";
    ss << "Host: discord.com\r\n";
    ss << "Authorization: Bot "+botToken+"\r\n";
    ss << "Connection: close\r\n";
    ss << "\r\n\r\n";

    return ss.str();
}

/**
 * Gets guild id and saves it as global variable
*/
void setGuildId() {
    guildId = parseGuildId(sendRqAndGetResponse(createGetRequest("/api/users/@me/guilds")));
}

/**
 * Gets channel ids from discord api and set isa-bot channle id as global variable 
*/
void setChannelId() {
    string response = sendRqAndGetResponse(createGetRequest("/api/guilds/"+guildId+"/channels"));
    channelId = parseChannels(response);    
}

/**
 * Main loop of the program 
*/

int main(int argc, char *argv[])
{
    proccessArguments(argc,argv,&vFlag,&botToken);

    setGuildId();
    printf("Connected to guild with id : %s\n",guildId.c_str());
    setChannelId();
    printf("Listening on #isa-bot channel with id : %s\n",channelId.c_str());

    int counter = 0;
    int recvReturnCode = 0;
    while (1)
    {
        string msId = getLastMessageId();
        string messages = getChannelMessages();
         
        if (lastMessgeId.empty())
        {  
            lastMessgeId = msId;     
        }
        else if(lastMessgeId != msId){
            if (!messages.empty()) //react to all messages
            {
                std::map<string,std::vector<string>> parsedMsg = parseMessages(messages);

                bool startResponding = false;
                for (auto const& pair : parsedMsg) {
                    auto key = pair.first;
                    if (startResponding)
                    {
                        if (!isBot(pair.second.at(1).c_str()))
                        {

                            string message = "echo: "+pair.second.at(1)+" - "+pair.second.at(0);

                            //if original message + echo: <username> prefix is too long the original message is split in two
                            if (message.length() > 2000)
                            {
                                string s = pair.second.at(0);
                                string half = s.substr(0, s.length()/2);
                                string otherHalf = s.substr(s.length()/2);

                                postMessage("echo: "+pair.second.at(1)+"(1/2) - "+half);
                                postMessage("echo: "+pair.second.at(1)+"(2/2) - "+otherHalf);
                            }
                            else postMessage(message);

                            if (vFlag)
                            {
                                //"<channel> - <username>: <message>".
                                printf("#isa-bot - %s: %s \n",pair.second.at(1).c_str(),pair.second.at(0).c_str());
                                //printf("msgid : %s content : %s username : %s \n",key.c_str(),pair.second.at(0).c_str(),pair.second.at(1).c_str());
                            }
                        }
                        lastMessgeId = key;     
                    }
                    if (lastMessgeId == key.c_str())
                    {
                        startResponding = true;
                    }
                }
            }
            //check new msg count
        }
       sleep(reqFrequency);
    }

    return 0;
}