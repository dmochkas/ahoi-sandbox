import serial
import time

# Configuration serial
ser = serial.Serial('COM6', 115200, timeout=1) #We define the serial port with the speed and timeout =1 to send just one time

def send_ahoi_packet(dst_id, src_id, packet_type, payload):
    # Header AHOI: [src, dst, type, status=0x00, dsn=0x00, len]
    header = bytearray([src_id, dst_id, packet_type, 0x00, 0x00, len(payload)]) # len = message length 
    packet = header + payload.encode('ascii') # Codif

   # packet = bytearray([0x58, 0x56, 0x10, 0x00, 0x00, 0x04, 0x00, 0x10, 0x20, 0x40])

    # Scape 0x10 and framing
    escaped = bytearray()
    for byte in packet:
        if byte == 0x10:
            escaped.append(0x10) # add the 0x10 
        escaped.append(byte)
    framed = bytearray([0x10, 0x02]) + escaped + bytearray([0x10, 0x03]) # beggin and end

    ser.write(framed) # send for the serial port
    print(f"Sending: {framed.hex(' ')}") # notif

    # 01 02 10 00 00 04 00 10 20 40 10 11 00 0A 09 0A
    #Example
user_payload = input("Enter the word to: ")

# Example: send "hola" de ID=00 to ID=01, tipo=0
send_ahoi_packet(dst_id=0x56, src_id=0x58, packet_type=0x00, payload=user_payload)

ser.close()