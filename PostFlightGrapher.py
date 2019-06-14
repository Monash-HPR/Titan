import tkinter as tk
from tkinter import filedialog
import numpy as np
import matplotlib.pyplot as plt
import math

root = tk.Tk()
root.withdraw()

file_path = filedialog.askopenfilename()
accx = []
accy = []
accz = []

gyrox = []
gyroy = []
gyroz = []

magx = []
magy =[]
magz = []

pressure = []
temp = []
count = 0

def dataclean(list):
    mean = np.mean(list)
    sd = np.std(list)
    out_list = [x for x in list if (x > mean - 4 * sd)]
    out_list = [x for x in out_list if (x < mean + 4 * sd)]
    return out_list, np.arange(0, len(out_list))

print(file_path)

with open(file_path) as f:
    line = f.readline()
    while line:
        line = line.replace('(', '')
        line = line.replace(')', '')
        # print(line)
        words = line.split(',')
        if int(words[0]) == 69:
            # print(words[0])

            gyrox.append(int(words[1]))
            gyroy.append(int(words[2]))
            gyroz.append(int(words[3]))

            accx.append(int(words[4])*0.488)
            accy.append(int(words[5])*0.488)
            accz.append(int(words[6])*0.488)

            magx.append(int(words[7]))
            magy.append(int(words[8]))
            magz.append(int(words[9]))

            pressure.append(float(words[10]))
            temp.append(words[11])

        line = f.readline()

    # print(accx)
    altitude = [(1 - math.pow(i / 1013.25, 0.190263)) * 44330.8 for i in pressure]
    tempC = [42.5 + float(i) / 480 for i in temp]
    clean_altitude = dataclean(altitude)
    clean_accx = dataclean(accx)
    clean_accy = dataclean(accy)
    clean_accz = dataclean(accz)
    clean_temp = dataclean(tempC)

    xmax = clean_altitude[1][np.argmax(clean_altitude[0])]
    ymax = max(clean_altitude[0])


    text= "x={:.3f}, y={:.3f}".format(xmax, ymax)
    bbox_props = dict(boxstyle="square,pad=0.3", fc="w", ec="k", lw=0.72)
    arrowprops=dict(arrowstyle="->",connectionstyle="angle,angleA=0,angleB=60")
    kw = dict(xycoords='data',textcoords="axes fraction",
              arrowprops=arrowprops, bbox=bbox_props, ha="right", va="top")

    for i in clean_accx[1]:
        if i > 2000:
            count += count
        else:
            count = 0

        if count > 10:
            launch_index = i
            print(launch_index)
            break


    fig, axs = plt.subplots(2, 2)
    length = np.arange(0, len(accx))
    axs[0, 0].plot(length, accx, length, accy, length, accz)
    axs[1, 0].plot(clean_altitude[1], clean_altitude[0])

    axs[0, 1].plot(clean_accx[1], clean_accx[0], clean_accy[1], clean_accy[0], clean_accz[1], clean_accz[0])
    axs[1, 0].annotate(text, xy=(xmax, ymax), xytext=(0.94, 0.96), **kw)
    axs[1, 1].plot(clean_temp[1], clean_temp[0])

    fig.tight_layout()
    plt.show()
