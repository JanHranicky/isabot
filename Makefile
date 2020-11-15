make: isabot.cpp
	g++ -o isabot jsonParser.cpp isabot.cpp -Wno-deprecated -g -lssl -lcrypto -std=c++11