
from PyQt5 import QtGui, QtCore
import pyqtgraph as pg
import numpy as np
import sys
import math
import serial
import struct
import math
import time
import xml.etree.ElementTree as ET
import kmlparser


ser = None
dt = 0  # Time delta in milliseconds
element_count = 0
curves = list()
curve_xdata = list()

size = 500
buffersize = 2*500

curr_longitude = 0
curr_latitude = 0
curr_altitude = 0
initialized = 0
fix = 0

ET.register_namespace("","http://www.opengis.net/kml/2.2")
timestr = time.strftime("%d%m%Y-%H%M")
f = open("Serial-" + timestr + ".txt", 'w')

ser = serial.Serial(port='COM12', baudrate=115200, timeout=2)


def make_curves(x, px):
    global element_count, curves, curve_xdata, buffersize
    for x in range(element_count):
        curves[x] = px.plot()
        curve_xdata[x] = np.zeros(buffersize+1, int)


def shift_elements(buffer, csv):
    global size, buffersize, element_count
    i = buffer[buffersize]
    buffer[i] = buffer[i+size] = csv[0]
    buffer[buffersize] = i = (i+1) % size


def close_app():
    sys.exit()

# for Spyder. When you close your window, the QtApplicaiton instance is
# still there after being created once. Therefore check if a Qt instance
# already exists, if it does, then use it, otherwise, create new instance


if not QtGui.QApplication.instance():
    app = QtGui.QApplication([])
else:
    app = QtGui.QApplication.instance()

# win = QtGui.QWidget()

win = pg.GraphicsWindow(title="Basic plotting examples")
win.setWindowTitle("Titan Plot")
win.resize(1000, 600)
# layout = QtGui.QGridLayout()
# win.setLayout(layout)

# p1 = pg.PlotWidget()

p1 = win.addPlot()
p1.setRange(yRange=[-16000, 16000])
p1.addLegend()
p1.showGrid(x=True, y=True, alpha=0.8)
p1.setLabel('left', 'Amplitude (16bit Signed)')

curve1 = p1.plot(pen='y', name="Acc X")
curve2 = p1.plot(pen='g', name="Acc Y")
curve3 = p1.plot(pen='r', name="Acc Z")

win.nextRow()
p2 = win.addPlot()
p2.setRange(yRange=[-200, 200])
p2.showGrid(x=True, y=True, alpha=0.8)
p2.setLabel('left', 'Altitude (M)')

altitude_curve = p2.plot(pen='r', name="Altitude")

# layout.addWidget(p1, 0, 0, 1, 3)

buffer1 = np.zeros(buffersize+1, int)
buffer2 = np.zeros(buffersize+1, int)
buffer3 = np.zeros(buffersize+1, int)
altitude_buffer = np.zeros(buffersize+1, int)

x = 0

# primary loop called by PyQtPlot timer update. Set dt to 0 to stop lag and update on new packet received


def update():
    global curve1, curve2, x, size, buffersize, lastByte, nextByte, initialized, fix, curr_latitude, curr_longitude, \
        curr_altitude, gps_coords, gps_string, tree, root, newPacket

    nextByte = bytes(ser.read())

    try:
        lastByte
    except NameError:
        lastByte = None

    if lastByte:
        if lastByte[0] == 6 and nextByte[0] == 133:

            mesLength = bytes(ser.read())
            payload = bytes(ser.read(mesLength[0] + 1))

            if payload:
                if payload[0] == 254 and len(payload) == 25:

                    gpsPacket = struct.unpack('B iii IH hb', payload)
                    print(gpsPacket)
                    if gpsPacket[6] == 0:
                        print(gpsPacket)

                        if gpsPacket[2] == 0 and gpsPacket[3] == 0:
                            print('No Fix')
                            fix = 0
                        else:

                            curr_altitude = int(gpsPacket[1]) / 100  # altitude
                            curr_latitude = int(gpsPacket[2]) / 10000000  # lat
                            curr_longitude = int(gpsPacket[3]) / 10000000  # long
                            gps_coords = [str(curr_longitude), str(curr_latitude), str(curr_altitude)]
                            gps_string = ",".join(gps_coords)  # convert whole thing to string

                            fix = 1
                            newPacket = 1

                            f.write(str(gpsPacket) + '\n')

                if payload[0] == 69 and len(payload) == 29:

                    # print(len(payload))
                    accPacket = struct.unpack('B hhh hhh hhh i h hB', payload)
                    # print(accPacket)
                    if accPacket[len(accPacket)-2] == 0:
                        # print(accPacket)
                        try:
                            altitudeM = 0.3048 * ((1 - math.pow(accPacket[10] / 1013.25, 0.190284)) * 145366.45)

                            if altitudeM > -10000:

                                altitude_counter = altitude_buffer[buffersize]
                                altitude_buffer[altitude_counter] = altitude_buffer[altitude_counter + size] = float(altitudeM)
                                altitude_buffer[buffersize] = altitude_counter = (altitude_counter + 1) % size

                                altitude_curve.setData(altitude_buffer[altitude_counter:altitude_counter + size])
                                altitude_curve.setPos(x, 0)
                                # print(altitudeM)

                                # print(accPacket)
                                gyrox = accPacket[1]
                                gyroy = accPacket[2]
                                gyroz = accPacket[3]

                                accx = accPacket[4]
                                accy = accPacket[5]
                                accz = accPacket[6]

                                magx = accPacket[7]
                                magy = accPacket[8]
                                magz = accPacket[9]

                                pressuremb = accPacket[10]

                                tempC = 42.5 + float(accPacket[11]) / 480
                                f.write(str(accPacket) + '\n')

                                # ======================================================================
                                # This is the main plotting process
                                x += 1

                                i = buffer1[buffersize]
                                buffer1[i] = buffer1[i + size] = (accx * 0.488)
                                buffer1[buffersize] = i = (i + 1) % size

                                j = buffer2[buffersize]
                                buffer2[j] = buffer2[j + size] = (accy * 0.488)
                                buffer2[buffersize] = j = (j + 1) % size

                                k = buffer3[buffersize]
                                buffer3[k] = buffer3[k + size] = (accz * 0.488)
                                buffer3[buffersize] = k = (k + 1) % size

                                curve1.setData(buffer1[i:i + size])
                                curve1.setPos(x, 0)

                                curve2.setData(buffer2[j:j + size])
                                curve2.setPos(x, 0)

                                curve3.setData(buffer3[k:k + size])
                                curve3.setPos(x, 0)

                        except ValueError:
                            print('failed packet')





                        # print(altitudeM)
                        # app.processEvents()
                        # =======================================================================
    else:
        print('serial not available')

    app.processEvents()
    lastByte = nextByte

    # initialize kml file if there is a fix
    if initialized == 0 and fix == 1:
        kmlstring = "<?xml version='1.0' encoding='UTF-8'?>" \
                    '<kml xmlns="http://www.opengis.net/kml/2.2">' \
                    '<Document>' \
                    '<name>Flight Path</name>' \
                    '<snippet>' \
                    'Created with the MCW HPR KML Creator v1.00 at 8/05/2019 12:17:06 AM' \
                    '</snippet>' \
                    '<Style id="multitrack">' \
                    '<IconStyle><Icon /></IconStyle>' \
                    '<LabelStyle><scale>0</scale></LabelStyle>' \
                    '</Style>' \
                    '<Style id="fs_1_Split">' \
                    '<LineStyle>' \
                    '<color>FFFFFFFF</color>' \
                    '<width>4</width>' \
                    '</LineStyle>' \
                    '<PolyStyle>' \
                    '<color>FF0000FF</color>' \
                    '</PolyStyle>' \
                    '</Style>' \
                    '<LookAt>' \
                    '<longitude>' + gps_coords[0] + '</longitude>' \
                    '<latitude>' + gps_coords[1] + '</latitude>' \
                    '<altitude>' + gps_coords[2] + '</altitude>' \
                    '<range>1000</range>' \
                    '<tilt>45</tilt>' \
                    '</LookAt>' \
                    '<Placemark>' \
                    '<name>CurrentLoc</name>' \
                    '<styleUrl>#fs_1_Split</styleUrl>' \
                    '<LineString>' \
                    '<extrude>1</extrude>' \
                    '<tessellate>1</tessellate>' \
                    '<altitudeMode>absolute</altitudeMode>' \
                    '<coordinates>' \
                    + gps_string + \
                    '</coordinates>' \
                    '</LineString>' \
                    '</Placemark>' \
                    '</Document>' \
                    '</kml>'

        tree = ET.ElementTree(ET.fromstring(kmlstring))
        tree.write(timestr + '.kml', xml_declaration=True, encoding="UTF-8")
        tree.write('NetWork_Current.kml', xml_declaration=True, encoding="UTF-8")
        root = tree.getroot()
        initialized = 1

    # if the kml file has been initialized and there is still a fix, parse gps data into kml
    if initialized == 1 and newPacket == 1:

        # access the root of the XML tree
        # root = tree.getroot()
        # navigate indices of root to find coordinates
        root[0][5][2][3].text += "\n" + gps_string
        # change coordinates text, adding the current GPS location
        # accesses the look at section of the KML and tells the camera to look at the most recent GPS coord
        root[0][4][0].text = gps_coords[0]
        root[0][4][1].text = gps_coords[1]
        root[0][4][2].text = gps_coords[2]

        # write new root to tree
        tree = ET.ElementTree(root)
        # write new tree to file
        tree.write(timestr + '.kml', xml_declaration=True, encoding="UTF-8")
        tree.write('NetWork_Current.kml', xml_declaration=True, encoding="UTF-8")

        newPacket = 0

timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(dt)
timer.setInterval(dt)
# if(ser != None):
#  timer.stop()
win.show()
app.exec_()
