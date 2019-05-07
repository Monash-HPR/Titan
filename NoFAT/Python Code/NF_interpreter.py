##From read_until sequence from serialcapture.py

#!/usr/bin/env python3
from bitstring import BitArray, BitStream
import base64
import binascii
import datetime

class Datum:
    def __init__(self,sensorN,feedN,channelN,valueN):

        self.sensor = sensorN
        self.feed = feedN
        self.time = datetime.datetime.now()

        if channelN % 2 == 1:
            self.data = valueN * -1
        else:
            self.data = valueN

        self.channel = (channelN - 9) - (channelN % 2)


def NF_rawToDatum(packet):
    ###############################################################
    # separate into nibbles

    # For Base32
    convTable = (
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V')

    # CONTROL
    nib_sensor = BitArray(bytes=packet, length=5, offset=0)
    nib_feed = BitArray(bytes=packet, length=3, offset=5)

    # 1
    nib_A1 = BitArray(bytes=packet, length=4, offset=8)
    nib_A2 = BitArray(bytes=packet, length=4, offset=12)

    # 2
    nib_B1 = BitArray(bytes=packet, length=4, offset=16)
    nib_B2 = BitArray(bytes=packet, length=4, offset=20)

    # 3
    nib_C1 = BitArray(bytes=packet, length=4, offset=24)
    nib_C2 = BitArray(bytes=packet, length=4, offset=28)

    # 4
    nib_D1 = BitArray(bytes=packet, length=4, offset=32)
    nib_D2 = BitArray(bytes=packet, length=4, offset=36)

    # TERMINATE
    byt_terminator = BitArray(bytes=packet, length=8, offset=40)

    ###############################################################
    # convert nibbles to plain values

    plain_sensor = convTable[nib_sensor.int]
    plain_feed = nib_feed.uint

    plain_A1 = nib_A1.uint
    plain_A2 = nib_A2.uint

    plain_B1 = nib_B1.uint
    plain_B2 = nib_B2.uint

    plain_C1 = nib_C1.uint
    plain_C2 = nib_C2.uint

    plain_D1 = nib_D1.uint
    plain_D2 = nib_D2.uint

    plain_terminator = byt_terminator.uint

    ###############################################################
    # convert plain values to sensible data

    value = int(str(plain_B1) + str(plain_B2) + str(plain_C1) + str(plain_C2) + str(plain_D1) + str(plain_D2)) * 10 ** (plain_A2 - 5)
    print(value)

    output = Datum(plain_sensor,plain_feed,plain_A1,value)

    return output

dataclass = NF_rawToDatum(bytes([80,178,86,87,101,255]))

print(dataclass.sensor)
