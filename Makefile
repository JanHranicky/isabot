make: isabot.cpp
	g++ -o isabot jsonParser.cpp isabot.cpp -lssl -lcrypto -std=c++11