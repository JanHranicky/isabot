# Discord bot

super simple discord bot whose only job is to copy paste the message you type in a channel it is monitoring. The goal of this project was not to code a bot, but rather learn the SSL communication from the bottom up, while high level libraries can do this using a single function call, the network communication in this repository is coded to the last bit.


## Running the bot

Build using:

    make    

Run it:

    ./isabot [-v] [-h] -t bot_token
        -v debugs the messages the bot responds to into terminal
        -h help
        -t bot_token discord API token needed for the communication 
