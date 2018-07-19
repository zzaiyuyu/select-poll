#include <stdio.h>
#include <sys/select.h>

int main()
{
	fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_SET(0, &read_fds);
	while(1)
	{
		struct timeval t;
		t.tv_sec = 3;
		t.tv_usec = 0; 
		printf("select wait..\n");
		int ret = select(1, &read_fds, NULL, NULL, &t);
		if(FD_ISSET(0, &read_fds)){
			printf("read。。");
			char buf[1024] = {0};
			int readS = read(0, buf, sizeof(buf)-1);
			if(readS < 0){
				perror("read");
				continue;
			}
			else{
				buf[readS] = 0;
				printf("%s\n", buf);
			}
		}
		FD_ZERO(&read_fds);
		FD_SET(0, &read_fds);
	}
}
