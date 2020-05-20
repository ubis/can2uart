#include <Arduino.h>
#include <CircularBuffer.h>
#include <due_can.h>
#include <ArduinoJson.h>

#define SERIAL_BAUDRATE 2500000
#define CANBUS_BAUDRATE 125000

#define BUFFER_LENGTH   65
#define MSG_QUEUE_SIZE  128

void ParseInput(void);
void CanRecv(CAN_FRAME *frame);

CircularBuffer<CAN_FRAME, MSG_QUEUE_SIZE> m_tx_buffer;
CircularBuffer<CAN_FRAME, MSG_QUEUE_SIZE> m_rx_buffer;
char m_buffer[BUFFER_LENGTH];

void setup() {
    // init serial console
    SerialUSB.begin(SERIAL_BAUDRATE);

    // init can bus
    Can0.init(CANBUS_BAUDRATE);
    Can0.setRXFilter(0, 0, true);
    Can0.attachCANInterrupt(CanRecv);
}

void loop() {
    ParseInput();

    if (!m_rx_buffer.isEmpty()) {
        CAN_FRAME frame = m_rx_buffer.shift();
        StaticJsonDocument<JSON_ARRAY_SIZE(8) + JSON_OBJECT_SIZE(3)> buffer;

        DynamicJsonDocument root(1024);
        root["ID"]     = frame.id;
        root["Length"] = frame.length;

        JsonArray data = root.createNestedArray("Data");

        for (uint8_t i = 0; i < frame.length; i ++) {
            data.add(frame.data.bytes[i]);
        }

        serializeJson(root, SerialUSB);
        SerialUSB.print("\r\n");
    }

    if (!m_tx_buffer.isEmpty()) {
        CAN_FRAME frame = m_tx_buffer.shift();
        Can0.sendFrame(frame);
    }
}

void ParseInput(void) {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '{';
    char endMarker = '}';
    char rc;

    while (SerialUSB.available() > 0) {
        rc = SerialUSB.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                m_buffer[ndx] = rc;
                ndx++;

                // overflow protection
                if (ndx >= BUFFER_LENGTH)
                    ndx = BUFFER_LENGTH - 1;
            } else {
                // add ending marker
                m_buffer[ndx] = rc;
                ndx++;

                // add string termination
                m_buffer[ndx] = '\0';
                recvInProgress = false;
                ndx = 0;

                // parse data
                StaticJsonDocument<JSON_ARRAY_SIZE(8) + JSON_OBJECT_SIZE(3)> buffer;
                deserializeJson(buffer, m_buffer);

                // create CAN frame
                CAN_FRAME frame;
                frame.id       = buffer["ID"];
                frame.length   = buffer["Length"];
                frame.extended = 1;

                for (int i = 0; i < frame.length; i ++) {
                    frame.data.bytes[i] = buffer["Data"][i];
                }

                // add to transmit buffer
                m_tx_buffer.push(frame);
            }
        } else if (rc == startMarker) {
            recvInProgress = true;
            m_buffer[ndx] = rc;
            ndx++;
        }
    }
}

void CanRecv(CAN_FRAME *frame) {
    m_rx_buffer.push(*frame);
}
