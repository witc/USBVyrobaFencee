# USBVyrobaFencee

# USART Packet:

| 2B  |  4B  |  n |  1B |
|---|---|---|---|
| Sync Word  - 0x2DD4  |Header|  payload |  crc - from whole packet |

**Header**
| 1B  |  2B | 1B  | 
|---|---|---|
| size of payload  |RFU| crc Header  

**Payload**
| 1B    | NB|
|---|---|
| Command (opCode) | |   


**CRC8 implementation with polynom = x7+ x6+ x4+ x2+ x0 (0xD5)**
CRC is the same for header and whole packet

**Table of radio commands **
| **cmd meaning**  | **opCode** 1B  | |**parameters** NB|  **retVal** 1B
|---|---|--|--|--|
| SetAUXPin_1  | 1  |  Log level 1/0 (1 B)| |
| SetAUXPin_2  | 2  |  Log level 1/0 (1 B)| |
| SetAUXPin_3  | 3  |  Log level 1/0 (1 B)| |
| SetAUXPin_4  | 4  |  Log level 1/0 (1 B)| |
| ReadtAUXPin_5  | 5  |  Log level 1/0 (1 B)| |
| ReadtAUXPin_6  | 6  |  Log level 1/0 (1 B)| |
| ButtonPushed  | 7  |  Button (8 nebo 7 - dle tlacitka) (1 B)| |
| SetAUX1_Blinking  | 8  |  TODO period time (4 B)| |
| SetAUX2_Blinking  | 8  |  TODO period time (4 B)| |
| SetAUX3_Blinking  | 8  |  TODO period time (4 B)| |
| SetAUX4_Blinking  | 8  |  TODO period time (4 B)| |
