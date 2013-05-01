#include <sys/unistd.h>
#include <sys/fcntl.h>

#include "net/udpnet.h"

#if defined WIN32
#include <ws2tcpip.h>
#include <ctype.h>
typedef int socklen_t;

static int inet_pton4(const char *src, char *dst);
static int inet_pton6(const char *src, char *dst);

static int inet_pton(int af, const char *src, char *dst)
{
    switch (af)
    {
    case AF_INET:
        return inet_pton4(src, dst);
    case AF_INET6:
        return inet_pton6(src, dst);
    default:
        return -1;
    }
}

#define NS_INADDRSZ  4
#define NS_IN6ADDRSZ 16
#define NS_INT16SZ   2

static int inet_pton4(const char *src, char *dst)
{
    uint8_t tmp[NS_INADDRSZ], *tp;

    int saw_digit = 0;
    int octets = 0;
    *(tp = tmp) = 0;

    int ch;
    while ((ch = *src++) != '\0')
    {
        if (ch >= '0' && ch <= '9')
        {
            uint32_t n = *tp * 10 + (ch - '0');

            if (saw_digit && *tp == 0)
                return 0;

            if (n > 255)
                return 0;

            *tp = n;
            if (!saw_digit)
            {
                if (++octets > 4)
                    return 0;
                saw_digit = 1;
            }
        }
        else if (ch == '.' && saw_digit)
        {
            if (octets == 4)
                return 0;
            *++tp = 0;
            saw_digit = 0;
        }
        else
            return 0;
    }
    if (octets < 4)
        return 0;

    memcpy(dst, tmp, NS_INADDRSZ);

    return 1;
}

static int inet_pton6(const char *src, char *dst)
{
    static const char xdigits[] = "0123456789abcdef";
    uint8_t tmp[NS_IN6ADDRSZ];

    uint8_t *tp = (uint8_t*) memset(tmp, '\0', NS_IN6ADDRSZ);
    uint8_t *endp = tp + NS_IN6ADDRSZ;
    uint8_t *colonp = NULL;

    /* Leading :: requires some special handling. */
    if (*src == ':')
    {
        if (*++src != ':')
            return 0;
    }

    const char *curtok = src;
    int saw_xdigit = 0;
    uint32_t val = 0;
    int ch;
    while ((ch = tolower(*src++)) != '\0')
    {
        const char *pch = strchr(xdigits, ch);
        if (pch != NULL)
        {
            val <<= 4;
            val |= (pch - xdigits);
            if (val > 0xffff)
                return 0;
            saw_xdigit = 1;
            continue;
        }
        if (ch == ':')
        {
            curtok = src;
            if (!saw_xdigit)
            {
                if (colonp)
                    return 0;
                colonp = tp;
                continue;
            }
            else if (*src == '\0')
            {
                return 0;
            }
            if (tp + NS_INT16SZ > endp)
                return 0;
            *tp++ = (uint8_t) (val >> 8) & 0xff;
            *tp++ = (uint8_t) val & 0xff;
            saw_xdigit = 0;
            val = 0;
            continue;
        }
        if (ch == '.' && ((tp + NS_INADDRSZ) <= endp) &&
                inet_pton4(curtok, (char*) tp) > 0)
        {
            tp += NS_INADDRSZ;
            saw_xdigit = 0;
            break; /* '\0' was seen by inet_pton4(). */
        }
        return 0;
    }
    if (saw_xdigit)
    {
        if (tp + NS_INT16SZ > endp)
            return 0;
        *tp++ = (uint8_t) (val >> 8) & 0xff;
        *tp++ = (uint8_t) val & 0xff;
    }
    if (colonp != NULL)
    {
        /*
         * Since some memmove()'s erroneously fail to handle
         * overlapping regions, we'll do the shift by hand.
         */
        const int n = tp - colonp;

        if (tp == endp)
            return 0;

        for (int i = 1; i <= n; i++)
        {
            endp[-i] = colonp[n - i];
            colonp[n - i] = 0;
        }
        tp = endp;
    }
    if (tp != endp)
        return 0;

    memcpy(dst, tmp, NS_IN6ADDRSZ);

    return 1;
}

#else
#define closesocket(s) close(s)
#endif

static int udp_err_ = 0;

static int get_last_error(void) {
#ifdef WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}

int udp_error(void)
{
	return udp_err_;
}


void udp_socket_init(void)
{
#if defined WIN32
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2,2), &wsa_data);
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
		udp_err_ = get_last_error();
		return s;
	}

#ifdef WIN32
	u_long i_mode = 1;
	int r = ioctlsocket(s, FIONBIO, &i_mode);
	if (r < 0) {
		udp_err_ = get_last_error();
		return r;
	}
#else
	fcntl(s, F_SETFL, O_NONBLOCK);
#endif
	
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
			udp_err_ = get_last_error();
			return ret;
		}
	}

	udp_err_ = 0;

	return s;
}


int udp_close_endpoint(int s)
{
	int ret = closesocket(s);

	udp_err_ = get_last_error();

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

#ifdef WIN32
	// already set nonblock using iosock nonsense
	ret = (int)recvfrom(s, data, size, 0, sender, &slen);
#else
	ret = (int)recvfrom(s, data, size, MSG_DONTWAIT, sender, &slen);
#endif
	udp_err_ = get_last_error();


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

#ifdef WIN32
	if (inet_pton(AF_INET, address, (char *)&theiraddr.sin_addr) != 1) {
		udp_err_ = get_last_error();
		return -1;
	}
#else
	if(inet_aton(address, &theiraddr.sin_addr) == 0)
	{
		udp_err_ = errno;
		return -1;
	}
#endif

#ifdef WIN32
	// Set using IOctrl earlier
	const int flags = 0;
#else
	const int flags = MSG_DONTWAIT;
#endif
	if(sendto(s, data, size, flags, receiver, lenaddr) < 0)
	{
		udp_err_ = get_last_error();
		return -2;
	}

	udp_err_ = get_last_error();

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
