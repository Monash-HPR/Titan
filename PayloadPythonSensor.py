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
x = 0

acc_string = ' a/g'
print(id(acc_string))

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
        # print(line)
        words = line.split(',')
        if 400000 < x < 480000:
            if words[1] == ' a/g' and len(words) == 8:
                accx.append(int(words[2])*0.488)
                accy.append(int(words[3])*0.488)
                accz.append(int(words[4])*0.488)
                gyrox.append(int(words[5]))
                gyroy.append(int(words[6]))
                gyroz.append(int(words[7]))
            if str(words[1]) == " m":
                magx.append(int(words[2]))
                magy.append(int(words[3]))
                magz.append(int(words[4]))
            if str(words[1]) == " tph":
                pressure.append(float(words[3]))
                temp.append(float(words[2]))
        x += 1
        line = f.readline()

    print(accx)
    altitude = [(1 - math.pow(i / 1009.983, 0.190263)) * 44330.8 for i in pressure]
    clean_altitude = dataclean(altitude)
    clean_accx = dataclean(accx)
    clean_accy = dataclean(accy)
    clean_accz = dataclean(accz)
    clean_temp = dataclean(temp)

    xmax = np.arange(0, len(altitude))[np.argmax(altitude)]
    ymax = max(altitude)


    text= "x={:.3f}, y={:.3f}".format(xmax, ymax)
    bbox_props = dict(boxstyle="square,pad=0.3", fc="w", ec="k", lw=0.72)
    arrowprops=dict(arrowstyle="->",connectionstyle="angle,angleA=0,angleB=60")
    kw = dict(xycoords='data',textcoords="axes fraction",
              arrowprops=arrowprops, bbox=bbox_props, ha="right", va="top")

    for i in accx:
        if i > 2000:
            count += count
        else:
            count = 0

        if count > 10:
            launch_index = i
            print(launch_index)
            break

    fig, axs = plt.subplots(2, 1)
    length = np.arange(0, len(accx))
    axs[0].plot(length, accx, length, accy, length, accz)
    axs[1].plot(np.arange(0, len(altitude)), altitude)
    axs2 = axs[1].twinx()
    color = 'tab:red'
    axs2.set_ylabel('temp', color=color)
    axs2.plot(np.arange(0, len(temp)), temp, color=color)

    # axs[0, 1].plot(clean_accx[1], clean_accx[0], clean_accy[1], clean_accy[0], clean_accz[1], clean_accz[0])
    axs[1].annotate(text, xy=(xmax, ymax), xytext=(0.94, 0.96), **kw)

    fig.tight_layout()
    plt.show()
