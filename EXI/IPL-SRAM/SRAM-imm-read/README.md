# SRAM-imm-read
This file simply passes a single command to SRAM to say it wishes to read, then reads a single immediate value one byte at the time.

# Requirements
The ability to understand devices connecting and disconnecting (due to LiboGC requiring this), along with the ability to do proper immediate byte transfers.
When writing an immediate command to this device in SRAM range, 0xff is written to any bytes within the immediate data buffer
The resulting byte always overwrites the highest byte in the immediate data buffer with the necessary value
# Expected results are:
20000100 | 00000004
20000100
ffffffff
(last byte read)ffffff

Along with a hexadecimal view of whatever the contents of SRAM are