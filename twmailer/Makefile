all: clientTarget ldapTarget serverTarget

clientTarget:
	g++ -std=c++14 -Wall -Werror -o twmailer-client client/client-runner.cpp client/client.cpp client/client.h

serverTarget:
	g++ -std=c++14 -pthread -Wall -Werror -o twmailer-server server/server-runner.cpp server/server.cpp server/server.h -lldap -llber

ldapTarget:
	g++ -std=c++14 -c server/ldap.cpp  -Wall -lldap -llber

clean:
	rm twmailer-client
	rm ldap.o
	rm twmailer-server
