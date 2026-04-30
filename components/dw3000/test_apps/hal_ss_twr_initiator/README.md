# DW3000 HAL SS-TWR Initiator

Hardware smoke test for a two-node single-sided TWR exchange. Flash this app on
the initiator node and `hal_ss_twr_responder` on the responder node.

The app:

- initializes the DW3000 with the MakerFabs-compatible radio profile,
- sends poll frames,
- receives delayed response frames,
- reads poll TX and response RX timestamps,
- consumes responder-provided poll RX and response TX timestamps,
- prints rough single-sided ToF and distance estimates.

Build, flash, and monitor:

```sh
cd components/dw3000/test_apps/hal_ss_twr_initiator
idf.py set-target esp32
idf.py -p PORT build flash monitor
```

Start the responder first, then start this initiator.
