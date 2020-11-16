#include "httpsClient.hpp"

SSL *ssl;
int sock;
SSL_CTX *ctx;

/**
 * Sends packet with request
 */
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

/**
 * inits SSL connection to DiscordAPI
 */
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
        fprintf(stderr,"Error creating SSL.\n");
        exit(-1);
    }
    sock = SSL_get_fd(ssl);
    SSL_set_fd(ssl, s);
    int err = SSL_connect(ssl);
    if (err <= 0)
    {
        fprintf(stderr,"Error creating SSL connection.  err=%x\n", err);
        exit(-1);
    }
}

/**
 * Sends request to Discord API and returns reponse
 */
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

/**
 * Logs unexcpected response code 
 */
void logResponseCode(string code) {
    fprintf( stderr, "Discord returned unexpected response code : %s \n", code.c_str());

    if (code == "401")
    {
        fprintf( stderr, "Bot token u have entered might not be valid\n");
    }
    
    exit(EXIT_FAILURE);
}