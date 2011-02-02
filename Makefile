all:
	cp template.h defines.h
	g++ main.cpp game.cpp irc.cpp modules.cpp -o GameServ -lmysqlclient -rdynamic -ldl
nomysql:
	cat template.h | sed "s/#define USE_MYSQL/\/\/#define USE_MYSQL/" > defines.h
	g++ main.cpp game.cpp irc.cpp -o GameServ -rdynamic -ldl
install:
	cp GameServ /usr/bin/GameServ
	chmod 0755 /usr/bin/GameServ
uninstall:
	rm -f /usr/bin/GameServ
clean:
	rm -f GameServ
