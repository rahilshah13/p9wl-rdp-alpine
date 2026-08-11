#ifndef TLS_H
#define TLS_H
#include <stdint.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>

#define TLS_PORT 10001
struct tls_config {
    char *cert_file;
    char *cert_fingerprint;
    int insecure;
};

int tls_init(void);
void tls_cleanup(void);
int tls_connect(int fd, SSL **ssl_out, struct tls_config *cfg);
void tls_disconnect(SSL *ssl);
int tls_verify_pinned(SSL *ssl, struct tls_config *cfg);
int tls_read_full(SSL *ssl, uint8_t *buf, int n);
int tls_write_full(SSL *ssl, const uint8_t *buf, int len);
int tls_cert_file_fingerprint(const char *path, char *out, int outlen);
#endif /* TLS_H */