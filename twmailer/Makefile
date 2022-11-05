all: clientTarget serverTarget

clientTarget:
	g++ -std=c++14 -Wall -Werror -o twmailer-client client/client-runner.cpp client/client.cpp client/client.h

serverTarget:
	g++ -std=c++14 -Wall -Werror -o twmailer-server server/server-runner.cpp server/server.cpp server/server.h

clean:
	rm twmailer-client
	rm twmailer-server
