# --- Build Stage ---
FROM alpine:latest AS builder

RUN apk add --no-cache \
    build-base \
    pkgconf \
    wlroots-dev \
    wayland-dev \
    wayland-protocols \
    libxkbcommon-dev \
    pixman-dev \
    freerdp-dev \
    winpr-dev \
    openssl-dev \
    fftw-dev \
    lz4-dev \
    zlib-dev

WORKDIR /app
COPY . .

RUN make

# --- Runtime Stage ---
FROM alpine:latest

RUN apk add --no-cache \
    wlroots \
    wayland \
    libxkbcommon \
    pixman \
    freerdp \
    winpr \
    openssl \
    fftw \
    lz4 \
    zlib

WORKDIR /app
COPY --from=builder /app/p9wl /app/p9wl

EXPOSE 3389
ENTRYPOINT ["/app/p9wl"]