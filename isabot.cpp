#include "isabot.hpp"
#include "jsonParser.hpp"

SSL *ssl;
int sock;
SSL_CTX *ctx;

string guildId;
string channelId;

bool vFlag = false;
string botToken; //test bot token = NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I

int reqFrequency = 1; //1s

string lastMessgeId;

int SendPacket(string request)
{
    const void *a = request.c_str();
    int len = SSL_write(ssl, a, request.length());
    //int len = SSL_write(ssl, &request, request.length());
    if (len < 0)
    {
        int err = SSL_get_error(ssl, len);
        switch (err)
        {
        case SSL_ERROR_WANT_WRITE:
            return 0;
        case SSL_ERROR_WANT_READ:
            return 0;
        case SSL_ERROR_ZERO_RETURN:
        case SSL_ERROR_SYSCALL:
        case SSL_ERROR_SSL:
        default:
            return -1;
        }
    }

    return 0;
}

void log_ssl()
{
    int err;
    while (err = ERR_get_error())
    {
        char *str = ERR_error_string(err, 0);
        if (!str)
            return;
        printf("\n");
        fflush(stdout);
    }
}

void initSSL()
{
    int s;
    s = socket(AF_INET, SOCK_STREAM, 0);

    if (s < 0)
    {
        printf("Error creating socket.\n");
        exit(-1);
    }

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr("162.159.138.232"); // discord ip
    sa.sin_port = htons(443);
    socklen_t socklen = sizeof(sa);
    if (connect(s, (struct sockaddr *)&sa, socklen))
    {
        printf("Error connecting to server.\n");
        exit(-1);
    }

    SSL_library_init();
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();

    const SSL_METHOD *meth = TLSv1_2_client_method();
    ctx = SSL_CTX_new(meth);
    ssl = SSL_new(ctx);

    if (!ssl)
    {
        //printf("Error creating SSL.\n");
        log_ssl();
        exit(-1);
    }
    sock = SSL_get_fd(ssl);
    SSL_set_fd(ssl, s);
    int err = SSL_connect(ssl);
    if (err <= 0)
    {
        //printf("Error creating SSL connection.  err=%x\n", err);
        log_ssl();
        fflush(stdout);
        exit(-1);
    }
    //printf("SSL connection using %s\n", SSL_get_cipher(ssl));
}

bool check200OK(string s) {
     std::regex codeRegex("^HTTP/1.1 200 OK");

    if (std::regex_search(s,codeRegex))
    {
        return true;
    }

    return false;        
}

string extractResponseCode(string s) {
    string code = executeRegex(regex("^HTTP\\/1.1\\s(\\d+)"),s);
    code = executeRegex(regex("\\d{3}"),code);
    return code;
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

string parseChannelInfo() { //returns last message id
    string messageString = sendRqAndGetResponse(createGetRequest("/api/channels/"+channelId));

    std::vector<string> parsedResponse;
    parsedResponse = splitString(messageString,"\r\n\r\n");

    return getLastMessageId(parsedResponse.at(1));
}

string getChannelMessages() {
    string messageString = sendRqAndGetResponse(createGetRequest("/api/channels/"+channelId+"/messages"));
    
    std::vector<string> splitResponse = splitString(messageString,"\r\n\r\n");

    if (splitResponse.size() == 1) //no msgs in channel
    {
        return "";
    }
    
    return splitString(messageString,"\r\n\r\n").at(1);
}

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

bool isBot(string userName) {
    std::regex botRegex("bot");

    if (std::regex_search(userName,botRegex))
    {
        return true;
    }
    
    return false;
}

string createGetRequest(string req) {
    stringstream ss;

    ss << "GET " << req << " HTTP/1.1\r\n";
    ss << "Host: discord.com\r\n";
    ss << "Authorization: Bot "+botToken+"\r\n";
    ss << "Connection: close\r\n";
    ss << "\r\n\r\n";

    return ss.str();
}


void proccessArguments(int argc,char *argv[]) {
    int c;
    bool tokenFlag = false;

    while ((c = getopt (argc, argv, "hvt:")) != -1)
    switch (c)
      {
      case 'h':
        printf("Hey I am isabot discord echo bot\n");
        printf("I accept these parameters : \n");
        printf("-h -> print help and exit\n");
        printf("-t <bot_token> -> mandatory parameter -t with valid discord bot_token, the bot token must be activated via websocket\n");
        printf("-v -> optional parameter, I will print messages I reacted to terminal\n");
        printf("I will connect to the server via the bot_token and will be listening on the #isa-bot channel\n");
        exit(EXIT_SUCCESS);
        break;
      case 'v':
        vFlag = true;
        break;
      case 't':
        botToken = optarg;
        tokenFlag = true;
        break;
      case '?':
        if (optopt == 't') {
          fprintf (stderr, "Option -%c requires bot access token\n", optopt);
          exit(-1);
        }
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        break;
      default:
        abort ();
    }
    
    if (!tokenFlag)
    {
        fprintf(stderr,"The -t parameter is mandatory, please restart the program with -t <bot_token> parameter \n");
        exit(EXIT_FAILURE);
    }
    
}

string getGuildId() {
    return parseGuildId(sendRqAndGetResponse(createGetRequest("/api/users/@me/guilds")));
}

void setChannelId() {
    string response = sendRqAndGetResponse(createGetRequest("/api/guilds/"+guildId+"/channels"));
    channelId = parseChannels(response);    
}

string sendRqAndGetResponse(string request) {

    initSSL();
    SendPacket(request);

    string response;
    char buf[101];
    int len = 100;

    while (1)
    {   
        len = SSL_read(ssl, buf, 100);
        buf[len] = 0;
        response += convertToString(buf,len);
        if (len == 0)
        {
            break;
        }     
    }

    SSL_free(ssl);
    SSL_CTX_free(ctx);

    string responseCode = extractResponseCode(response);

    //429,500 codes to try again 
    if (responseCode == "500" || responseCode == "429") return sendRqAndGetResponse(request);
    else if (responseCode == "200") return response;  
    else logResponseCode(responseCode);

    //func should never get to this point
    return "";
}

void logResponseCode(string code) {
    fprintf( stderr, "Discord returned unexpected response code : %s \n", code.c_str());

    if (code == "401")
    {
        fprintf( stderr, "Bot token u have entered might not be valid\n");
    }
    
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    proccessArguments(argc,argv);

    guildId = getGuildId();
    printf("Connected to guild with id : %s\n",guildId.c_str());
    setChannelId();
    printf("Listening on #isa-bot channel with id : %s\n",channelId.c_str());

    int counter = 0;
    int recvReturnCode = 0;
    while (1)
    {
        string msId = parseChannelInfo();
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