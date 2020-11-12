#include "isabot.hpp"
#include "jsonParser.hpp"

SSL *ssl;
int sock;
string guildId;
string channelId;

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
    SSL_CTX *ctx = SSL_CTX_new(meth);
    ssl = SSL_new(ctx);

    if (!ssl)
    {
        printf("Error creating SSL.\n");
        log_ssl();
        exit(-1);
    }
    sock = SSL_get_fd(ssl);
    SSL_set_fd(ssl, s);
    int err = SSL_connect(ssl);
    if (err <= 0)
    {
        printf("Error creating SSL connection.  err=%x\n", err);
        log_ssl();
        fflush(stdout);
        exit(-1);
    }
    printf("SSL connection using %s\n", SSL_get_cipher(ssl));
}

void requestChannelInfo() {
    string request = "GET /api/channels/"+channelId+" HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I\r\nUser-Agent: DiscordBot ($url, $versionNumber)\r\n\r\n";
    SendPacket(request);
}

void requestChannelMessages() {
    string request = "GET /api/channels/"+channelId+"/messages HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I\r\nUser-Agent: DiscordBot ($url, $versionNumber)\r\n\r\n";
    SendPacket(request);
}

bool check200OK(string s) {
     std::regex codeRegex("^HTTP/1.1 200 OK");

    if (std::regex_search(s,codeRegex))
    {
        return true;
    }

    return false;        
}

bool checkEndOfMessage(string s) {
    std::regex endRegex("^0\\r\\n\\r\\n");
    std::regex endRegex02("\n}\n");

    if (std::regex_search(s,endRegex) || std::regex_search(s,endRegex02))
    {
        return true;
    }
    
    return false;
    
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
    char buf[101];
    int len = 100;

    bool parsedHead = false;
    string messageString;
    while (1)
    {   
        len = SSL_read(ssl, buf, 100);
        buf[len] = 0;
        //printf("%s",buf);
        if (!parsedHead)
        {
            if (!check200OK(convertToString(buf,len)));
            {
                //TODO 
            }
            
            parsedHead = true;
        }
        else {
            string converted = convertToString(buf,len); 
            messageString += converted;
            if (checkEndOfMessage(convertToString(buf,len)))
            {
                //printf("%s",messageString.c_str());
                break;
            }
        }
    }
    //printf("%s",messageString.c_str());

    std::vector<string> parsedResponse;
    parsedResponse = splitString(messageString,"\r\n\r\n");

    return getLastMessageId(parsedResponse.at(1));
}

string getChannelMessages() {
    string channelMessagesResponse;

    char buf[200];
    int len = 100;

    bool code200Ok = true;
    bool checkedHead = false;
    while (1)
    {   
        len = SSL_read(ssl, buf, 100);
        buf[len] = 0;
        //printf("%s",buf);

        if (!checkedHead)
        {
            if (!check200OK(convertToString(buf,len)))
            {
                code200Ok = false;
            }
            checkedHead = true;
        }
        else {
            if(code200Ok) {
                channelMessagesResponse += convertToString(buf,len);
            }
            if (checkEndOfMessage(convertToString(buf,len)))
            {
                if (!channelMessagesResponse.empty())
                {
                    return splitString(channelMessagesResponse,"\r\n\r\n").at(1).c_str();
                }
                
                return channelMessagesResponse;
            }
        }
    }   
}

bool postMessage(string message) {
    string request = "POST /api/channels/"+channelId+"/messages HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I\r\nContent-Type: application/json\r\nContent-Length: 153\r\nCache-Control: no-cache\r\n\r\n{\"content\": \"Hello, World!\",\n\"tts\": false,\n\"embed\": {\n\"title\": \"Hello, Embed!\",\n\"description\": \"This is an embedded message.\"\n}\n}\n";
    
    string messageJson = "{\n"
    "  \"content\": \"";
    messageJson.append(message);
    messageJson.append("\",\n"
    " \"tts\": false,\n"
    " \"embed\": \"\"\n"
    "}");
    
    //printf("%s",messageJson.c_str());

    int size = messageJson.length();
    string c = to_string(size);

    string req;
    req.append("POST /api/channels/"+channelId+"/messages HTTP/1.1\r\n"
"Host: discord.com\r\n"
"Authorization: Bot NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I\r\n"
"Content-Length: ");
req.append(c);
req.append("\r\n"
"Content-Type: application/json\r\n"
"Cache-Control: no-cache\r\n"
"Postman-Token: f4b2e754-2b42-a134-185c-5a2cdc686c6f\r\n"
"\r\n");
    req.append(messageJson);

    SendPacket(req);

    char buf[200];
    int len = 100;

    bool code200OK = true;
    bool checkedHead = false;

    while (1)
    {
        len = SSL_read(ssl, buf, 100);
        buf[len] = 0;
        printf("%s",buf);

        if (!checkedHead)
        {
            if (!check200OK(convertToString(buf,len)))
            {
                code200OK = false;
            }
            checkedHead = true;
        }
        else {
            if (checkEndOfMessage(convertToString(buf,len)))
            {
                return code200OK;
            }      
        }
    }   
}   

bool isBot(string userName) {
    std::regex botRegex("bot");

    if (std::regex_search(userName,botRegex))
    {
        return true;
    }
    
    return false;
}

void proccessArguments(int argc,char *argv[]) {
    int c;

    while ((c = getopt (argc, argv, "hvt:")) != -1)
    switch (c)
      {
      case 'h':
        printf("Printin help\n");
        exit(0);
        break;
      case 'v':
        printf("Procceding to print message I am reacting to\n");
        break;
      case 't':
        printf("option -c argument : %s\n",optarg);
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
}

string getGuildId() {
    string request = "GET /api/users/@me/guilds HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I\r\nUser-Agent: DiscordBot ($url, $versionNumber)\r\n\r\n";
    SendPacket(request);

    string response;
    char buf[101];
    int len = 100;

    while (1)
    {   
        len = SSL_read(ssl, buf, 100);
        buf[len] = 0;
        response += convertToString(buf,len);
        if (checkEndOfMessage(convertToString(buf,len)))
        {
            break;
        }     
    }

    return parseGuildId(response);
}

void setChannelId() {
    string request = "GET /api/guilds/"+guildId+"/channels HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I\r\nUser-Agent: DiscordBot ($url, $versionNumber)\r\n\r\n";
    printf("%s",request.c_str());
    SendPacket(request);

    string response;
    char buf[101];
    int len = 100;

    while (1)
    {   
        len = SSL_read(ssl, buf, 100);
        buf[len] = 0;
        //printf("%s",buf);
        response += convertToString(buf,len);
        if (checkEndOfMessage(convertToString(buf,len)))
        {
            break;
        }     
    }

    channelId = parseChannels(response);    
}

int main(int argc, char *argv[])
{
    proccessArguments(argc,argv);
    initSSL();

    guildId = getGuildId();
    printf("guild id je : %s\n",guildId.c_str());
    setChannelId();
    printf("channle if je : %s\n",channelId.c_str());

    int counter = 0;
    int recvReturnCode = 0;
    while (1)
    {
        printf("zacatek cyklu \n");
        requestChannelInfo();
        string msId = parseChannelInfo();
        printf("msid z channel infa je %s \n",msId.c_str());
        requestChannelMessages();
        string messages = getChannelMessages();
         
        if (lastMessgeId.empty())
        {   
            lastMessgeId = msId;     
            printf("new msid set ! \n");
        }
        else if(lastMessgeId != msId){
            printf("porovnavam %s a %s \n",lastMessgeId.c_str(),msId.c_str());
            printf("new messages \n");
            if (!messages.empty()) //react to all messages
            {
                //printf("%s",messages.c_str());
                std::map<string,std::vector<string>> parsedMsg = parseMessages(messages);

                bool startResponding = false;
                for (auto const& pair : parsedMsg) {
                    auto key = pair.first;
                    if (startResponding)
                    {
                        //printf("isabot - %s: %s\n",key);
                        printf("msgid : %s content : %s username : %s \n",key.c_str(),pair.second.at(0).c_str(),pair.second.at(1).c_str());
                        if (!isBot(pair.second.at(1).c_str()))
                        {
                            while (!postMessage(pair.second.at(0))) {
                            }
                        }
                        lastMessgeId = key;     
                    }
                    if (lastMessgeId == key.c_str())
                    {
                        printf("RESPOND FLAG TRUE \n");
                        startResponding = true;
                    }
                }
            }
            printf("reacted to new messages\n");
            printf("seting new msid : %s\n",msId.c_str());
            //check new msg count
        }
       printf("konec cyklu going to sleep for 3s \n");
       sleep(reqFrequency);
    }

    return 0;
}