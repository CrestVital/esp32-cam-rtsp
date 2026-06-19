# Architecture Decision Records

This directory contains Architecture Decision Records (ADRs) for esp32-cam-rtsp.

An ADR documents a significant architectural decision: what was decided, why,
and what the consequences are. Once accepted, ADRs are immutable — if a decision
is reversed, a new ADR is written superseding the old one.

## Index

| ADR | Title | Status |
|-----|-------|--------|
| [ADR-001](ADR-001-mjpeg-vs-h264.md) | MJPEG vs H.264 for RTP stream | Accepted |

## Template

```markdown
# ADR-XXX: Title

**Date:** YYYY-MM-DD
**Status:** Proposed | Accepted | Deprecated | Superseded by ADR-YYY
**Ticket:** ESPCAMFW-XX

## Context
Why was this decision needed?

## Decision
What was decided?

## Rationale
Why this option over alternatives?

## Consequences
What are the trade-offs and follow-up actions?
```
