/*
 * p9_tls.h - TLS support for secure connections on Alpine Linux
 *
 * Provides TLS transport with certificate pinning for secure connections,
 * utilizing OpenSSL for cryptographic operations.
 */

#ifndef P9_TLS_H
#define P9_TLS_H

#include <stdint.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>

/* ============== Constants ============== */

/* Default port for secure connections */
#define P9_TLS_PORT 10001

/* ============== Configuration ============== */

/*
 * TLS connection configuration.
 *
 * Exactly one of cert_file, cert_fingerprint, or insecure should be set.
 * If none are set, the connection will fail certificate verification.
 */
struct tls_config {
    /*
     * Path to pinned certificate in PEM format.
     * Server's certificate must exactly match this file.
     */
    char *cert_file;

    /*
     * SHA256 fingerprint of expected certificate as hex string.
     * 64 characters (32 bytes as hex). Colons/spaces are ignored.
     */
    char *cert_fingerprint;

    /*
     * Skip certificate verification (DANGEROUS).
     * Connection is encrypted but server identity is NOT verified.
     */
    int insecure;
};

/* ============== Initialization ============== */

/*
 * Initialize OpenSSL library and create global SSL context.
 *
 * Must be called once before any other TLS functions.
 * Returns 0 on success, -1 on failure.
 */
int tls_init(void);

/*
 * Clean up OpenSSL resources.
 *
 * Frees global SSL context. Call at program shutdown.
 */
void tls_cleanup(void);

/* ============== Connection Management ============== */

/*
 * Establish TLS connection over existing socket.
 *
 * Performs TLS handshake and verifies server certificate against
 * pinned certificate or fingerprint (unless insecure mode).
 *
 * fd:      connected TCP socket file descriptor
 * ssl_out: output - SSL object on success
 * cfg:     TLS configuration (pinning method)
 *
 * Returns 0 on success, -1 on failure.
 */
int tls_connect(int fd, SSL **ssl_out, struct tls_config *cfg);

/*
 * Close TLS connection and free SSL object.
 *
 * ssl: SSL object from tls_connect(), or NULL (no-op)
 */
void tls_disconnect(SSL *ssl);

/* ============== Certificate Verification ============== */

/*
 * Verify server certificate against pinned certificate or fingerprint.
 *
 * ssl: established SSL connection
 * cfg: configuration with cert_file or cert_fingerprint
 *
 * Returns 0 if certificate matches, -1 on mismatch or error.
 */
int tls_verify_pinned(SSL *ssl, struct tls_config *cfg);

/* ============== I/O Operations ============== */

/*
 * Read exactly n bytes through TLS connection.
 *
 * Returns n on success, -1 on error or connection close.
 */
int tls_read_full(SSL *ssl, uint8_t *buf, int n);

/*
 * Write exactly len bytes through TLS connection.
 *
 * Returns len on success, -1 on error.
 */
int tls_write_full(SSL *ssl, const uint8_t *buf, int len);

/* ============== Utility Functions ============== */

/*
 * Compute SHA256 fingerprint of a PEM certificate file.
 *
 * path:   path to PEM certificate file
 * out:    output buffer for hex string (must be at least 65 bytes)
 * outlen: size of output buffer
 *
 * Returns 0 on success, -1 on error.
 */
int tls_cert_file_fingerprint(const char *path, char *out, int outlen);

#endif /* P9_TLS_H */