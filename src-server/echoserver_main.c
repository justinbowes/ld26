/*
 * udpserver.c - A simple UDP echo server
 * usage: udpserver <port>
 */
#ifndef WIN32
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <netdb.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "uthash.h"

#include "xpl.h"
#include "xpl_log.h"
#include "xpl_hash.h"

#include "game/game.h"
#include "game/packet.h"

#include "net/udpnet.h"

#define BUFSIZE 1024

#define TIMEOUT 5.0

typedef struct client_info {
	int					id;
	UDPNET_ADDRESS				remote_addr;
	uint32_t				seq;
	double					last_packet_time;
	player_id_t				player_id;
	bool					drop;
	UT_hash_handle				hh;
} client_info_t;

static client_info_t 	*clients 		= NULL;
static int 		client_count 		= 0;
static uint16_t 	client_uid_counter 	= 1;
static int 		sock			= 0;
static const char 	*motd 			= "motd.txt";
static double		uptime			= 0.0;

/*
 * error - wrapper for perror
 */
static void exit_error(char *msg) {
	LOG_ERROR("%s", msg);
	exit(1);
}

static void log_event(const char *type, client_info_t *client_info, const char *format, ...) {
	char timebuf[32];
	char buffer[1024];
	uint16_t cid = client_info ? client_info->player_id.client_id : 0;
	const char *cname = client_info ? client_info->player_id.name : "";
    	time_t now = time(0);
	va_list args;

	va_start(args, format);
	vsnprintf(buffer, 1024, format, args);
	va_end(args);
    	strftime(timebuf, 32, "%Y-%m-%d %H:%M:%S.000", localtime(&now));
	LOG_INFO("%s [%s] client_id=[%u,\"%s\"] data=[%s]", timebuf, type, cid, cname, buffer);
}

static void pointcast_buffer(uint8_t *buf, int size, client_info_t *client) {
	int ret = udp_send(sock, buf, size, client->remote_addr.address, client->remote_addr.port);
	if (ret) {
		log_event("send_drop", client, "size=%d", size);
		client->drop = true;
	}
}

static void pointcast_packet(uint16_t subject, packet_t *packet, client_info_t *client) {
	uint8_t buf[1024];
	size_t size = packet_encode(packet, subject, buf);
	pointcast_buffer(buf, (int)size, client);
}

static void broadcast_buffer(uint8_t *buf, int size) {
	client_info_t *dest, *tmp;
	HASH_ITER(hh, clients, dest, tmp) {
		pointcast_buffer(buf, size, dest);
	}
}


static void broadcast_packet(uint16_t subject, packet_t *packet) {
	uint8_t buf[1024];
	size_t size = packet_encode(packet, subject, buf);
	broadcast_buffer(buf, (int)size);
}

static void client_send_motd(client_info_t *client) {

	packet_t packet;
	packet.type = pt_chat;

	char stat_message[CHAT_MAX] = { 0 };
	snprintf(stat_message, CHAT_MAX, "Welcome to UltraPew! %d users, uptime %.2f h", client_count, uptime / 3600.0);
	strncpy(packet.chat, stat_message, CHAT_MAX);
	pointcast_packet(0, &packet, client);

	size_t bytes_read = 0;

	char file_message[CHAT_MAX] = { 0 };
	FILE *file = fopen(motd, "r");
	if (file) {
		bytes_read = fread(file_message, 1, CHAT_MAX - 1, file);
		fclose(file);
	}
	if (bytes_read) strncpy(packet.chat, file_message, CHAT_MAX);
	pointcast_packet(0, &packet, client);

}

static client_info_t *get_client(UDPNET_ADDRESS *remote_addr) {
	client_info_t *client;
	int hash = xpl_hashs(remote_addr->address, XPL_HASH_INIT);
	hash = xpl_hashi(remote_addr->port, hash);
	HASH_FIND_INT(clients, &hash, client);
	
	if (! client) {
		client = xpl_calloc_type(client_info_t);
		client->id = hash;
		client->remote_addr = *remote_addr;
		client->player_id.client_id = client_uid_counter++;
		++client_count;
		log_event("join", client, "ip=\"%s\",port=%d", remote_addr->address, remote_addr->port);
		HASH_ADD_INT(clients, id, client);
	}
	
	return client;
}

static void delete_client(client_info_t *client, const char *reason) {
	log_event("delete", client, "reason=\"%s\"", reason);
	packet_t bye;
	memset(&bye, 0, sizeof(bye));
	bye.type = pt_goodbye;
	bye.goodbye = client->player_id;
	broadcast_packet(client->player_id.client_id, &bye);

	HASH_DEL(clients, client);

	--client_count;	

	xpl_free(client);
}

static void purge_clients() {
	double time = xpl_get_time();
	client_info_t *dest, *tmp;
	HASH_ITER(hh, clients, dest, tmp) {
		
		if (dest->drop) {
			delete_client(dest, "drop");
		}
		
		if (time - dest->last_packet_time > TIMEOUT) {
			delete_client(dest, "timeout");
		}
	}
}

int main(int argc, char **argv) {
	uint8_t buf[BUFSIZE];				/* message buf */
	int n;							/* message byte size */
	
	xpl_init_timer();

	double initial_time = xpl_get_time();
	
	/*
	 * check command line arguments
	 */
	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}
	int portno = atoi(argv[1]);
	
	/* setsockopt: Handy debugging trick that lets
	 * us rerun the server immediately after we kill it;
	 * otherwise we have to wait about 20 secs.
	 * Eliminates "ERROR on binding: Address already in use" error.
	 */
	//	struct sockaddr_in serveraddr;	/* server's addr */
	//	int optval;						/* flag value for setsockopt */
	//	optval = 1;
	//	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
	//			   (const void *)&optval , sizeof(int));

	udp_socket_init();
	sock = udp_create_endpoint(portno);
	if (sock < 0) {
		exit_error("Couldn't bind endpoint");
	}
	
	LOG_INFO("Socket bound on port %d", portno);
	
	while (1) {
		memset(buf, 0, 1024);
		purge_clients();
		
		UDPNET_ADDRESS src;
		n = udp_receive(sock, buf, BUFSIZE, &src);
		
		if (n < 0) {
			int e = udp_error();
			if (e == EWOULDBLOCK ||
				e == EAGAIN) {
				xpl_sleep_seconds(0.01);
				continue;
			}
			exit_error("Error code from socket on receive");
		}
		
		LOG_DEBUG("Received packet");
		
		client_info_t *client_info = get_client(&src);

		uint16_t client_source;
		packet_t packet;
		if (! packet_decode(&packet, &client_source, buf)) {
			LOG_WARN("Malformed packet, dropping");
			continue;
		}
		
		if (packet.seq <= client_info->seq) {
			LOG_DEBUG("Dropping old packet %d", packet.seq);
			continue;
		}
		client_info->seq = packet.seq;
		
		if (packet.type == pt_hello) {
			
			if (client_count > 127) {
				
				client_info_t temp_client;
				memset(&temp_client, 0, sizeof(temp_client));
				temp_client.remote_addr = src;
				
				packet_t full_packet;
				full_packet.type = pt_chat;
				strncpy(full_packet.chat, "Server is full", CHAT_MAX);
				
				pointcast_packet(0, &packet, &temp_client);
		
				log_event("full", NULL, "");
				
				continue;
			}
			
			if (packet.hello.nonce && client_source == 0) {
				client_source = client_info->player_id.client_id;
				log_event("hello", client_info, "nonce=%u", packet.hello.nonce);
				packet.hello.client_id = client_info->player_id.client_id;
				pointcast_packet(client_source, &packet, client_info);
				client_send_motd(client_info);
				assert(client_source != 0);
			}
			strncpy(client_info->player_id.name, packet.hello.name, NAME_SIZE);
			// Overwrite the nonce so it's not shared
			packet.hello.nonce = 0;
		}
		
		if (packet.type == pt_chat) {
			packet.chat[63] = '\0';
			log_event("chat", client_info, "message=\"%s\"", packet.chat);
		}
		
		if (packet.type == pt_damage) {
			log_event("damage", client_info, "damage=%u,origin=%u,flags=%u",
					  packet.damage.amount,
					  packet.damage.player_id,
					  packet.damage.flags);
		}
		
		if (client_source != client_info->player_id.client_id) {
			LOG_WARN("Packet client_id mismatch (claim %u, have %u); kicking packet",
					 client_source, client_info->player_id.client_id);
		}
		
		double time = xpl_get_time();
		uptime = time - initial_time;
		client_info->last_packet_time = time;
		
		broadcast_packet(client_source, &packet);
		
	}
}
#endif
