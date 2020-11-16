#include "argumentParser.hpp"

/**
 * Procceses arguments and sets flags and variables
 */
void proccessArguments(int argc,char *argv[], bool* vFlag,string *botToken) {
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
        *vFlag = true;
        break;
      case 't':
        *botToken = optarg;
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