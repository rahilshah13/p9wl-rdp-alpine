1. `docker build -t p9wl-rdp .`
2. `docker run -itd --security-opt seccomp=unconfined -p 3389:3389 p9wl-rdp`
3. docker exec: `firefox --no-sandbox`
4. TODO: rdp client connection issues