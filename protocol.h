#ifndef _PROTOCOL_H
#define _PROTOCOL_H
#define h_addr h_addr_list[0]

#include <stdio.h>
#include <jansson.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <netdb.h>
#include "queue.h"
#include "sha256.h"
#include "util.h"

typedef enum {SUBS_SENT, AUTH_SENT, AUTHORIZED} stratum_state;

typedef struct block_header {
        unsigned int    version;
        // dont let the "char" fool you, this is binary data not the human readable version
        uint32_t prev_block[8];
        uint32_t merkle_root[8];
        // unsigned char   prev_block[32];
        // unsigned char   merkle_root[32];
        unsigned int    timestamp;
        unsigned int    bits;
        unsigned int    nonce;
        uint32_t padding[12];
} block_header;
 

struct work {
	uint32_t data[32];
	uint32_t target[8];

	char *job_id;

	size_t nonce2_size;
	unsigned char *nonce2;
};

struct stratum_job {
	char *job_id;
	unsigned char prevhash[32];
	size_t coinbase_size;
	unsigned char *coinbase;
	unsigned char *nonce2;
	size_t merkle_count;
	unsigned char **merkle;
	char *merkle_root;
	unsigned char version[4];
	unsigned char nbits[4];
	unsigned char ntime[4];
	bool clean;
	double diff;
};

struct stratum_connection {
	int socket;
	unsigned int send_id;
	pthread_mutex_t send_queue_mutex;
	connection_buffer *recv_queue;
	connection_buffer *send_queue;
};

struct stratum_context {
	stratum_state state;
	char *user;
	unsigned int send_id;
	double next_diff;
	// char *subscription_id;
	// char *notification_id;
	char *session_id;
	size_t nonce1_size;
	unsigned char *nonce1;
	size_t nonce2_size;
	struct stratum_job job;
	pthread_mutex_t context_mutex;
};

/* Connection */
struct stratum_connection* create_stratum_connection(char *ip, char *port);
void destruct_stratum_connection(struct stratum_connection *connection);
size_t get_message(struct stratum_connection* connection, char **message);
int send_messages(struct stratum_connection* connection);

/* Senders */
void stratum_submit_share(struct stratum_connection *connection, 
	struct stratum_context *context,  struct work *work);
void stratum_authorize(struct stratum_connection *connection,
	const char *user, const char *password);
void stratum_subscribe(struct stratum_connection *connection);
void stratum_client_version(struct stratum_connection *connection, json_t *json_obj);

/* Receivers */
void stratum_load_subscription(struct stratum_context *context, json_t *json_message);
void stratum_notify(struct stratum_context *context, json_t *json_obj);
void stratum_set_next_job_difficulty(struct stratum_context *context, json_t *json_message);

/* Work generators */
void stratum_generate_new_work(struct stratum_context *context, struct work* work);

/* Stratum context */
struct stratum_context* create_stratum_context();
void destruct_stratum_context(struct stratum_context *context);

#endif /* protocol.h */
