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
| API header/source scaffold | Not started | `include/dw3000_api.h`, `src/dw3000_api.c`, optional `dw3000_types/api.h`.            | Start with one small facade before splitting into submodules.                                           |
| Device lifecycle           | Not started | `dw3000_api_init`, `dw3000_api_deinit`, `dw3000_api_reset_recover`.                   | Caller owns `dw3000_device_t` storage; API initializes and recovers an existing context only.           |
| Configuration presets      | Not started | Default PHY/MAC/STS profiles for common channel/preamble/data-rate choices.           | Avoid board policy; board pin/SPI setup remains outside core API.                                       |
| Basic TX                   | Not started | Immediate frame send, delayed frame send, wait-for-TX-complete helper.                | Should handle TX buffer, frame control, start command, status clear, and stuck-TX recovery policy.      |
| Basic RX                   | Not started | Start RX, receive-one-frame with timeout, read payload/metadata/timestamp.            | Should handle RX enable, frame wait timeout, status/error decode, buffer selection, and cleanup.        |
| IRQ/event dispatch         | Not started | Read/clear status and classify events into API event structs.                         | Should support polling and task-level IRQ handling; no ISR-heavy logic.                                 |
| Timestamp helpers          | Not started | Convert/read TX/RX timestamps, delayed-time arithmetic, timeout/lateness helpers.     | Needed before delayed TX/RX and ranging examples are pleasant to use.                                   |
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
| 1. API scaffold and lifecycle   | Not started | Create public header/source, lifecycle structs, init/deinit/recovery wrappers. | App can initialize a caller-owned device without direct HAL sequencing. |
| 2. Basic TX/RX                  | Not started | Immediate TX, blocking TX wait, RX one-frame helper, status cleanup.      | Example can send and receive raw UWB frames using API only.                   |
| 3. Events and timestamps        | Not started | Event decoding, timestamp helpers, delayed-time arithmetic.               | Delayed TX/RX examples can be written without raw register math.              |
| 4. Secure timestamp quality     | Not started | STS/CIA quality wrappers and receive-result flags.                        | Caller gets a clear `timestamp_valid` / `secure_timestamp_valid` decision.    |
| 5. Power/calibration ergonomics | Not started | Sleep/wake and normal calibration sequences.                              | Low-power and temperature-compensation examples do not need raw HAL calls.    |
| 6. Examples/tests               | Not started | ESP-IDF examples plus host/mocked tests for API behavior.                 | API has integration proof and repeatable regression checks.                   |

## First API Draft

These are candidate types/functions for the first milestone. Names may change
when implementation starts.

| Symbol                       | Status   | Purpose                                                                                  |
| ---------------------------- | -------- | ---------------------------------------------------------------------------------------- |
| `dw3000_api_config_t`        | Proposed | Aggregate API-level defaults: bring-up timing, init options, PHY/MAC/STS profile choice. |
| `dw3000_api_tx_options_t`    | Proposed | Immediate/delayed TX selection, wait-for-response, timeout behavior.                     |
| `dw3000_api_rx_options_t`    | Proposed | RX timeout, frame wait timeout, accepted errors, timestamp requirements.                 |
| `dw3000_api_rx_result_t`     | Proposed | Payload length, RX metadata, timestamp, quality flags, raw status.                       |
| `dw3000_api_init()`          | Proposed | Initialize an existing `dw3000_device_t` through the standard HAL sequence.              |
| `dw3000_api_send_frame()`    | Proposed | Write TX buffer, configure TX frame, start TX, optionally wait for completion.           |
| `dw3000_api_receive_frame()` | Proposed | Start RX and return one frame plus metadata/status within a timeout.                     |
| `dw3000_api_handle_events()` | Proposed | Read/clear status and produce a compact event summary.                                   |

## Open Design Questions

| Question                                       | Current leaning                                                                    | Why it matters                                                             |
| ---------------------------------------------- | ---------------------------------------------------------------------------------- | -------------------------------------------------------------------------- |
| Single `dw3000_api.h` or subheaders?           | Start single, split later.                                                         | Keeps first implementation small while preserving future room.             |
| Should API allocate the device?                | No. API functions require a caller-owned `dw3000_device_t*`.                       | Ownership stays explicit; static allocation remains natural and `dw3000_device_new/delete` remains optional outside the API layer. |
| Should API own ESP-IDF SPI bus setup?          | No, not in core API.                                                               | Bus/pin setup is board-specific; use examples or platform helpers.         |
| Blocking vs non-blocking API?                  | Provide bounded blocking helpers first, keep HAL available for custom async flows. | Fastest path to useful examples without locking users out of RTOS designs. |
| Should ranging protocols live in `dw3000_api`? | Not initially.                                                                     | Basic packet/timestamp APIs should stabilize before protocol choreography. |

## Current Priority

1. Define the minimal `dw3000_api.h` scaffold and first lifecycle/TX/RX types.
2. Implement lifecycle wrapper on top of `dw3000_hal_initialize`.
3. Implement immediate TX and receive-one-frame helpers.
4. Add an ESP-IDF example that uses `dw3000_api`, not raw HAL.
