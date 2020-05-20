# CAN2UART

Simple CAN Bus to UART tranceiver. Powered by Arduino Due board.

Utilizes SerialUSB port and CAN interface. Data is sent/received in json format.

Basically it converts CAN frames to json format and sends it through SerialUSB while also receives CAN frames in the same json format and sends CAN messages.

Using the following libraries:

* Arduino Json
* ArduinoStreamUtils (needed for Arduino Json)
* CircularBuffer
* due_can

Note: this project is generated with PlatformIO Arduino platform.

An example of json CAN message:

```
{"ID":269418753,"Length":5,"Data":[60,151,1,10,11]}
```

Using Extended CAN aka CAN 2.0 with 29-bit identifier.

You might want to change Serial/CAN rates, as of now SerialUSB runs at 2500000bps and CAN at 125 kbit/s.