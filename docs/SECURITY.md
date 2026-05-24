# AUREON — Security Guide

## Overview

AUREON is hardened for production from day one.
This document covers every security layer,
why it exists, and how to verify it is working.

---

## Security Stack

┌─────────────────────────────────┐
│         Cloudflare              │
│  DDoS protection + SSL/TLS      │
│  Hides origin VPS IP address    │
└───────────────┬─────────────────┘
│
┌───────────────▼─────────────────┐
│            UFW                  │
│  Firewall — deny all by default │
│  Only ports 22, 80, 443 open    │
└───────────────┬─────────────────┘
│
┌───────────────▼─────────────────┐
│           Nginx                 │
│  Reverse proxy — C++ binary     │
│  never directly exposed         │
└───────────────┬─────────────────┘
│
┌───────────────▼─────────────────┐
│       AUREON C++ Engine         │
│  Internal only — port 8080      │
│  Not accessible from outside    │
└─────────────────────────────────┘

---

## Layer 1 — SSH Hardening

### Use SSH Keys Only
Password authentication is disabled by default
on DigitalOcean droplets with SSH keys added.
Never enable password authentication.

```bash
# Verify SSH key authentication is working
ssh root@YOUR_VPS_IP

# Should connect without password prompt
```

### SSH Key Best Practices

✅ Use ed25519 keys (modern, secure)
✅ Never share your private key
✅ Never commit private keys to GitHub
✅ Store keys in ~/.ssh/ only
✅ Use a passphrase for extra security

### Verify SSH Config
```bash
cat /etc/ssh/sshd_config | grep PasswordAuthentication
# Should show: PasswordAuthentication no
```

---

## Layer 2 — UFW Firewall

### The Golden Rule

ALWAYS allow SSH before enabling UFW.
Enabling UFW without allowing port 22 =
permanent lockout from your server.

### Correct Setup Order
```bash
# Step 1 — Allow ports FIRST
ufw allow 22/tcp
ufw allow 80/tcp
ufw allow 443/tcp

# Step 2 — THEN enable
ufw enable

# Step 3 — Verify
ufw status verbose
```

### What Each Rule Does

| Port | Protocol | Purpose |
|------|----------|---------|
| 22 | TCP | SSH access |
| 80 | TCP | HTTP (redirects to HTTPS via Cloudflare) |
| 443 | TCP | HTTPS |

### Port 8080 — Internal Only
The AUREON C++ engine runs on port 8080.
This port is intentionally NOT opened in UFW.
It is only accessible internally via Nginx proxy.
Direct external access to port 8080 is blocked.

```bash
# Verify 8080 is NOT exposed
ufw status verbose | grep 8080
# Should return nothing
```

---

## Layer 3 — Cloudflare Protection

### Orange Cloud — Always On
Cloudflare's orange cloud proxy mode:
- Hides your real VPS IP address
- Absorbs DDoS attacks before they reach your VPS
- Caches static assets globally
- Provides free SSL/TLS certificates

### SSL/TLS Mode — Full

Flexible  → Cloudflare to origin is HTTP (insecure)
Full      → Cloudflare to origin is HTTPS ✅
Full Strict → Requires valid SSL cert on origin

AUREON uses **Full** mode — encrypted all the way
from the user to the origin VPS.

### Verify Cloudflare Settings
1. Login to cloudflare.com
2. Select your domain
3. SSL/TLS → Overview → Confirm **Full** is selected
4. DNS → Confirm both A records show orange cloud ✅

---

## Layer 4 — Nginx Security

### Reverse Proxy Benefits
Nginx sits between the internet and your C++ binary:
- C++ engine never directly exposed
- Nginx handles malformed requests
- Easy to add rate limiting
- Easy to add request filtering

### Hide Nginx Version
```bash
nano /etc/nginx/nginx.conf
```

Add inside `http {}` block:
```nginx
server_tokens off;
```

This prevents Nginx from advertising its version
in response headers — reduces attack surface.

### Verify Headers
```bash
curl -I https://yourdomain.dev
# Server header should not show Nginx version
```

---

## Layer 5 — AUREON Backend Security

### CORS Headers
All API responses include:

Access-Control-Allow-Origin: *

For production with a known frontend origin,
restrict this to your domain:
```cpp
"Access-Control-Allow-Origin: https://yourdomain.dev\r\n"
```

### Input Validation
The `/api/greet` endpoint accepts a `name` parameter.
Current implementation extracts via string parsing.
Always validate and sanitise user input before
using it in responses.

### No Sensitive Data in Responses
API responses contain only:
- System status information
- Personalised greeting messages
- No server internals exposed
- No file paths exposed
- No error stack traces exposed

---

## Security Checklist

Run through this checklist after every deployment:

SSH
☐ SSH key authentication working
☐ Password authentication disabled
☐ Private key never committed to git
Firewall
☐ UFW enabled and active
☐ Port 22 open (SSH)
☐ Port 80 open (HTTP)
☐ Port 443 open (HTTPS)
☐ Port 8080 NOT exposed externally
☐ All other ports blocked
Cloudflare
☐ Both A records proxied (orange cloud)
☐ SSL/TLS mode set to Full
☐ Domain resolving correctly
Nginx
☐ Reverse proxy configured correctly
☐ Default site removed
☐ Config test passing (nginx -t)
☐ Service enabled and running
AUREON Backend
☐ systemd service active and enabled
☐ Auto restart configured
☐ Running on internal port 8080 only
☐ CORS headers present in responses

---

## Incident Response

### Locked out of SSH
```bash
# Use VPS provider web console
# DigitalOcean: Droplet → Access → Launch Console
ufw disable
ufw reset -y
ufw allow 22/tcp
ufw allow 80/tcp
ufw allow 443/tcp
ufw enable
```

### Service Down
```bash
# Check service status
systemctl status aureon

# View recent logs
journalctl -u aureon -n 50

# Restart service
systemctl restart aureon
```

### Nginx Down
```bash
# Test config
nginx -t

# Check logs
tail -f /var/log/nginx/error.log

# Restart
systemctl restart nginx
```

---

*AUREON — Secure by default. Hardened by design.*

