# AUREON — UFW Firewall Rules

## Critical Warning

> ⚠️ ALWAYS add firewall rules BEFORE enabling UFW.
> Enabling UFW without allowing port 22 will
> permanently lock you out of your server via SSH.
> This is the most common VPS deployment mistake.

---

## The Correct Order — Never Deviate

```bash
# Step 1 — Set defaults
ufw default deny incoming
ufw default allow outgoing

# Step 2 — Allow ports FIRST (before enabling)
ufw allow 22/tcp      # SSH — NEVER skip this
ufw allow 80/tcp      # HTTP
ufw allow 443/tcp     # HTTPS

# Step 3 — THEN enable
ufw enable

# Step 4 — Verify
ufw status verbose
```

---

## Expected Output After Setup

Status: active
To Action From

22/tcp              ALLOW IN  Anywhere
80/tcp              ALLOW IN  Anywhere
443/tcp             ALLOW IN  Anywhere
22/tcp (v6)         ALLOW IN  Anywhere (v6)
80/tcp              ALLOW IN  Anywhere (v6)
443/tcp             ALLOW IN  Anywhere (v6)

---

## Port Reference

| Port | Protocol | Purpose | Required |
|------|----------|---------|----------|
| 22 | TCP | SSH access | ✅ Always |
| 80 | TCP | HTTP traffic | ✅ Always |
| 443 | TCP | HTTPS traffic | ✅ Always |
| 8080 | TCP | AUREON engine | ❌ Internal only |

> Port 8080 is intentionally NOT opened.
> AUREON runs internally, accessed only via Nginx.
> Direct external access to 8080 is blocked by design.

---

## Emergency Recovery — Locked Out

If you locked yourself out by enabling UFW
without allowing SSH:

**Step 1 — Access VPS web console**
- DigitalOcean: Droplet → Access → Launch Droplet Console
- Vultr: Server → View Console
- Linode: Dashboard → Launch Console

**Step 2 — Disable and reset UFW**
```bash
ufw disable
ufw reset -y
```

**Step 3 — Rebuild rules correctly**
```bash
ufw default deny incoming
ufw default allow outgoing
ufw allow 22/tcp
ufw allow 80/tcp
ufw allow 443/tcp
ufw enable
ufw status verbose
```

**Step 4 — Verify SSH works again**
```bash
# From your local machine
ssh root@YOUR_VPS_IP
```

---

## Additional Rules (Optional)

```bash
# Allow specific IP for SSH only (more secure)
ufw allow from YOUR_HOME_IP to any port 22

# Rate limit SSH (prevents brute force)
ufw limit 22/tcp

# Allow a custom port
ufw allow 3000/tcp

# Remove a rule
ufw delete allow 3000/tcp

# Check rule numbers
ufw status numbered

# Delete rule by number
ufw delete 3
```

---

## Verify Security at Any Time

```bash
# Full status with all rules
ufw status verbose

# Check if specific port is open
ufw status verbose | grep 22

# Check if 8080 is NOT exposed (should return nothing)
ufw status verbose | grep 8080

# View UFW logs
tail -f /var/log/ufw.log
```

---

## UFW Quick Reference

```bash
ufw enable          # Enable firewall
ufw disable         # Disable firewall
ufw reset           # Reset all rules
ufw status verbose  # Show all rules
ufw allow 22/tcp    # Open a port
ufw deny 22/tcp     # Close a port
ufw delete allow 22 # Remove a rule
ufw limit 22/tcp    # Rate limit a port
```

---

*When in doubt — allow SSH first, always.*

