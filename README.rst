.. _nrf91_nrf52_serial_echo:

Serial port between nRF91 and nRF52
###################################

Overview
********

This sample demonstrates how to route a serial port between nRF9160 and nRF52840 devices
on Nordic's nRF9160 DK development board.

Application itself is based on Zephyr's serial Echo bot sample, but uses two UARTs instead of one.
Characters received from console, are forwared into a serial port between nRF91 and nRF52.
Charactes received from this inter-chip serial port, are echoed on the screen.

Study material
**************

* `Zehypr documentation: Device Tree <https://docs.zephyrproject.org/latest/build/dts/index.html>`_
* `nRF Connect SDK: Pincontrol <https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/ug_pinctrl.html>`_
