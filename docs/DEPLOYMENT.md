# AUREON — Deployment Guide

## Overview

This guide covers the complete production deployment
of AUREON on a Linux VPS from zero to live.
Estimated time: 30-60 minutes.

---

## Prerequisites

Before starting you will need:

- A Linux VPS (DigitalOcean, Vultr, Linode, etc)
    - Minimum: 1 vCPU, 1GB RAM, 25GB SSD
    - Recommended OS: Ubuntu 24.04 LTS
- A domain name (Namecheap, Cloudflare, etc)
- A Cloudflare account (free tier is enough)
- SSH key pair generated on your local machine
- Git installed locally
- CLion or any C++ build environment

---

## Step 0 — Generate SSH Key (Local Machine)

Never deploy without an SSH key.
Password authentication is a security risk.

```bash
# Generate key pair
ssh-keygen -t ed25519 -C "your-server-name"

# Press Enter three times (accept defaults, no passphrase)

# Copy your public key
cat ~/.ssh/id_ed25519.pub
```

Save that public key — you'll paste it into your
VPS provider during droplet creation.

> ⚠️ CRITICAL: Add your SSH key to the VPS
> BEFORE creating it. Not after.

---

## Step 1 — Create Your VPS

On DigitalOcean (or your provider of choice):

1. Click **Create → Droplets**
2. Select **Ubuntu 24.04 LTS**
3. Choose plan: **$6/month (1vCPU, 1GB RAM)**
4. Select your region (closest to your audience)
5. Under **Authentication → SSH Keys** → paste your public key
6. Click **Create Droplet**
7. Wait 30 seconds → copy the **Public IPv4 address**

---

## Step 2 — SSH Into Your VPS

```bash
ssh root@YOUR_VPS_IP
```

First connection will ask: 

Are you sure you want to continue connecting? > Yes

You are now inside your production server.

---

## Step 3 — Configure UFW Firewall

> ⚠️ CRITICAL ORDER: Always allow SSH BEFORE
> enabling UFW. Enabling first = locked out forever.

```bash
# Allow ports FIRST
ufw allow 22/tcp      # SSH — never skip this
ufw allow 80/tcp      # HTTP
ufw allow 443/tcp     # HTTPS

# THEN enable
ufw enable

# Verify
ufw status verbose
```

Expected output:

To Action From

22/tcp              ALLOW IN  Anywhere
80/tcp              ALLOW IN  Anywhere
443/tcp             ALLOW IN  Anywhere

---

## Step 4 — Install Build Tools

```bash
# Update system
apt update && apt upgrade -y

# Install required tools
apt install -y git g++ cmake make

# Verify installations
git --version
g++ --version
cmake --version
```

---

## Step 5 — Clone and Build AUREON

```bash
# Clone the repository
git clone https://github.com/APO72A/aureon-cpp-backend.git
cd aureon-cpp-backend

# Fix CMake version requirement for Linux
sed -i 's/cmake_minimum_required(VERSION 4.2)/cmake_minimum_required(VERSION 3.28)/' CMakeLists.txt

# Fix Windows-only library link
sed -i 's/target_link_libraries(cpp_backend_test ws2_32)/if(WIN32)\n    target_link_libraries(cpp_backend_test ws2_32)\nendif()/' CMakeLists.txt

# Build the binary
cmake -B build && cmake --build build

# Verify binary exists
ls build/
```

You should see `cpp_backend_test` in the build folder.

---

## Step 6 — Test the Server

```bash
# Run the server manually first
./build/cpp_backend_test
```

In a separate terminal:
```bash
curl http://localhost:8080/api/status
```

Expected response:
```json
{"status": "online", "engine": "C++", "message": "AUREON is running!"}
```

If that works — stop the server with `Ctrl+C`
and move to the next step.

---

## Step 7 — Configure systemd Service

Copy the service file:
```bash
cp deployment/aureon.service /etc/systemd/system/aureon.service
```

Or create it manually:
```bash
nano /etc/systemd/system/aureon.service
```

Paste:
```ini
[Unit]
Description=AUREON C++ Backend
After=network.target

[Service]
WorkingDirectory=/root/aureon-cpp-backend
ExecStart=/root/aureon-cpp-backend/build/cpp_backend_test
Restart=always
User=root

[Install]
WantedBy=multi-user.target
```

Enable and start:
```bash
systemctl daemon-reload
systemctl enable aureon
systemctl start aureon

# Verify it's running
systemctl status aureon
```

Expected output: 

● aureon.service - AUREON C++ Backend
Active: active (running)

---

## Step 8 — Install and Configure Nginx

```bash
# Install Nginx
apt install -y nginx

# Create site config
nano /etc/nginx/sites-available/aureon
```

Paste:
```nginx
server {
    listen 80;
    server_name yourdomain.dev www.yourdomain.dev;

    location / {
        proxy_pass http://localhost:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
    }
}
```

Enable the site:
```bash
# Enable site
ln -s /etc/nginx/sites-available/aureon /etc/nginx/sites-enabled/

# Remove default site
rm /etc/nginx/sites-enabled/default

# Test config
nginx -t

# Restart Nginx
systemctl restart nginx
systemctl enable nginx
```

---

## Step 9 — Configure Cloudflare DNS

1. Log into **cloudflare.com**
2. Add your domain if not already added
3. Go to **DNS → Records**
4. Add two A records:

| Type | Name | Content | Proxy |
|------|------|---------|-------|
| A | @ | YOUR_VPS_IP | ✅ Proxied |
| A | www | YOUR_VPS_IP | ✅ Proxied |

5. Go to **SSL/TLS → Overview**
6. Set encryption mode to **Full**
7. Wait 2-5 minutes for DNS propagation

---

## Step 10 — Verify Everything

```bash
# Check AUREON service
systemctl status aureon

# Check Nginx
systemctl status nginx

# Check firewall
ufw status verbose

# Test API live
curl https://yourdomain.dev/api/status
```

Visit your domain in a browser — AUREON should
be live with HTTPS! 🔥

---

## Updating the Deployment

When you push new code to GitHub:

```bash
# On your VPS
cd ~/aureon-cpp-backend
git pull origin main
cmake --build build
systemctl restart aureon
```

---

## Troubleshooting

**SSH locked out after UFW enable**
→ Use VPS provider web console
→ Run: `ufw disable && ufw reset -y`
→ Start again from Step 3 in correct order

**Build fails with winsock2.h error**
→ The CMakeLists.txt still links ws2_32 unconditionally
→ Run the sed fix in Step 5

**Site not loading after Nginx setup**
→ Run `nginx -t` to check config syntax
→ Check `systemctl status nginx` for errors
→ Verify Cloudflare SSL is set to Full not Flexible

**Service not starting**
→ Run `journalctl -u aureon -n 50`
→ Check WorkingDirectory path is correct
→ Verify binary exists at the ExecStart path

---

## What You End Up With

✅ C++ binary running as system service
✅ Auto restart on failure
✅ Nginx reverse proxy
✅ Cloudflare CDN + DDoS protection
✅ HTTPS secured domain
✅ UFW hardened firewall
✅ Production grade infrastructure

---

*AUREON — Deploy once. Run forever.*