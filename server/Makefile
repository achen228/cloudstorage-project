.PHONY:all
all:server upload download
server:server.cpp
	g++ $^ -o $@ -std=c++11 -lpthread -ljsoncpp -L/usr/lib64/mysql/ -lmysqlclient
upload:upload.cpp
	g++ $^ -o $@ -std=c++11
download:download.cpp
	g++ $^ -o $@ -std=c++11

.PHONY:clean
clean:
	rm server
	rm upload
	rm download
