# Security Policy

## Supported Versions

This project is under active development. Only the latest commit on the
`main` branch receives security fixes.

| Version | Supported |
|---------|-----------|
| latest (main) | Yes |
| older tags | No |

## Reporting a Vulnerability

**Please do not report security vulnerabilities through public GitHub issues.**

If you discover a security vulnerability in this firmware, please report it
privately so it can be addressed before public disclosure.

### How to report

Use GitHub's **private vulnerability reporting** feature:

1. Go to the [Security tab](https://github.com/CrestVital/esp32-cam-rtsp/security)
2. Click **"Report a vulnerability"**
3. Fill in the details: description, affected component, steps to reproduce,
   and — if available — a suggested fix or patch

We aim to acknowledge reports within **5 business days** and to provide a fix
or mitigation within **30 days** for confirmed vulnerabilities.

## Scope

This repository covers the ESP32-S3 firmware only. The broader CrestVital
platform (edge pipeline, inference, API) is out of scope for this policy.

### In scope

- Buffer overflows or memory corruption in firmware components
- Hardcoded credentials or secrets accidentally committed
- Insecure OTA update handling (unsigned firmware acceptance, MITM)
- RTSP stream authentication bypass
- NVS storage of sensitive data in plaintext

### Out of scope

- Issues requiring physical access to a flashed device (JTAG, serial console)
- Theoretical vulnerabilities with no practical exploit path on constrained hardware
- Denial-of-service via WiFi disruption

## Disclosure Policy

We follow **coordinated disclosure**: we ask that you give us a reasonable
time to fix the issue before publishing details publicly. We will credit
reporters in the release notes unless they prefer to remain anonymous.