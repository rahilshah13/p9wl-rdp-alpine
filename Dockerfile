# --- Build Stage ---
FROM alpine:latest AS builder
RUN apk add --no-cache \
    build-base \
    pkgconf \
    wlroots0.19-dev \
    wayland-dev \
    wayland-protocols \
    libxkbcommon-dev \
    pixman-dev \
    freerdp-dev \
    openssl-dev \
    fftw-dev \
    lz4-dev \
    zlib-dev \
    xkeyboard-config \
    linux-headers \
    openssl

WORKDIR /app
COPY . .
RUN openssl req -x509 -newkey rsa:2048 -nodes \
    -keyout /app/server.key \
    -out /app/server.crt \
    -days 365 -subj "/CN=p9wl"

RUN make

# --- Runtime Stage ---
FROM alpine:latest
RUN apk add --no-cache \
    wlroots0.19 \
    wayland \
    libxkbcommon \
    pixman \
    freerdp \
    openssl \
    fftw \
    lz4 \
    zlib \
    xkeyboard-config \
    firefox \
    ttf-dejavu \
    ttf-liberation \
    fontconfig

WORKDIR /app
COPY --from=builder /app/p9wl-rdp-alpine /app/p9wl-rdp-alpine
COPY --from=builder /app/server.crt /app/server.crt
COPY --from=builder /app/server.key /app/server.key

# Secure TLS key and certificate file permissions for FreeRDP requirements
RUN chmod 600 /app/server.key && chmod 644 /app/server.crt

# Enable OpenSSL 3.x legacy provider for WinPR / FreeRDP MD4 support
RUN printf 'openssl_conf = openssl_init\n\n\
[openssl_init]\n\
providers = provider_sect\n\n\
[provider_sect]\n\
default = default_sect\n\
legacy = legacy_sect\n\n\
[default_sect]\n\
activate = 1\n\n\
[legacy_sect]\n\
activate = 1\n' > /app/openssl.cnf

ENV OPENSSL_CONF=/app/openssl.cnf
# Optional: Set WinPR/FreeRDP logging to trace if handshake diagnostics are needed
ENV WLOG_LEVEL=TRACE
RUN mkdir -p /tmp/xdg
# RUN mkdir -p /tmp/xdg /var/lib/dbus && dbus-uuidgen > /var/lib/dbus/machine-id
ENV XDG_RUNTIME_DIR=/tmp/xdg \
    WAYLAND_DISPLAY=wayland-0 \
    MOZ_ENABLE_WAYLAND=1 \
    MOZ_DISABLE_CONTENT_SANDBOX=1

EXPOSE 3389
ENTRYPOINT ["/app/p9wl-rdp-alpine", "-d"]
