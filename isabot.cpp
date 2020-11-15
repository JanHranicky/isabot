#include "isabot.hpp"
#include "jsonParser.hpp"

SSL *ssl;
int sock;
SSL_CTX *ctx;

string guildId;
string channelId;

bool vFlag = false;
string botToken = "NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I";

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
    string request = "GET /api/channels/"+channelId+" HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot "+botToken+"\r\nConnection: close\r\nUser-Agent: DiscordBot ($url, $versionNumber)\r\n\r\n";
    string messageString = sendRqAndGetResponse(request);

    std::vector<string> parsedResponse;
    parsedResponse = splitString(messageString,"\r\n\r\n");

    return getLastMessageId(parsedResponse.at(1));
}

string getChannelMessages() {
    string request = "GET /api/channels/"+channelId+"/messages HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot "+botToken+"\r\nConnection: close\r\nUser-Agent: DiscordBot ($url, $versionNumber)\r\n\r\n";  
    string messageString = sendRqAndGetResponse(request);
    return splitString(messageString,"\r\n\r\n").at(1);
}

bool postMessage(string message) {
    //string request = "POST /api/channels/"+channelId+"/messages HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot NzYyMTAyODE1MjQxNjY2NTkw.X3kRjg.DJE2YvnLBS77t6x6POlYO8xOX_I\r\nContent-Type: application/json\r\nContent-Length: 153\r\nCache-Control: no-cache\r\n\r\n{\"content\": \"Hello, World!\",\n\"tts\": false,\n\"embed\": {\n\"title\": \"Hello, Embed!\",\n\"description\": \"This is an embedded message.\"\n}\n}\n";
    
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
"Authorization: Bot "+botToken+"\r\n"
"Connection: close\r\n"
"Content-Length: ");
req.append(c);
req.append("\r\n"
"Content-Type: application/json\r\n"
"Cache-Control: no-cache\r\n"
"Postman-Token: f4b2e754-2b42-a134-185c-5a2cdc686c6f\r\n"
"\r\n");
    req.append(messageJson);


    string response = sendRqAndGetResponse(req);
    if (check200OK(response))
    {
        return true;
    }
    else return false;
 
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
        vFlag = true;
        break;
      case 't':
        botToken = optarg;
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
    string request = "GET /api/users/@me/guilds HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot "+botToken+"\r\nConnection: close\r\nUser-Agent: DiscordBot ($url, $versionNumber)\r\n\r\n";
    return parseGuildId(sendRqAndGetResponse(request));
}

void setChannelId() {
    string request = "GET /api/guilds/"+guildId+"/channels HTTP/1.1\r\nHost: discord.com\r\nAuthorization: Bot "+botToken+"\r\nConnection: close\r\nUser-Agent: DiscordBot ($url, $versionNumber)\r\n\r\n";
    string response = sendRqAndGetResponse(request);
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

    return response;    
}

int main(int argc, char *argv[])
{
    proccessArguments(argc,argv);

    guildId = getGuildId();
    setChannelId();

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
                //printf("%s",messages.c_str());
                std::map<string,std::vector<string>> parsedMsg = parseMessages(messages);

                bool startResponding = false;
                for (auto const& pair : parsedMsg) {
                    auto key = pair.first;
                    if (startResponding)
                    {
                        //printf("isabot - %s: %s\n",key);
                        if (!isBot(pair.second.at(1).c_str()))
                        {

                            string message = "echo: "+pair.second.at(1)+" - "+pair.second.at(0);
                            
                            while (!postMessage(message)) {
                            }
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