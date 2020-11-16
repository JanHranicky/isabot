make: isabot.cpp
	g++ -o isabot argumentParser.cpp httpsClient.cpp jsonParser.cpp isabot.cpp -Wno-deprecated -g -lssl -lcrypto -std=c++11