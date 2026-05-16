\# AUREON



> High-performance backend infrastructure. Built in C++. Production grade.



\[!\[Status](https://img.shields.io/badge/status-live-00ff88)](https://aureonops.dev)

\[!\[Engine](https://img.shields.io/badge/engine-C%2B%2B20-00d4ff)](https://aureonops.dev)

\[!\[License](https://img.shields.io/badge/license-MIT-white)](LICENSE)



\*\*Live deployment:\*\* \[aureonops.dev](https://aureonops.dev)



\---



\## What is AUREON



AUREON is a production grade backend system written in raw C++20. No frameworks. No bloat. Pure performance.



Built from first principles for engineers who refuse to compromise on speed.



\---



\## Why AUREON



| Metric | Most Backends | AUREON |

|--------|---------------|--------|

| Runtime | Node.js / Python interpreter | Native C++ binary |

| Memory footprint | 80-200 MB | < 10 MB |

| Cold start | 200-800 ms | < 10 ms |

| Dependencies | Hundreds | Zero |

| Framework | Express / FastAPI / etc | None |



\---



\## Stack

Engine        →  C++20

OS            →  Ubuntu 24.04 LTS

Proxy         →  Nginx (reverse proxy)

DNS / CDN     →  Cloudflare

Security      →  SSL / TLS + UFW

Persistence   →  systemd service

Host          →  DigitalOcean VPS

---



\## Quick Start



\### Linux



```bash

git clone https://github.com/APO72A/aureon-cpp-backend.git

cd aureon-cpp-backend

cmake -B build \&\& cmake --build build

./build/cpp\_backend\_test

```



\### Windows



Open in CLion → Build → Run.



Server starts on `http://localhost:8080`.



\---



\## API Endpoints



| Endpoint | Method | Response |

|----------|--------|----------|

| `/` | GET | Frontend (HTML) |

| `/api/status` | GET | System telemetry |

| `/api/message` | GET | Basic backend health |

| `/api/greet?name=X` | GET | Personalised response |



\---



\## Production Deployment



Battle tested deployment stack:



1\. \*\*Ubuntu VPS\*\* — minimum 1 vCPU, 1 GB RAM

2\. \*\*systemd\*\* — persistent service, auto restart on failure

3\. \*\*Nginx\*\* — reverse proxy from port 80/443 to 8080

4\. \*\*Cloudflare\*\* — DNS, CDN, SSL termination

5\. \*\*UFW\*\* — firewall hardened, only essential ports open



Full deployment guide → \[coming soon]



\---



\## Architecture

┌─────────────┐

│  Cloudflare │  ← DNS + CDN + SSL

└──────┬──────┘

│

┌──────▼──────┐

│    Nginx    │  ← Reverse proxy

└──────┬──────┘

│

┌──────▼──────┐

│   AUREON    │  ← C++20 engine

│   Backend   │

└─────────────┘

---



\## Roadmap



\- \[x] Cross platform build (Windows + Linux)

\- \[x] Production deployment on VPS

\- \[x] HTTPS via Cloudflare

\- \[x] systemd persistent service

\- \[x] Premium frontend dashboard

\- \[ ] Multi threaded request handling

\- \[ ] Connection pooling

\- \[ ] Built in metrics + monitoring

\- \[ ] Docker container support

\- \[ ] One command deployment script



\---



\## Why This Exists



Modern backends prioritise developer convenience over machine efficiency. AUREON prioritises both.



Built for engineers who care about:



\- Sub millisecond response times

\- Zero framework overhead

\- Memory discipline

\- Production grade reliability

\- Infrastructure they fully understand



\---



\## Contact



\*\*Founder:\*\* Jashan

\*\*Project:\*\* \[aureonops.dev](https://aureonops.dev)

\*\*Repository:\*\* \[github.com/APO72A/aureon-cpp-backend](https://github.com/APO72A/aureon-cpp-backend)



\---



<sub>Built with C++ · Deployed on DigitalOcean · Secured by Cloudflare</sub>

