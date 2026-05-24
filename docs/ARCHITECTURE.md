# AUREON - Architecture

## Overview 

AUREON is a production-grade HTTP server written in raw C++20. No frameworks. No external libraries. Every byte of every response is handled by the C++ engine directly.

---

# Full Stack

┌─────────────────────────┐
│       Cloudflare        │
│  DNS + CDN + DDoS + SSL │
└───────────┬─────────────┘
│ HTTPS (443)
┌───────────▼─────────────┐
│          Nginx          │
│      Reverse Proxy      │
└───────────┬─────────────┘
│ HTTP (8080)
┌───────────▼─────────────┐
│    AUREON C++ Engine    │
│      Raw C++20 Binary   │
└───────────┬─────────────┘
│
┌───────────▼─────────────┐
│      DigitalOcean VPS   │
│    Ubuntu 24.04 LTS     │
│    1 vCPU / 1GB RAM     │
└─────────────────────────┘

---

## Component Breakdown

### C++ engine (main.cpp)
- Raw POSIX sockets on Linux
- Raw WinSock2 on Windows
- Single threaded blocking connection model
- Manual HTTP request parsing via string matching
- Static file serving from frontend/ directory
- JSON API responses built as raw strings

### Nginx
- Reverse proxy from port 80/443 > 8080
- SSL termination in combination with Cloudflare
- Shields the C++ binary from direct exposure
- Easy to extend with rate limiting and caching

### Cloudflare
- Authoritative DNS for aureonops.dev
- Orange Cloud CDN - static assets cached globally
- Free DDoS protection - automatic
- SSL/TLS Full mode - encrypted end to end
- Zero cost enterprise grade protection

### systemd
- Manages AUREON as a persistent system service
- Auto starts on VPS boot
- Auto restarts on crash or failure
- Runs independently of any terminal session

### UFW Firewall
- Default deny all incoming
- Whitelist only: 22 (SSH), 80 (HTTP), 443 (HTTPS)
- Port 8080 internal only - not exposed directly
- Hardens the VPS against unauthorized access

---

# Request Flow

1. User visit aureonops.dev
2. DNS resolves via Cloudflare
3. Cloudflare CDN checks cache
4. If not cached > forwards to VPS IP
5. Nginx receives request on port 80/443
6. Nginx proxies to localhost:8080
7. AUREON C++ engine receives raw HTTP
8. Engine parses request string
9. Matches ruote > builds response
10. Sends raw HTTP response back
11. Nginx forwards to Cloudflare
12. Cloudflare delivers to user

---

---

## Cross Platform Design

AUREON runs on both Windows and Linux from
the same codebase using preprocessor guards:

```cpp
#ifdef _WIN32
    // Windows: WinSock2
    #include <winsock2.h>
    using SocketType = SOCKET;
#else
    // Linux: POSIX sockets
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    using SocketType = int;
#endif
```

**Development:** Windows + CLion
**Production:** Ubuntu 24.04 LTS + systemd

---

## File Structure

aureon-cpp-backend/
├── frontend/               # Static frontend assets
│   ├── index.html          # Landing page
│   ├── style.css           # Premium dark UI
│   └── script.js           # API interaction layer
├── docs/                   # Documentation
│   ├── ARCHITECTURE.md     # This file
│   ├── DEPLOYMENT.md       # Production deployment guide
│   ├── SECURITY.md         # Security hardening guide
│   └── ROADMAP.md          # Future development plan
├── deployment/             # Production config files
│   ├── aureon.service      # systemd service unit
│   ├── nginx-aureon.conf   # Nginx reverse proxy config
│   └── ufw-rules.md        # Firewall setup guide
├── main.cpp                # C++ backend engine
├── CMakeLists.txt          # Cross platform build config
├── README.md               # Project overview
└── LICENSE                 # MIT License

---

## Technology Choices

| Technology | Why |
|------------|-----|
| C++20 | Maximum performance, zero runtime overhead |
| Raw sockets | Full transparency, no hidden abstractions |
| Nginx | Industry standard, battle tested proxy |
| Cloudflare | Free CDN + DDoS + SSL, zero configuration |
| systemd | Native Linux service management |
| DigitalOcean | Reliable VPS, predictable pricing |
| Ubuntu 24.04 | LTS stability, wide documentation |

---

*AUREON — Built from first principles.*