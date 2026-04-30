# DW3000 HAL SS-TWR Responder

Hardware smoke test for a two-node single-sided TWR exchange. Flash this app on
the responder node and `hal_ss_twr_initiator` on the initiator node.

The responder:

- initializes the DW3000 with the MakerFabs-compatible radio profile,
- waits for poll frames,
- reads the poll RX timestamp,
- schedules a delayed response using `DX_TIME`,
- transmits the response with poll RX and scheduled response TX timestamps.

Build, flash, and monitor:

```sh
cd components/dw3000/test_apps/hal_ss_twr_responder
idf.py set-target esp32
idf.py -p PORT build flash monitor
```

Start this responder first, then start the initiator.
