#include <stdio.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#define INIT -1
int array[1024] = {0};

void INITArray(int listen_sock){
	int i = 0;
	array[0] = listen_sock;
	for(i=1; i<1024; i++){
		array[i] = INIT;
	}	
}
int Add(int sock)
{
	int i = 0;
	for(i=1; i< 1024; i++){
		if(array[i] == INIT){
			array[i] = sock;
			break;
		}
	}
	if(i == 1024){
		return -1;
	}
	return 0;
}
void Reload(int listen_sock, int* maxFd, fd_set* read_fds)
{
	//将等待数组设置进read_fds,并找到最大fd
	FD_ZERO(read_fds);
	FD_SET(listen_sock, read_fds);		
	int max= listen_sock, i;
	for(i=1; i<1024; i++){
		if(array[i] != INIT){
			FD_SET(array[i], read_fds);
			if(array[i] > max){
				max = array[i];
			}
		}
	}
	*maxFd = max;
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
		array[num] = INIT;
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
	INITArray(listen_sock);
	fd_set read_fds;
	for(;;){
		fflush(stdout);
		int maxFd = listen_sock;
		Reload(listen_sock, &maxFd, &read_fds);
	//	printf("maxFd:%d\n", maxFd);
	//	printf("befor select %d\n", FD_ISSET(listen_sock, &read_fds));
		int ret = select(maxFd+1, &read_fds, NULL, NULL, NULL);
	//	printf("after select %d\n", FD_ISSET(listen_sock, &read_fds));
		fflush(stdout);
		if(ret <0){
			perror("select");
			continue;
		}	
		//数据准备就绪
		if(FD_ISSET(listen_sock, &read_fds)){
			//是链接请求
			struct sockaddr_in client;
			socklen_t len  = sizeof(client);
			int sock = accept(listen_sock, (struct sockaddr *)&client, &len);
			if(sock < 0){
				perror("accept");
				continue;
			}
			printf("client login :%s\n", inet_ntoa(client.sin_addr));
			fflush(stdout);
			//将新的sock加入等待数组
			if(Add(sock) <0){
				printf("达到监听sock上限\n");
				close(sock);
				continue;
			}	
		}	
		//普通数据
		//轮询检测哪些文件描述符准备就绪
		int i;
		for(i=1; i< 1024; i++){
			if(array[i] == INIT){
				continue;	
			}
			if(!FD_ISSET(array[i], &read_fds)){
				continue;
			}
			//array[i]可以读写
			Request(array[i], i);	
		}	
	}
	close(listen_sock);
}
