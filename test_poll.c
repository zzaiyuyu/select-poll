#include <stdio.h>
#include <unistd.h>
#include <poll.h>

int main()
{
	//用户关心0 fd的POLLIN事件
	struct pollfd poll_fd;
	poll_fd.fd = 0;
	poll_fd.events = POLLIN;
	for(;;){
		printf("poll wait...\n");
		int ret = poll(&poll_fd, 1, 1000);	
		if(ret < 0){
			perror("poll");
			continue;
		}
		if(ret == 0){
			printf("timeout");
			continue;
		}
		if(poll_fd.revents == POLLIN){
			char buf[1024];
			int readSize = read(0, buf, sizeof(buf)-1);
			buf[readSize] = 0;
			printf("%s", buf);
		}
	}
}
