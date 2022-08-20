#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <string>
#include <cctype>
#include <algorithm>
#include <queue>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

const char SERVER[] = "/tmp/server";
const int MAX_LENGTH = 64;
const int MAX_BUF_SIZE = 1000;

void produce(std::string& input, std::queue<std::string>& queue);
int consume(std::queue<std::string>& queue);
bool isDigitString(const std::string& string);

int main()
{
	std::queue<std::string> q;
	std::mutex mut;
	std::condition_variable not_empty, not_full;

	std::thread producer([&]()
								 	{
										while (true)
										{
											if (q.size() == MAX_BUF_SIZE)
											{
												std::unique_lock<std::mutex> uniqueLock{mut};
												not_full.wait(uniqueLock, [&]
												{
													return q.size() < MAX_BUF_SIZE;
												});
											}
											std::string input;
											std::cin >> input;
											produce(input, q);
											not_empty.notify_one();
										}
								 	}); // producer uses produce and throw it to consumer

	std::thread consumer([&]()
									{
										int sock = -1;
										int client_sock = -1;
										int rc = -1;
										struct sockaddr_un addr = {AF_UNIX};
										strcpy(addr.sun_path, SERVER);
										
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
										int length = MAX_LENGTH;
										setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&length, sizeof(length));
										
										while (true)
										{
											if (q.empty())
											{
											 std::unique_lock<std::mutex> uniqueLock{mut};
											 not_empty.wait(uniqueLock, [&]
											 {
												 return !q.empty();
											 });
											}
											std::string message = std::to_string(consume(q));
											
											while (rc < 0)
											{
											 rc = bind(sock, (struct sockaddr*)&addr, SUN_LEN(&addr));
											 if (rc < 0)
											 {
												 unlink(SERVER);
												 sleep(1);
												 continue;
											 }
											}
											
											rc = -1;
											while (rc < 0)
											{
												rc = listen(sock, 1);
												if (rc < 0)
												{
													perror("listen() failed");
													sleep(1);
													continue;
												}
											}
											
											while (client_sock < 0)
											{
												client_sock = accept(sock, nullptr, nullptr);
												if (client_sock < 0)
												{
													perror("accept() failed");
													sleep(1);
													continue;
												}
											}
											setsockopt(client_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&length, sizeof(length));
											
											rc = -1;
											while (rc < 0)
											{
												rc = send(client_sock, message.c_str(), message.size(), 0);
												if (rc < 0)
												{
													perror("send() failed");
													sleep(1);
													continue;
												}
											}
											
											not_full.notify_one();
										}
									}); // consumer uses consume and throw it to program_2 by socket

	producer.join();
	consumer.join();
}

void produce(std::string& input, std::queue<std::string>& queue)
{
	if ((input.length() > MAX_LENGTH) || !isDigitString(input))
	{
		std::cerr << "Incorrect input string!\n";
		return;
	}
	std::sort(input.begin(), input.end());
	std::reverse(input.begin(), input.end());

	auto for_upper_bound = input.length();
	for (size_t i = 0; i < for_upper_bound; i++)
	{
		if ((input[i] - '0') % 2 == 0)
		{
			input.replace(i++, 1, "KB");
			++for_upper_bound;
		}
	}

	queue.push(input);
}

bool isDigitString(const std::string& string)
{
	return std::none_of(string.begin(), string.end(), [](char c)
	{
		return !std::isdigit(c);
	});
}

int consume(std::queue<std::string>& queue)
{
	std::string string = queue.front();
	queue.pop();

	int sum = 0;
	for (char c : string)
	{
		if (std::isdigit(c))
		{
			sum += c - '0';
		}
	}
	std::cout << string << " ; sum = " << sum << '\n';
	return sum;
}
