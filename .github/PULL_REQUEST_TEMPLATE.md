# Jira Ticket

Closes [ESPCAMFW-NNN](https://crestvital.atlassian.net/browse/ESPCAMFW-NNN)
*(Replace NNN with the actual ticket number)*

## Description

A clear description of what was changed and why. Reference the specific
problem or feature being addressed in the Jira ticket.

## Type of Change

Check only one:

- [ ] feat (new feature or new firmware component)
- [ ] fix (bug fix)
- [ ] docs (documentation changes)
- [ ] refactor (code change that neither fixes a bug nor adds a feature)
- [ ] test (adding or updating unit tests)
- [ ] chore (build config, CI, tooling, dependencies)

## Checklist

All items must be checked before requesting a review:

- [ ] `.\scripts\pre-pr.ps1` passes with no errors
- [ ] `pio run -e lilygo-t-display-s3` builds with **zero errors and zero warnings**
- [ ] All new functions have Doxygen `@brief` / `@param` / `@return` comments
- [ ] All `esp_err_t` return values are checked (no silent ignores)
- [ ] Large buffers (> 8 KB) allocated from PSRAM via `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)`
- [ ] No blocking calls inside ISR context; ISR functions marked `IRAM_ATTR`
- [ ] No `printf()` — only `ESP_LOG*` macros
- [ ] `CHANGELOG.md` updated under `[Unreleased]`
- [ ] No `git commit` or `git push` run by AI tools — commits created by owner only
