# DW3000 HAL Mocked Unit Tests

This ESP-IDF Unity app runs HAL tests against a fake `dw3000_port_t` register
backend. It is intended for behavior that cannot be proven by pure helper tests
alone, such as register packing, write-one-to-clear semantics, cached state, and
error propagation.

Build, flash, and monitor:

```sh
cd components/dw3000/tests/hal_mock
idf.py set-target esp32
idf.py build flash monitor
```

The initial tests validate `SYS_STATUS` / `SYS_ENABLE` read, clear, enable,
disable, cached event handling, IO error propagation, TX frame preparation,
TX timestamp reads, and TX fast-command sequencing. The expected Unity output
contains four passing tests:

```text
4 Tests 0 Failures 0 Ignored
OK
```

Extend `mock_dw3000_port` as additional HAL flows need direct or indirect
register sequencing checks.
