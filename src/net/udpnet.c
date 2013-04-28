#include <sys/unistd.h>
#include <sys/fcntl.h>

#include "net/udpnet.h"

#if defined WIN32
typedef int socklen_t;
#else
#define closesocket(s) close(s)
#endif

static int udp_err_ = 0;

int udp_error(void)
{
	return udp_err_;
}


void udp_socket_init(void)
{
#if defined WIN32
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(1,1), &wsa_data);
#endif
}


void udp_socket_exit(void)
{
#if defined WIN32
	WSACleanup();
#endif
}


int udp_create_endpoint(int port)
{
	struct sockaddr_in si_me;
	int s;
	const struct sockaddr *me;

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if(s < 0)
	{
		udp_err_ = errno;

		return s;
	}

	fcntl(s, F_SETFL, O_NONBLOCK);
	
	if(port)
	{
		int ret;

		memset(&si_me, 0, sizeof(si_me));

		si_me.sin_family = AF_INET;
		si_me.sin_port = htons(port);
		si_me.sin_addr.s_addr = htonl(INADDR_ANY);

		me = (const struct sockaddr*)&si_me;

		ret = bind(s, me, sizeof(si_me));

		if(ret < 0)
		{
			udp_err_ = errno;
			return ret;
		}
	}

	udp_err_ = 0;

	return s;
}


int udp_close_endpoint(int s)
{
	int ret = closesocket(s);

	udp_err_ = errno;

	return ret;
}


int udp_receive(int s, void *data, int size, UDPNET_ADDRESS *src)
{
	struct sockaddr_in si_other;
	socklen_t slen = sizeof(si_other);
	struct sockaddr *sender = NULL;
	int ret;

	if(src)
	{
		sender = (struct sockaddr*)&si_other;
	}

	ret = (int)recvfrom(s, data, size, MSG_DONTWAIT, sender, &slen);

	udp_err_ = errno;

	if(ret < 0)
	{
		return ret;
	}

	if(src)
	{
		src->port = ntohs(si_other.sin_port);
		strcpy(src->address, inet_ntoa(si_other.sin_addr));
	}

	return ret;
}


int udp_send(int s, void *data, int size, const char *address,
	int port)
{
	struct sockaddr_in theiraddr;
	int lenaddr = sizeof(theiraddr);
	struct sockaddr *receiver;

	if(!address) return -1;

	memset(&theiraddr, 0, lenaddr);

	theiraddr.sin_family = AF_INET;
	theiraddr.sin_port = htons(port);

	receiver = (struct sockaddr *)&theiraddr;

	if(inet_aton(address, &theiraddr.sin_addr) == 0)
	{
		udp_err_ = errno;
		return -1;
	}

	if(sendto(s, data, size, MSG_DONTWAIT, receiver, lenaddr) < 0)
	{
		udp_err_ = errno;

		return -2;
	}

	udp_err_ = errno;

	return 0;
}


/*
#define LOCAL_IP	"127.0.0.1"

#define BUFLEN 		512
#define NPACK 		10

#define SERVER_PORT 9001
#define CLIENT_PORT 9002


int client_test(SOCKET s)
{
	char buf[BUFLEN];
	int i;

	for(i = 0; i < NPACK; i++)
	{
		sprintf(buf, "This is SPARTA!!!!! %d", i);

		if(udp_send(s, buf, BUFLEN, LOCAL_IP, SERVER_PORT) < 0)
		{
			perror("udp_send:\n");
			break;
		}
	}

	close(s);

	return 1;
}


int server_test(SOCKET s)
{
	int i;
	char buf[BUFLEN];
	UDPNET_ADDRESS addr;

	i = 0;

	while(i < NPACK)
	{
		int ret = udp_receive(s, buf, BUFLEN, &addr);

		if(ret < 0)
		{
			if(udp_error() == EWOULDBLOCK)
			{
				continue;
			}
			else if(udp_error() == EAGAIN)
			{
				continue;
			}
			else
			{
				return -1;
			}
		}

		printf("Received %d bytes from %s:%d\n",
			ret,
			addr.address,
			addr.port);


		printf("DATA: %s\n", buf);

		i++;
	}

	return 0;
}


static void error_out(const char *str)
{
	perror(str);
	exit(1);
}


int main(void)
{
	SOCKET server, client;

	udp_socket_init();

	server = udp_create_endpoint(SERVER_PORT);
	if(server < 0) error_out("socket");

	client = udp_create_endpoint(0);
	if(client < 0) error_out("socket");

	client_test(client);
	server_test(server);

	udp_close_endpoint(client);
	udp_close_endpoint(server);

	udp_socket_exit();
}
*/
