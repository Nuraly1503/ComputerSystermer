#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <assert.h>

#ifdef __APPLE__
#include "./endian.h"
#else
#include <endian.h>
#endif

#include "./networking.h"
#include "./sha256.h"

char server_ip[IP_LEN];
char server_port[PORT_LEN];
char my_ip[IP_LEN];
char my_port[PORT_LEN];

int c;

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
 * Combine a password and salt together and hash the result to form the 
 * 'signature'. The result should be written to the 'hash' variable. Note that 
 * as handed out, this function is never called. You will need to decide where 
 * it is sensible to do so.
 */
void get_signature(char* password, char* salt, hashdata_t* hash)
{
    // Your code here. This function has been added as a guide, but feel free 
    // to add more, or work in other parts of the code
    
    // create salted password string
    char salted[strlen(password) + strlen(salt)];
    strcpy(salted, strcat(password, salt));
    
    // hash salted to make signature
    get_data_sha(salted, *(hash), sizeof(salted), SHA256_HASH_SIZE);
}

/*
 * Register a new user with a server by sending the username and signature to 
 * the server
 */
void register_user(char* username, char* password, char* salt)
{
    // Your code here. This function has been added as a guide, but feel free 
    // to add more, or work in other parts of the code

    // struct for pass and salt
    PasswordAndSalt_t ps;
    strcpy(ps.password, password);
    //strcpy(ps.salt, salt); //<-- Trace stack fault. Why??

    // Request Header struct
    RequestHeader_t request_header;
    strcpy(request_header.username, username);
    request_header.length = 0; // length of requested data, the payload. Set to 0 when registrering user.

    // Hashing pass and salt
    get_signature(password, salt, &request_header.salted_and_hashed);

    // Print debugging for structs
    /* printf("Password: %s\n", ps.password);
    printf("Salt: %s\n", ps.salt);
    printf("Username: %s\n", request_header.username);
    printf("Salted and hashed: %u\n", request_header.salted_and_hashed);
    printf("Length: %u\n", request_header.length); */
    

    // Connection variables
    int clientfd;
    char* host = &server_ip[0];
    char* port = &server_port[0];
    char buf[MAXLINE];
    compsys_helper_state_t state;
     
    // Open clientfd connection
    if ((clientfd = compsys_helper_open_clientfd(host, port)) < 0) {
      printf("Socket connection failed with error %i\n", clientfd);
      exit(EXIT_FAILURE);
    };

    // Init read buffer
    compsys_helper_readinitb(&state, clientfd);
    if (state.compsys_helper_fd == 0) {
      printf("Read-buffer is not initialized\n");
      exit(EXIT_FAILURE);
    };

    // Write username, signature, and payload size (length) to buffer
    // ASK IF THERE'S A BETTER WAY TO DO THIS!?

    /* buf[0] = request_header.username[0];
    buf[USERNAME_LEN] = request_header.salted_and_hashed[0];
    buf[USERNAME_LEN + SHA256_HASH_SIZE] = request_header.length; */

    size_t index = 0;
    for (int i = 0; i < USERNAME_LEN; i++) {
      buf[index] = request_header.username[i];
      index++;
    }
    for (int i = 0; i < SHA256_HASH_SIZE; i++) {
      buf[index] = request_header.salted_and_hashed[i];
      index++;
    }
    buf[index] = request_header.length;

    // Write the buffer to the socket (send 'register user' protocol to server)
    compsys_helper_writen(clientfd, buf, MAXLINE);

    // Read response from server and send to stdout
    compsys_helper_readlineb(&state, state.compsys_helper_buf, MAXLINE); // <-- NOT WORKING !!!
    printf("Got response: ");
    fputs(state.compsys_helper_buf, stdout);
}

/*
 * Get a file from the server by sending the username and signature, along with
 * a file path. Note that this function should be able to deal with both small 
 * and large files. 
 */
void get_file(char* username, char* password, char* salt, char* to_get)
{
    // Your code here. This function has been added as a guide, but feel free 
    // to add more, or work in other parts of the code
}

int main(int argc, char **argv)
{
    // Users should call this script with a single argument describing what 
    // config to use
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(EXIT_FAILURE);
    } 

    // Read in configuration options. Should include a client_directory, 
    // client_ip, client_port, server_ip, and server_port
    char buffer[128];
    fprintf(stderr, "Got config path at: %s\n", argv[1]);
    FILE* fp = fopen(argv[1], "r");
    while (fgets(buffer, 128, fp)) {
        if (starts_with(buffer, CLIENT_IP)) {
            memcpy(my_ip, &buffer[strlen(CLIENT_IP)], 
                strcspn(buffer, "\r\n")-strlen(CLIENT_IP));
            if (!is_valid_ip(my_ip)) {
                fprintf(stderr, ">> Invalid client IP: %s\n", my_ip);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, CLIENT_PORT)) {
            memcpy(my_port, &buffer[strlen(CLIENT_PORT)], 
                strcspn(buffer, "\r\n")-strlen(CLIENT_PORT));
            if (!is_valid_port(my_port)) {
                fprintf(stderr, ">> Invalid client port: %s\n", my_port);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, SERVER_IP)) {
            memcpy(server_ip, &buffer[strlen(SERVER_IP)], 
                strcspn(buffer, "\r\n")-strlen(SERVER_IP));
            if (!is_valid_ip(server_ip)) {
                fprintf(stderr, ">> Invalid server IP: %s\n", server_ip);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, SERVER_PORT)) {
            memcpy(server_port, &buffer[strlen(SERVER_PORT)], 
                strcspn(buffer, "\r\n")-strlen(SERVER_PORT));
            if (!is_valid_port(server_port)) {
                fprintf(stderr, ">> Invalid server port: %s\n", server_port);
                exit(EXIT_FAILURE);
            }
        }        
    }
    fclose(fp);

    fprintf(stdout, "Client at: %s:%s\n", my_ip, my_port);
    fprintf(stdout, "Server at: %s:%s\n", server_ip, server_port);

    char username[USERNAME_LEN];
    char password[PASSWORD_LEN];
    char user_salt[SALT_LEN+1];
    
    fprintf(stdout, "Enter a username to proceed: ");
    scanf("%16s", username);
    while ((c = getchar()) != '\n' && c != EOF);
    // Clean up username string as otherwise some extra chars can sneak in.
    for (int i=strlen(username); i<USERNAME_LEN; i++)
    {
        username[i] = '\0';
    }
 
    fprintf(stdout, "Enter your password to proceed: ");
    scanf("%16s", password);
    while ((c = getchar()) != '\n' && c != EOF);
    // Clean up password string as otherwise some extra chars can sneak in.
    for (int i=strlen(password); i<PASSWORD_LEN; i++)
    {
        password[i] = '\0';
    }

    // Note that a random salt should be used, but you may find it easier to
    // repeatedly test the same user credentials by using the hard coded value
    // below instead, and commenting out this randomly generating section.
    for (int i=0; i<SALT_LEN; i++)
    {
        user_salt[i] = 'a' + (random() % 26);
    }
    user_salt[SALT_LEN] = '\0';
    strncpy(user_salt, 
       "0123456789012345678901234567890123456789012345678901234567890123\0", 
       SALT_LEN+1);

    fprintf(stdout, "Using salt: %s\n", user_salt);

    // The following function calls have been added as a structure to a 
    // potential solution demonstrating the core functionality. Feel free to 
    // add, remove or otherwise edit. Note that if you are creating a system 
    // for user-interaction the following lines will almost certainly need to 
    // be removed/altered.

    // Register the given user. As handed out, this line will run every time 
    // this client starts, and so should be removed if user interaction is 
    // added
    register_user(username, password, user_salt);

    // Retrieve the smaller file, that doesn't not require support for blocks. 
    // As handed out, this line will run every time this client starts, and so 
    // should be removed if user interaction is added
    get_file(username, password, user_salt, "tiny.txt");

    // Retrieve the larger file, that requires support for blocked messages. As
    // handed out, this line will run every time this client starts, and so 
    // should be removed if user interaction is added
    get_file(username, password, user_salt, "hamlet.txt");

    exit(EXIT_SUCCESS);
}
