#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#ifdef __APPLE__
#include "./endian.h"
#else
#include <endian.h>
#endif

#include "./peer.h"
#include "./sha256.h"


// Global variables to be used by both the server and client side of the peer.
// Some of these are not currently used but should be considered STRONG hints
PeerAddress_t *my_address;

pthread_mutex_t network_mutex = PTHREAD_MUTEX_INITIALIZER;
PeerAddress_t** network = NULL;
uint32_t peer_count = 0;

pthread_mutex_t retrieving_mutex = PTHREAD_MUTEX_INITIALIZER;
FilePath_t** retrieving_files = NULL;
uint32_t file_count = 0;


/*
 * Gets a sha256 hash of specified data, sourcedata. The hash itself is
 * placed into the given variable 'hash'. Any size can be created, but a
 * a normal size for the hash would be given by the global variable
 * 'SHA256_HASH_SIZE', that has been defined in sha256.h
 */
void get_data_sha(const char* sourcedata, hashdata_t hash, uint32_t data_size, 
    int hash_size)
{
  SHA256_CTX shactx;
  unsigned char shabuffer[hash_size];
  sha256_init(&shactx);
  sha256_update(&shactx, sourcedata, data_size);
  sha256_final(&shactx, shabuffer);

  for (int i=0; i<hash_size; i++)
  {
    hash[i] = shabuffer[i];
  }
}

/*
 * Gets a sha256 hash of specified data file, sourcefile. The hash itself is
 * placed into the given variable 'hash'. Any size can be created, but a
 * a normal size for the hash would be given by the global variable
 * 'SHA256_HASH_SIZE', that has been defined in sha256.h
 */
void get_file_sha(const char* sourcefile, hashdata_t hash, int size)
{
    int casc_file_size;

    FILE* fp = fopen(sourcefile, "rb");
    if (fp == 0)
    {
        printf("Failed to open source: %s\n", sourcefile);
        return;
    }

    fseek(fp, 0L, SEEK_END);
    casc_file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    char buffer[casc_file_size];
    fread(buffer, casc_file_size, 1, fp);
    fclose(fp);

    get_data_sha(buffer, hash, casc_file_size, size);
}

/*
 * A simple min function, which apparently C doesn't have as standard
 */
uint32_t min(int a, int b)
{
    if (a < b) 
    {
        return a;
    }
    return b;
}

/*
 * Select a peer from the network at random, without picking the peer defined
 * in my_address
 */
void get_random_peer(PeerAddress_t* peer_address)
{ 
    PeerAddress_t** potential_peers = malloc(sizeof(PeerAddress_t*));
    uint32_t potential_count = 0; 
    for (uint32_t i=0; i<peer_count; i++)
    {
        if (strcmp(network[i]->ip, my_address->ip) != 0 
                || strcmp(network[i]->port, my_address->port) != 0 )
        {
            potential_peers = realloc(potential_peers, 
                (potential_count+1) * sizeof(PeerAddress_t*));
            potential_peers[potential_count] = network[i];
            potential_count++;
        }
    }

    if (potential_count == 0)
    {
        printf("No peers to connect to. You probably have not implemented "
            "registering with the network yet.\n");
    }

    uint32_t random_peer_index = rand() % potential_count;

    memcpy(peer_address->ip, potential_peers[random_peer_index]->ip, IP_LEN);
    memcpy(peer_address->port, potential_peers[random_peer_index]->port, 
        PORT_LEN);

    free(potential_peers);

    printf("Selected random peer: %s:%s\n", 
        peer_address->ip, peer_address->port);
}

/*
 * Send a request message to another peer on the network. Unless this is 
 * specifically an 'inform' message as described in the assignment handout, a 
 * reply will always be expected.
 */
void send_message(PeerAddress_t peer_address, int command, char* request_body)
{
    fprintf(stdout, "Connecting to server at %s:%s to run command %d (%s)\n", 
        peer_address.ip, peer_address.port, command, request_body);

    compsys_helper_state_t state;
    char msg_buf[MAX_MSG_LEN];
    FILE* fp;

    // Setup the eventual output file path. This is being done early so if 
    // something does go wrong at this stage we can avoid all that pesky 
    // networking
    char output_file_path[strlen(request_body)+1];
    if (command == COMMAND_RETREIVE)
    {     
        strcpy(output_file_path, request_body);

        if (access(output_file_path, F_OK ) != 0 ) 
        {
            fp = fopen(output_file_path, "a");
            fclose(fp);
        }
    }

    // Setup connection
    int peer_socket = compsys_helper_open_clientfd(peer_address.ip, peer_address.port);
    compsys_helper_readinitb(&state, peer_socket);

    // Construct a request message and send it to the peer
    struct RequestHeader request_header;
    strncpy(request_header.ip, my_address->ip, IP_LEN);
    request_header.port = htonl(atoi(my_address->port));
    request_header.command = htonl(command);
    request_header.length = htonl(strlen(request_body));

    memcpy(msg_buf, &request_header, REQUEST_HEADER_LEN);
    memcpy(msg_buf+REQUEST_HEADER_LEN, request_body, strlen(request_body));

    compsys_helper_writen(peer_socket, msg_buf, REQUEST_HEADER_LEN+strlen(request_body));

    // We don't expect replies to inform messages so we're done here
    if (command == COMMAND_INFORM)
    {
        return;
    }

    // Read a reply
    compsys_helper_readnb(&state, msg_buf, REPLY_HEADER_LEN);

    // Extract the reply header 
    char reply_header[REPLY_HEADER_LEN];
    memcpy(reply_header, msg_buf, REPLY_HEADER_LEN);

    uint32_t reply_length = ntohl(*(uint32_t*)&reply_header[0]);
    uint32_t reply_status = ntohl(*(uint32_t*)&reply_header[4]);
    uint32_t this_block = ntohl(*(uint32_t*)&reply_header[8]);
    uint32_t block_count = ntohl(*(uint32_t*)&reply_header[12]);
    hashdata_t block_hash;
    memcpy(block_hash, &reply_header[16], SHA256_HASH_SIZE);
    hashdata_t total_hash;
    memcpy(total_hash, &reply_header[48], SHA256_HASH_SIZE);

    // Determine how many blocks we are about to recieve
    hashdata_t ref_hash;
    memcpy(ref_hash, &total_hash, SHA256_HASH_SIZE);
    uint32_t ref_count = block_count;

    // Loop until all blocks have been recieved
    for (uint32_t b=0; b<ref_count; b++)
    {
        // Don't need to re-read the first block
        if (b > 0)
        {
            // Read the response
            compsys_helper_readnb(&state, msg_buf, REPLY_HEADER_LEN);

            // Read header
            memcpy(reply_header, msg_buf, REPLY_HEADER_LEN);

            // Parse the attributes
            reply_length = ntohl(*(uint32_t*)&reply_header[0]);
            reply_status = ntohl(*(uint32_t*)&reply_header[4]);
            this_block = ntohl(*(uint32_t*)&reply_header[8]);
            block_count = ntohl(*(uint32_t*)&reply_header[12]);

            memcpy(block_hash, &reply_header[16], SHA256_HASH_SIZE);
            memcpy(total_hash, &reply_header[48], SHA256_HASH_SIZE);

            // Check we're getting consistent results
            if (ref_count != block_count)
            {
                fprintf(stdout, 
                    "Got inconsistent block counts between blocks\n");
                close(peer_socket);
                return;
            }

            for (int i=0; i<SHA256_HASH_SIZE; i++)
            {
                if (ref_hash[i] != total_hash[i])
                {
                    fprintf(stdout, 
                        "Got inconsistent total hashes between blocks\n");
                    close(peer_socket);
                    return;
                }
            }
        }

        // Check response status
        if (reply_status != STATUS_OK)
        {
            if (command == COMMAND_REGISTER && reply_status == STATUS_PEER_EXISTS)
            {
                printf("Peer already exists\n");
            }
            else
            {
                printf("Got unexpected status %d\n", reply_status);
                close(peer_socket);
                return;
            }
        }

        // Read the payload
        char payload[reply_length+1];
        compsys_helper_readnb(&state, msg_buf, reply_length);
        memcpy(payload, msg_buf, reply_length);
        payload[reply_length] = '\0';
        
        // Check the hash of the data is as expected
        hashdata_t payload_hash;
        get_data_sha(payload, payload_hash, reply_length, SHA256_HASH_SIZE);

        for (int i=0; i<SHA256_HASH_SIZE; i++)
        {
            if (payload_hash[i] != block_hash[i])
            {
                fprintf(stdout, "Payload hash does not match specified\n");
                close(peer_socket);
                return;
            }
        }

        // If we're trying to get a file, actually write that file
        if (command == COMMAND_RETREIVE)
        {
            // Check we can access the output file
            fp = fopen(output_file_path, "r+b");
            if (fp == 0)
            {
                printf("Failed to open destination: %s\n", output_file_path);
                close(peer_socket);
            }

            uint32_t offset = this_block * (MAX_MSG_LEN-REPLY_HEADER_LEN);
            fprintf(stdout, "Block num: %d/%d (offset: %d)\n", this_block+1, 
                block_count, offset);
            fprintf(stdout, "Writing from %d to %d\n", offset, 
                offset+reply_length);

            // Write data to the output file, at the appropriate place
            fseek(fp, offset, SEEK_SET);
            fputs(payload, fp);
            fclose(fp);
        }
    }

    // Confirm that our file is indeed correct
    if (command == COMMAND_RETREIVE)
    {
        fprintf(stdout, "Got data and wrote to %s\n", output_file_path);

        // Finally, check that the hash of all the data is as expected
        hashdata_t file_hash;
        get_file_sha(output_file_path, file_hash, SHA256_HASH_SIZE);

        for (int i=0; i<SHA256_HASH_SIZE; i++)
        {
            if (file_hash[i] != total_hash[i])
            {
                fprintf(stdout, "File hash does not match specified for %s\n", 
                    output_file_path);
                close(peer_socket);
                return;
            }
        }
    }

    // If we are registering with the network we should note the complete 
    // network reply
    char* reply_body = malloc(reply_length + 1);
    memset(reply_body, 0, reply_length + 1);
    memcpy(reply_body, msg_buf, reply_length);

    if (reply_status == STATUS_OK)
    {
        if (command == COMMAND_REGISTER)
        {
            // Your code here. This code has been added as a guide, but feel 
            // free to add more, or work in other parts of the code

            // Lock network mutex for client thread
            pthread_mutex_lock(&network_mutex);
            
            // Update peer count
            peer_count = reply_length / (IP_LEN + 4);

            // Update network memory
            network = (PeerAddress_t**) realloc(network, sizeof(PeerAddress_t*) * peer_count);

            // Update network 
            for (uint32_t i = 0; i < peer_count; i++) {

              uint32_t ip_index = i * 20;
              uint32_t port_index = ip_index + 16;
              
              // Convert port to host-byte-order (uint32_t)
              uint32_t reply_port_h;
              memcpy(&reply_port_h, &reply_body[port_index], 4);
              reply_port_h = ntohl(reply_port_h);

              // Struct for peer address
              PeerAddress_t* peer = malloc(sizeof(PeerAddress_t));
              memcpy(peer->ip, &reply_body[ip_index], 16);
              sprintf(peer->port, "%u", reply_port_h); // <-- uint32_t (unsigned int) to string 

              // Add peer address to network
              network[i] = peer;
            }

            //Interaction
            printf("Got network: ");
            for (uint32_t i = 0; i < peer_count; i++) {
              printf("%s:%s", network[i]->ip, network[i]->port);
              if (i != peer_count - 1) printf(", ");
            };
            printf("\n");

            // Unlock network mutex for client thread
            pthread_mutex_unlock(&network_mutex);

        }
    } 
    else
    {
        printf("Got response code: %d, %s\n", reply_status, reply_body);
    }
    free(reply_body);
    close(peer_socket);
}


/*
 * Function to act as thread for all required client interactions. This thread 
 * will be run concurrently with the server_thread but is finite in nature.
 * 
 * This is just to register with a network, then download two files from a 
 * random peer on that network. As in A3, you are allowed to use a more 
 * user-friendly setup with user interaction for what files to retrieve if 
 * preferred, this is merely presented as a convienient setup for meeting the 
 * assignment tasks
 */ 
void* client_thread(void* thread_args)
{
    struct PeerAddress *peer_address = thread_args;

    // Register the given user
    send_message(*peer_address, COMMAND_REGISTER, "\0");

    // Update peer_address with random peer from network
    // get_random_peer(peer_address);
    // sleep(1); // <-- SLEEP FOR DEBUG

    // Retrieve the smaller file, that doesn't not require support for blocks
    // send_message(*peer_address, COMMAND_RETREIVE, "tiny.txt");
    // sleep(1); // <-- SLEEP FOR DEBUG

    // Update peer_address with random peer from network
    // get_random_peer(peer_address);
    // sleep(1); // <-- SLEEP FOR DEBUG

    // Retrieve the larger file, that requires support for blocked messages
    // send_message(*peer_address, COMMAND_RETREIVE, "hamlet.txt");

    return NULL;
}

/*
 * Handle any 'register' type requests, as defined in the asignment text. This
 * should always generate a response.
 */
void handle_register(int connfd, char* client_ip, int client_port_int)
{
    // Your code here. This function has been added as a guide, but feel free 
    // to add more, or work in other parts of the code

    // DEBUG
    printf("handle_register()\n");

    // Lock network mutex
    pthread_mutex_lock(&network_mutex);

    // State
    uint32_t body_length = (IP_LEN + 4) * peer_count;
    uint32_t status;
    uint32_t block_number;
    uint32_t block_count;

    // Make a response header and sent it to the client peer
    struct ReplyHeader reply_header;
    reply_header.length = htonl(body_length);
    reply_header.status = htonl(status);
    reply_header.this_block = htonl(block_number);
    reply_header.block_count = htonl(block_count);
    //get_data_sha(network, reply_header.block_hash, sizeof(network), SHA256_HASH_SIZE); // << OBS!
    //get_data_sha(network, reply_header.total_hash, sizeof(network), SHA256_HASH_SIZE); // << OBS!


    // Unlock network mutex
    pthread_mutex_unlock(&network_mutex);
}

/*
 * Handle 'inform' type message as defined by the assignment text. These will 
 * never generate a response, even in the case of errors.
 */
void handle_inform(char* request)
{
    // Your code here. This function has been added as a guide, but feel free 
    // to add more, or work in other parts of the code

    // Error handling: 
    // Can only recieve inform messages if on the network
    // Check if the address the request is from is already on the network.
    // If not, then don't handle the inform request

    // Convert port to host-byte-order (uint32_t)
    uint32_t port_hbol;
    memcpy(&port_hbol, &request[IP_LEN], 4);
    port_hbol = ntohl(port_hbol);

    // Create new peer address struct
    PeerAddress_t* new_peer = malloc(sizeof(PeerAddress_t));
    memcpy(new_peer->ip, request, IP_LEN);
    sprintf(new_peer->port, "%u", port_hbol);

    // Update network list
    pthread_mutex_lock(&network_mutex);

    peer_count += 1;
    network = (PeerAddress_t**) realloc(network, (sizeof(PeerAddress_t*)) * peer_count);
    network[peer_count - 1] = new_peer;

    pthread_mutex_unlock(&network_mutex);

    // Interaction
    printf("Informed of new peer: %s:%s\n", new_peer->ip, new_peer->port);
}



/*
 * Handle 'retrieve' type messages as defined by the assignment text. This will
 * always generate a response
 */
void handle_retrieve(int connfd, char* request)
{
    // Your code here. This function has been added as a guide, but feel free 
    // to add more, or work in other parts of the code

    // Lock retrieving mutex
    pthread_mutex_lock(&retrieving_mutex);

    char* filepath = request;
    uint32_t length;
    uint32_t status;
    uint32_t file_size;
    uint32_t total_block_count;
    uint32_t max_payload_len = MAX_MSG_LEN - REPLY_HEADER_LEN;
    char payload[max_payload_len];
    char response_msg[MAX_MSG_LEN];

    // Reply Header
    struct ReplyHeader reply_header;

    // Open file stream
    FILE* file = fopen(filepath, "r");
    if (file == NULL) {
      // If file doesn't exist, respond with a Bad Request message
      char bad_request[max_payload_len];
      sprintf(bad_request, "Requested content %s does not exist", filepath);
      printf("%s\n", bad_request);

      // Write bad request message to payload
      status = STATUS_BAD_REQUEST;
      file_size = strlen(bad_request);
      strncpy(payload, bad_request, file_size);

    } else {
      // Retrieve file
      printf("Sending requested data from %s\n", filepath);
      status = STATUS_OK;
    }

    // Get file size
    if (status == STATUS_OK) {
      fseek(file, 0, SEEK_END);
      file_size = ftell(file); 
      rewind(file);
    }

    // Get total block count
    total_block_count = (file_size / max_payload_len) + 1;

    // Loop until all blocks have been sent (in packets)
    for (uint32_t i = 0; i < total_block_count; i++) {

      // Set payload length
      if (i + 1 == total_block_count) {
        length = file_size - ((total_block_count - 1) * max_payload_len);
      } else {
        length = max_payload_len;
      }

      // Write file data to payload buffer
      if (status == STATUS_OK) {
        memset(payload, '\0', max_payload_len);
        fread(payload, sizeof(char), length, file);
      }

      // Update Reply Header
      reply_header.length = length;
      reply_header.status = status;
      reply_header.block_count = total_block_count;
      reply_header.this_block = i;
      get_data_sha(payload, reply_header.block_hash, length, SHA256_HASH_SIZE);
      if (file == NULL) {
        get_data_sha(payload, reply_header.total_hash, length, SHA256_HASH_SIZE);
      } else {
        get_file_sha(filepath, reply_header.total_hash, SHA256_HASH_SIZE);
      }
      
      // Interaction
      printf("Sending reply %u/%u with payload length of %u bytes\n", 
        reply_header.this_block + 1, 
        reply_header.block_count,
        reply_header.length
      );

      // Compose response message
      reply_to_net(&reply_header);
      memcpy(&response_msg, &reply_header, REPLY_HEADER_LEN);
      memcpy(&response_msg[REPLY_HEADER_LEN], &payload, length);

      // Send response packet
      compsys_helper_writen(connfd, response_msg, MAX_MSG_LEN);
    }
    fclose(file);

    // Unlock mutex
    pthread_mutex_unlock(&retrieving_mutex);
}

/*
 * Handler for all server requests. This will call the relevent function based 
 * on the parsed command code
 */
void* handle_server_request(void* vargp)
{
    // Your code here. This function has been added as a guide, but feel free 
    // to add more, or work in other parts of the code

    // Detach connfd connection as an independent thread
    int connfd = *((int*) vargp);
    pthread_detach(pthread_self());
    free(vargp);

    // Read incoming request header    
    struct RequestHeader request_header;
    compsys_helper_state_t helper_state;
    compsys_helper_readinitb(&helper_state, connfd);
    compsys_helper_readnb(&helper_state, &request_header, REQUEST_HEADER_LEN);
    request_header.port = ntohl(request_header.port);
    request_header.command = ntohl(request_header.command);
    request_header.length = ntohl(request_header.length);

    // Read incoming body (payload) if the commands are 'retrieve' or 'inform'
    char request_body[request_header.length];
    if (request_header.command != COMMAND_REGISTER) {
      compsys_helper_readnb(&helper_state, &request_body, request_header.length);
    }

    // Write incoming peer IP and port to PeerAddress struct
    struct PeerAddress peer_address;
    memcpy(&peer_address.ip, request_header.ip, IP_LEN);
    sprintf(peer_address.port, "%u", request_header.port);

    // Interaction
    fprintf(stdout, 
      "Got request from %s:%u, with Command: %u, Payload length: %u\n",
      request_header.ip, 
      request_header.port, 
      request_header.command, 
      request_header.length
    );

    // Handle request commands
    switch (request_header.command) {
      case COMMAND_REGISTER:
        // printf("Got register message from %s:%u\n", request_header.ip, request_header.port);
        handle_register(connfd, peer_address.ip, atoi(peer_address.port));
        break;
      case COMMAND_RETREIVE:
        // printf("Got retrieve message from %s:%u\n", request_header.ip, request_header.port);
        handle_retrieve(connfd, request_body);
        break;
      case COMMAND_INFORM:
        // printf("Got inform message from %s:%u\n", request_header.ip, request_header.port);
        handle_inform(request_body);
        break;
      default:
        // ERROR handling
        printf("Unable to read incoming request command\n");
        break;
    }

    // Close port connection and return
    close(connfd);
    return NULL;
}

/*
 * Function to act as basis for running the server thread. This thread will be
 * run concurrently with the client thread, but is infinite in nature.
 */
void* server_thread()
{
    // Your code here. This function has been added as a guide, but feel free 
    // to add more, or work in other parts of the code

    // Open concurrent listening port (as independent thread) 
    // and make it call handle_server_request. 
    // Infinite while-loop waits for incoming connections.
    // Ref: Implementation taken from CompSys Lecture 30/10/2023, slide 15-16 
    printf("Starting server at: %s:%s\n", my_address->ip, my_address->port);

    int listenfd;
    listenfd = compsys_helper_open_listenfd(my_address->port);
    
    pthread_t tid;
    int* connfdp;
    socklen_t clientlen;
    struct sockaddr clientaddr;
    while(1) {
      clientlen = sizeof(struct sockaddr_storage);
      connfdp = malloc(sizeof(int));
      *connfdp = accept(listenfd, &clientaddr, &clientlen);
      pthread_create(&tid, NULL, handle_server_request, connfdp);
    };

    close(listenfd);
    return NULL;
}


int main(int argc, char **argv)
{
    // Initialise with known junk values, so we can test if these were actually
    // present in the config or not
    struct PeerAddress peer_address;
    memset(peer_address.ip, '\0', IP_LEN);
    memset(peer_address.port, '\0', PORT_LEN);
    memcpy(peer_address.ip, "x", 1);
    memcpy(peer_address.port, "x", 1);

    // Users should call this script with a single argument describing what 
    // config to use
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(EXIT_FAILURE);
    } 

    my_address = (PeerAddress_t*)malloc(sizeof(PeerAddress_t));
    memset(my_address->ip, '\0', IP_LEN);
    memset(my_address->port, '\0', PORT_LEN);

    // Read in configuration options. Should include a client_ip, client_port, 
    // server_ip, and server_port
    char buffer[128];
    fprintf(stderr, "Got config path at: %s\n", argv[1]);
    FILE* fp = fopen(argv[1], "r");
    while (fgets(buffer, 128, fp)) {
        if (starts_with(buffer, MY_IP)) {
            memcpy(&my_address->ip, &buffer[strlen(MY_IP)], 
                strcspn(buffer, "\r\n")-strlen(MY_IP));
            if (!is_valid_ip(my_address->ip)) {
                fprintf(stderr, ">> Invalid client IP: %s\n", my_address->ip);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, MY_PORT)) {
            memcpy(&my_address->port, &buffer[strlen(MY_PORT)], 
                strcspn(buffer, "\r\n")-strlen(MY_PORT));
            if (!is_valid_port(my_address->port)) {
                fprintf(stderr, ">> Invalid client port: %s\n", 
                    my_address->port);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, PEER_IP)) {
            memcpy(peer_address.ip, &buffer[strlen(PEER_IP)], 
                strcspn(buffer, "\r\n")-strlen(PEER_IP));
            if (!is_valid_ip(peer_address.ip)) {
                fprintf(stderr, ">> Invalid peer IP: %s\n", peer_address.ip);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, PEER_PORT)) {
            memcpy(peer_address.port, &buffer[strlen(PEER_PORT)], 
                strcspn(buffer, "\r\n")-strlen(PEER_PORT));
            if (!is_valid_port(peer_address.port)) {
                fprintf(stderr, ">> Invalid peer port: %s\n", 
                    peer_address.port);
                exit(EXIT_FAILURE);
            }
        }
    }
    fclose(fp);

    retrieving_files = malloc(file_count * sizeof(FilePath_t*));
    srand(time(0));

    network = malloc(sizeof(PeerAddress_t*));
    network[0] = my_address;
    peer_count = 1;

    // Setup the client and server threads 
    pthread_t client_thread_id;
    pthread_t server_thread_id;
    if (peer_address.ip[0] != 'x' && peer_address.port[0] != 'x')
    {   
        pthread_create(&client_thread_id, NULL, client_thread, &peer_address);
    } 
    pthread_create(&server_thread_id, NULL, server_thread, NULL);

    // Start the threads. Note that the client is only started if a peer is 
    // provided in the config. If none is we will assume this peer is the first
    // on the network and so cannot act as a client.
    if (peer_address.ip[0] != 'x' && peer_address.port[0] != 'x')
    {
        pthread_join(client_thread_id, NULL);
    }
    pthread_join(server_thread_id, NULL);

    exit(EXIT_SUCCESS);
}




// Helper function converting Reply Header to host byte order
void reply_to_host(ReplyHeader_t* reply_header) {
  reply_header->length = ntohl(reply_header->length);
  reply_header->status = ntohl(reply_header->status);
  reply_header->this_block = ntohl(reply_header->this_block);
  reply_header->block_count = ntohl(reply_header->block_count);
}

// Helper function converting Reply Header to network byte order
void reply_to_net(ReplyHeader_t* reply_header) {
  reply_header->length = htonl(reply_header->length);
  reply_header->status = htonl(reply_header->status);
  reply_header->this_block = htonl(reply_header->this_block);
  reply_header->block_count = htonl(reply_header->block_count);
}