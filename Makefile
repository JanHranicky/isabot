make: isabot.cpp
	g++ -o isabot jsonParser.cpp isabot.cpp -g -lssl -lcrypto -std=c++11