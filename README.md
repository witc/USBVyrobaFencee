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

**Table of radio commands**
| **cmd meaning**  | **opCode** 1B  | |**parameters** NB|  **retVal** 1B
|---|---|--|--|--|
| SetAUXPin_1  | 1  |  Log level 1/0 (1 B)| |
| SetAUXPin_2  | 2  |  Log level 1/0 (1 B)| |
| SetAUXPin_3  | 3  |  Log level 1/0 (1 B)| |
| SetAUXPin_4  | 4  |  Log level 1/0 (1 B)| |
| ReadAUXPin_7  | 7  |  Log level 1/0 (1 B)| |
| ReadAUXPin_8  | 8  |  Log level 1/0 (1 B)| |
| ButtonPushed  | 7  |  Button (5 nebo 6 - dle tlacitka) (1 B)| |
| SetAUX1_Blinking  | 8  |  TODO period time (4 B)| |
| SetAUX2_Blinking  | 9  |  TODO period time (4 B)| |
| SetAUX3_Blinking  | 10  |  TODO period time (4 B)| |
| SetAUX4_Blinking  | 11  |  TODO period time (4 B)| |


- AUX 4,3,2,1 => Vystupy
- AUX 5,6 => Tlacitka reagujici na sestupnou hranu
- AUX 8,7 => Pro cteni vstupu

![IMG_20210623_103957](https://user-images.githubusercontent.com/13749560/123067524-5aedd280-d411-11eb-891e-3f4466e4b76a.jpg)
