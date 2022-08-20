#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

const char SERVER[] = "/tmp/server";
const int BUFFER_LENGTH = 4;

void check(long num);

int main()
{
	struct sockaddr_un addr{AF_UNIX};
	strcpy(addr.sun_path, SERVER);

	char buf[BUFFER_LENGTH];
	int sock = -1;
	int rc = -1;
	
	while (sock < 0)
	{
		sock = socket(AF_UNIX, SOCK_STREAM, 0);
		if (sock < 0)
		{
			perror("socket() failed");
			sleep(1);
			continue;
		}
	}
	int length = BUFFER_LENGTH;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&length, sizeof(length));
	
	while	(true)
	{
		while (rc < 0)
		{
			rc = connect(sock, (struct sockaddr*)&addr, SUN_LEN(&addr));
			if (rc < 0)
			{
				perror("connect() failed");
				sleep(1);
				continue;
			}
		}
		
		rc = recv(sock, &buf,BUFFER_LENGTH, 0);
		if (rc < 0)
		{
			perror("recv() failed");
			sleep(1);
			continue;
		}
		else if (rc == 0)
		{
			std::cerr << "nothing to read\n";
			rc = connect(sock, (struct sockaddr*)&addr, SUN_LEN(&addr));
			sleep(1);
			continue;
		}
		
		long sum = std::strtol(buf, nullptr, 0);
		check(sum);
	}
}

void check(long num)
{
	if (num % 32 != 0 || num < 100)
	{
		std::cerr << "Incorrect sum number: " << num << "!\n";
	}
	else
	{
		std::cout << "sum = " << num << '\n';
	}
}
