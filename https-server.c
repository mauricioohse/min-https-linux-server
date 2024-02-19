#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <string.h>
#include <openssl/err.h>

void main()
{
  // creating socket ip4 tcp
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in address = {
      AF_INET,
      htons(8080),
      0 // 0 ip address -> only local to my machine
  };

  bind(sockfd, &address, sizeof(address));
  listen(sockfd, 5);


  // now SSL/https setup
  SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
  if (!ctx)
  {
    // Handle SSL_CTX_new error
    ERR_print_errors_fp(stderr);
    exit(1);
  }
  while(1){
  int clientfd = accept(sockfd, NULL, NULL);


  SSL *ssl = SSL_new(ctx);
  if (!ssl)
  {
    // Handle SSL_new error
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  SSL_set_fd(ssl, clientfd);

  int ret = SSL_use_certificate_chain_file(ssl, "fullChain.pem");
  if (ret != 1)
  {
    // Handle certificate file error
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  ret = SSL_use_PrivateKey_file(ssl, "theKey.pem", SSL_FILETYPE_PEM);
  if (ret != 1)
  {
    // Handle private key file error
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  ret = SSL_accept(ssl);
  if (ret <= 0)
  {
    // Handle SSL_accept error
    int err = SSL_get_error(ssl, ret);
    printf("SSL_accept failed with error: %d\n", err);
    ERR_print_errors_fp(stderr);
    //exit(1);
  }

  // Now proceed with SSL_read() and other operations
  char buffer[1024] = {0};

  if (SSL_read(ssl, buffer, 1023) == 0)
  {
    printf("ssl read failed\n");
  }

  // GET /file ...
  char *file_request = buffer + 5; // skipping 'GET /'
  char response[1024] = {0};

  char *metadata = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";
  memcpy(response, metadata, strlen(metadata));

  if (strncmp(file_request, "index.html ", 11) == 0)
  {
    printf("index.html requested\n");
    FILE *f = fopen("index.html", "r");
    fread(response + strlen(metadata), 1024 - strlen(metadata) - 1, 1, f);

    fclose(f);
  }
  else
  {
    char *errormsg = "No page found";
    memcpy(response + strlen(metadata), errormsg, strlen(errormsg));
  }

  int success = SSL_write(ssl, response, 1024);

  if (success <= 0)
  {
    int error = SSL_get_error(ssl, success);
    switch (error)
    {
    case SSL_ERROR_ZERO_RETURN:
      fprintf(stderr, "SSL connection closed gracefully.\n");
      break;
    case SSL_ERROR_SYSCALL:
      perror("SSL_write");
      break;
    case SSL_ERROR_SSL:
      fprintf(stderr, "SSL error occurred");
      break;
    default:
      fprintf(stderr, "Unknown SSL write error: %d\n", error);
      break;
    }
  }
  else
  {
    printf("Written response: %s\n", response);
    printf("Written bytes: %d\n", success);
  }

  SSL_shutdown(ssl);
  close(clientfd);
  }
}