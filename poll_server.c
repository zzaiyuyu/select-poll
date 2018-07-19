#include <stdio.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>

#define INIT -1
//将fd和事件绑定的结构体数组
#define FDS 1024
struct pollfd fds[1024]; 

void InitFds()
{
	int i;
	for(i=0; i<FDS; i++){
		fds[i].fd = INIT;
		fds[i].events = 0;
		fds[i].revents = 0;
	}	
}

void Add(int sock)
{
	int i;
	for(i=0; i<FDS; i++){
		if(fds[i].fd == INIT){
			fds[i].fd = sock;
			fds[i].events = POLLIN;
			break;
		}
	}
}
void Request(int sock, int num)
{
	char buf[1024] = {0};
	int readSize = read(sock, buf, sizeof(buf)-1);
	if(readSize< 0){
		perror("read");
		return;
	}
	if(readSize == 0){
		printf("client quit!\n");
		close(sock);
		fds[num].fd = INIT;
	}
	buf[readSize] = 0;
	printf("client :%s\n", buf);
	write(sock, buf, sizeof(buf));		
}

int main(int argc, char* argv[])
{
	if(argc < 3){
		exit(1);
	}
	int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock <0){
		perror("socket");
		exit(2);
	}
	struct sockaddr_in server;
	struct sockaddr_in client;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(argv[1]);
	server.sin_port = htons(atoi(argv[2]));
	if(bind(listen_sock, (struct sockaddr *)&server, sizeof(server))< 0){
		perror("bind");
		close(listen_sock);
		exit(3);
	}
	if(listen(listen_sock, 5)< 0){
		perror("listen");
		close(listen_sock);
		exit(4);
	}
	printf("listen...");	
	InitFds();
	Add(listen_sock);
	for(;;){
		//等
		int ret = poll(fds, FDS, 3000);
		if(ret <0){
			perror("POLL");
			continue;
		}
		if(ret == 0){
			printf("timeout\n");
			continue;
		}
		//poll返回，有文件描述符就绪,进行轮询
		int i;
		for(i=0; i<FDS; i++){
			if(fds[i].fd == INIT){
				continue;
			}
			if(!(fds[i].revents & POLLIN)){
				continue;
			}
			//fds[i].fd就绪
			if(fds[i].fd == listen_sock){
				//监听套接字
				struct sockaddr_in client;
				socklen_t len  = sizeof(client);	
				int connect_fd = accept(listen_sock, (struct sockaddr*)&client, &len);
				if(connect_fd <0){
					perror("accept");
					continue;
				}
				printf("client login:%s\n", inet_ntoa(client.sin_addr));
				Add(connect_fd);
			}	
			else{
				//普通数据
				Request(fds[i].fd, i);
			}
		}

	}
}
