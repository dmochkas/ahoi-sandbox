import serial

ser = serial.Serial('COM5', 115200, timeout=None) # always reciever 
 # This is the change, it's just to know 
def decode_ahoi_packet(data):
    # Extract the  header: [src, dst, type, status, dsn, len]
    header = data[:6]
    payload_len = header[5]

    payload = data[6:(6 + header[5])]  #use the part len of length 
    footer =data[6 +  payload_len : 6 + payload_len + 6]
    print(f"Reciever ID={header[0]}: {payload.decode('ascii')}") # read id y deco
    print(f"Footer: {footer.hex(' ')}")

    # Interpretation footer
    if len(footer) == 6:
        power, rssi, biterrors, agcMean, agcMin, agcMax = footer
        print(f"  - Power (RMSE): {power}%")
        print(f"  - RSSI: {rssi}%")
        print(f"  - Bit errors: {biterrors}")
        print(f"  - AGC (mean/min/max): {agcMean}/{agcMin}/{agcMax}")

print("waiting for the package AHOI...")
buffer = bytearray()
while True:
    byte = ser.read(1) # detect DLE
    if byte == b'\x10':
        next_byte = ser.read(1)
        if next_byte == b'\x02':  # STX start
            buffer = bytearray()
        elif next_byte == b'\x03':  # ETX end
            decode_ahoi_packet(buffer)
        elif next_byte == b'\x10':  # Byte escaped
            buffer.append(0x10)
    else:
        buffer += byte # save the data
