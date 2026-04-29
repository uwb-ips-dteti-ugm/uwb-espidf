# DW3000 API Development Status

This file tracks the planned application-facing API layer above `dw3000_hal`.
The API should make common DW3000 use ergonomic without hiding the HAL from
advanced users.

## Layer Boundary

| Layer         | Responsibility                                                                                                        | Non-goals                                                           |
| ------------- | --------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------- |
| `dw3000_hal`  | Register-file ownership, chip feature workflows, precise state/control primitives.                                    | Application policy, protocol choreography, board-specific defaults. |
| `dw3000_api`  | Common device lifecycle, TX/RX flows, IRQ/status handling, timestamp helpers, and safe defaults built from HAL calls. | Full RTLS engines, application frame formats, network management.   |
| Examples/apps | Board wiring, task structure, ranging demos, and product-specific behavior.                                           | New chip register abstractions.                                     |

## Design Principles

- Keep API calls composable: a user should be able to drop to HAL at any point.
- Do not allocate or free `dw3000_device_t`; API functions operate on caller-owned device contexts.
- Preserve explicit error returns; do not swallow `TIMEOUT`, `BUSY`, `HPDWARN`, or timestamp-quality failures.
- Prefer small structs for options/results instead of long argument lists.
- Keep blocking helpers bounded by caller-provided timeouts.
- Do not bake in a specific ranging protocol in the first API pass.
- Keep ISR-facing APIs minimal; do heavy work from task context.

## Planned Public Surface

| Area                       | Status      | Planned API shape                                                                     | Notes                                                                                                   |
| -------------------------- | ----------- | ------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------- |
| API header/source scaffold | Initial | `include/dw3000_api.h`, `src/dw3000_api.c`, optional `dw3000_types/api.h`.            | Initial single-header facade exists; split only if the surface grows.                                   |
| Device lifecycle           | Initial | `dw3000_api_init`, `dw3000_api_deinit`, `dw3000_api_reset_recover`.                   | Caller owns `dw3000_device_t` storage; API initializes and recovers an existing context only.           |
| Configuration presets      | Not started | Default PHY/MAC/STS profiles for common channel/preamble/data-rate choices.           | Avoid board policy; board pin/SPI setup remains outside core API.                                       |
| Basic TX                   | Initial | Immediate frame send, delayed frame send, wait-for-TX-complete helper.                | Handles TX buffer, frame control, start command, status clear, timestamp read, and radio cleanup.       |
| Basic RX                   | Initial | Start RX, receive-one-frame with timeout, read payload/metadata/timestamp.            | Handles RX start, optional frame wait timeout, status/error decode, payload read, timestamp read, and cleanup. |
| IRQ/event dispatch         | Initial | Read/clear status and classify events into API event structs.                         | Supports polling and task-level IRQ handling; no ISR-heavy logic.                                       |
| Timestamp helpers          | Partial | Convert/read TX/RX timestamps, delayed-time arithmetic, timeout/lateness helpers.     | TX/RX result timestamp reads exist; delayed-time arithmetic is still missing.                           |
| STS validation             | Not started | Helpers to validate STS quality before accepting secure timestamps.                   | Wrap ACC_QUAL, CIA TOAST, and STS timestamp reliability checks.                                         |
| MAC convenience            | Not started | Frame filtering, auto-ACK, wait-for-response setup helpers.                           | Keep 802.15.4 policy configurable; do not hardcode frame formats.                                       |
| Power/sleep convenience    | Not started | Sleep entry/wake completion wrapper with AON save/restore options.                    | Should expose what is retained/restored and whether PLL/RX is restored.                                 |
| Calibration convenience    | Not started | Factory OTP load, SAR reading conversion, RX/PLL recalibration wrappers.              | HAL already has primitives; API should choose the normal sequence.                                      |
| Diagnostics                | Not started | RX quality summary, CIR read helper, status/error summary.                            | Useful for examples and bring-up debug.                                                                 |
| AES/security convenience   | Deferred    | Auth/encrypt/decrypt wrappers around AES DMA.                                         | Keep separate from first basic TX/RX milestone unless needed by an example.                             |
| External sync convenience  | Deferred    | OSTR setup and sync-readiness helpers.                                                | Advanced TDoA infrastructure feature, not first API milestone.                                          |
| Ranging protocols          | Deferred    | SS-TWR/DS-TWR helpers or examples.                                                    | Build after basic TX/RX/timestamp APIs are stable.                                                      |

## Proposed Milestones

| Milestone                       | Status      | Scope                                                                     | Exit criteria                                                                 |
| ------------------------------- | ----------- | ------------------------------------------------------------------------- | ----------------------------------------------------------------------------- |
| 1. API scaffold and lifecycle   | Initial | Create public header/source, lifecycle structs, init/deinit/recovery wrappers. | App can initialize a caller-owned device without direct HAL sequencing. |
| 2. Basic TX/RX                  | Initial | Immediate TX, blocking TX wait, RX one-frame helper, status cleanup.      | Example can send and receive raw UWB frames using API only.                   |
| 3. Events and timestamps        | Partial | Event decoding, timestamp helpers, delayed-time arithmetic.               | Event decoding and timestamp reads exist; delayed-time arithmetic remains.    |
| 4. Secure timestamp quality     | Not started | STS/CIA quality wrappers and receive-result flags.                        | Caller gets a clear `timestamp_valid` / `secure_timestamp_valid` decision.    |
| 5. Power/calibration ergonomics | Not started | Sleep/wake and normal calibration sequences.                              | Low-power and temperature-compensation examples do not need raw HAL calls.    |
| 6. Examples/tests               | Not started | ESP-IDF examples plus host/mocked tests for API behavior.                 | API has integration proof and repeatable regression checks.                   |

## First API Draft

These are the current first-pass public symbols. Names may still change before
the first examples and tests lock the API down.

| Symbol                       | Status   | Purpose                                                                                  |
| ---------------------------- | -------- | ---------------------------------------------------------------------------------------- |
| `dw3000_api_config_t`        | Initial | Aggregate API-level defaults: bring-up timing and init options.                          |
| `dw3000_api_tx_options_t`    | Initial | Immediate/delayed TX selection, wait-for-response command, timeout behavior.             |
| `dw3000_api_rx_options_t`    | Initial | RX timeout, optional frame wait timeout, timestamp requirements.                         |
| `dw3000_api_rx_result_t`     | Initial | Payload length, RX metadata, timestamp, quality flags, raw status.                       |
| `dw3000_api_init()`          | Initial | Initialize an existing `dw3000_device_t` through the standard HAL sequence.              |
| `dw3000_api_send_frame()`    | Initial | Write TX buffer, configure TX frame, start TX, optionally wait for completion.           |
| `dw3000_api_receive_frame()` | Initial | Start RX and return one frame plus metadata/status within a timeout.                     |
| `dw3000_api_handle_events()` | Initial | Read/clear status and produce a compact event summary.                                   |

## Open Design Questions

| Question                                       | Current leaning                                                                    | Why it matters                                                             |
| ---------------------------------------------- | ---------------------------------------------------------------------------------- | -------------------------------------------------------------------------- |
| Single `dw3000_api.h` or subheaders?           | Start single, split later.                                                         | Keeps first implementation small while preserving future room.             |
| Should API allocate the device?                | No. API functions require a caller-owned `dw3000_device_t*`.                       | Ownership stays explicit; static allocation remains natural and `dw3000_device_new/delete` remains optional outside the API layer. |
| Should API own ESP-IDF SPI bus setup?          | No, not in core API.                                                               | Bus/pin setup is board-specific; use examples or platform helpers.         |
| Blocking vs non-blocking API?                  | Provide bounded blocking helpers first, keep HAL available for custom async flows. | Fastest path to useful examples without locking users out of RTOS designs. |
| Should ranging protocols live in `dw3000_api`? | Not initially.                                                                     | Basic packet/timestamp APIs should stabilize before protocol choreography. |

## Current Priority

1. Add delayed-time arithmetic helpers and lateness-focused result flags.
2. Add STS/CIA timestamp quality wrappers.
3. Add an ESP-IDF example that uses `dw3000_api`, not raw HAL.
4. Add host/mocked tests for API TX/RX/event behavior.
