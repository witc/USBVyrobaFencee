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
| **cmd meaning**  | **opCode** 1B  |**action flags** 1B |**parameters** NB|  **retVal** 1B
|---|---|--|--|--|
| TxFreq  | 1  | yes | Frequency (4B)| 1 - Succes /0 - failure|
| RxFreq  | 2  | yes |  Frequency (4B)| 1 - Succes /0 - failure|
| TxPower  | 3  | yes | Power (1B)| 1 - Succes /0 - failure|
| TxSF  | 4  | yes | Spreading factor SF5-SF12 (1B)| 1 - Succes /0 - failure|
