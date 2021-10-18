"""MouseLickLED
DeepLabCut Toolbox (deeplabcut.org)
© A. & M. Mathis Labs

Licensed under GNU Lesser General Public License v3.0
"""


import serial
import struct
import time
import numpy as np
import pandas as pd
import time

from dlclive import Processor


class CuttlefishExpt(Processor):
    def __init__(self, 
            com = 1, 
            lik_thresh = 0.5, 
            baudrate = int(9600), 
            jose_x_pars = 630.0, 
            jose_y_pars = 540.0,
            soft_y_up = 500.0,
            soft_y_dn = 580.0,
            L_amplitude = 20, 
            R_amplitude = 20, 
            swing_freq = 255, 
            swing_period = 0,
            swap_delay = 1000,
            if_random = 0):

        super().__init__()
        self.ser = serial.Serial('/dev/ttyACM'+str(com), baudrate)
        self.lik_thresh = lik_thresh
        self.frame_time = []
        self.out_time = []
        self.in_time = []
        self.jose_x_pars = jose_x_pars
        self.jose_y_pars = jose_y_pars
        self.soft_y_up = soft_y_up
        self.soft_y_dn = soft_y_dn
        self.L_amplitude = L_amplitude
        self.R_amplitude = R_amplitude
        self.if_random = if_random

        self.frame_num = 0
        self.frame_numls = []

        self.posels = []

        self.frame_t0 = time.time()
        self.frame_t1 = time.time()

        if self.ser.isOpen():
            print ('hellow cuttlefish!!!')

        self.ser.reset_input_buffer()
        # self.ser.write(b"L")

        self.R_buffer = 0
        self.L_buffer = 0

        self.shrimp_count = 0
        self.shrimp_dead_count = 0
        self.shrimp_home = False
        self.shrimp_loc = 0         # 0: start state, 1: left, 2: right

        self.timeout = 0
        self.timeout_start = 650

        # print ("wait for Arduino...")
        # while (self.ser.inWaiting() == 0):
        #     pass
        # self.ser.read(self.ser.inWaiting())
        print ("send parameter~")
        time.sleep(1)
        self.ser.write(b"Z")
        time.sleep(1)
        self.ser.write("{} {} {}".format(int(swing_freq), int(swing_period), int(swap_delay)).encode())
        # time.sleep(1)
        # print ("move to middle~")
        # self.ser.write(b"M")
        # self.ser.write(b"M")
        # self.ser.write(b"M")
        # self.ser.write(b"M")

    def close_serial(self):
        self.ser.close()

    def switch_to_left(self):
        ### flush input buffer ###
        self.ser.reset_input_buffer()
        self.ser.write(b"L")
        print ('To LEFT~~~')

    def switch_to_right(self):
        ### flush input buffer ###
        self.ser.reset_input_buffer()
        self.ser.write(b"R")
        print ('To RIGHT~~~')

    def switch_to_left_swing(self):
        ### flush input buffer ###
        self.ser.reset_input_buffer()
        if self.R_amplitude == 0:
            if self.if_random:
                self.ser.write(b"l")
            else:
                self.ser.write(b"L")
        elif self.L_amplitude == 10:
            if self.if_random:
                self.ser.write(b"a")
            else:
                self.ser.write(b"A")
        elif self.L_amplitude == 20:
            if self.if_random:
                self.ser.write(b"c")
            else:
                self.ser.write(b"C")
        elif self.L_amplitude == 40:
            if self.if_random:
                self.ser.write(b"e")
            else:
                self.ser.write(b"E")
        elif self.L_amplitude == 50:
            self.ser.write(b"G")
        print ('To LEFT swing~~~')
        self.shrimp_loc == 1

    def switch_to_right_swing(self):
        ### flush input buffer ###
        self.ser.reset_input_buffer()
        if self.R_amplitude == 0:
            if self.if_random:
                self.ser.write(b"r")
            else:
                self.ser.write(b"R")
        elif self.R_amplitude == 10:
            if self.if_random:
                self.ser.write(b"b")
            else:
                self.ser.write(b"B")
        elif self.R_amplitude == 20:
            if self.if_random:
                self.ser.write(b"d")
            else:
                self.ser.write(b"D")
        elif self.R_amplitude == 40:
            if self.if_random:
                self.ser.write(b"f")
            else:
                self.ser.write(b"F")
        elif self.R_amplitude == 50:
            self.ser.write(b"H")
        print ('To RIGHT swing~~~')
        self.shrimp_loc == 2

    def switch_to_startpoint(self):
        ### flush input buffer ###
        self.ser.reset_input_buffer()
        self.ser.write(b"O")

    def process(self, pose, **kwargs):

        ### bodyparts
        # 0. front_tip
        # 1. middle
        # 2. left
        # 3. right
        # 4. tail_tip
        # 5. shrimp_head
        # 6. shrimp_body
        # 7. shrimp_tail

        self.timeout += 1
        if (self.timeout < self.timeout_start):
            print ('time out: ', self.timeout_start - self.timeout)
            self.ser.write(b"M")
            return pose

        if kwargs["record"]:
            self.frame_time.append(kwargs["frame_time"])
            self.frame_numls.append(self.frame_num)
            self.posels.append(pose)
            self.frame_num += 1

        print ('pose:=======')
        print(np.array_str(pose, precision=3, suppress_small=True))


        is_cutlef = 0
        for j in range(5):
            if pose[j, 2] > self.lik_thresh:
                is_cutlef += 1

        is_shrimp = 0
        for k in range(3):
            if pose[k+5, 2] > self.lik_thresh:
                is_shrimp += 1
        
        if is_shrimp > 1:
            print (self.shrimp_count, self.shrimp_dead_count, 'shrimp is happy!!!!!')
        else:
            print ('shrimp missing...')
        if is_cutlef > 3:
            print ('cuttlefish is happy!!!')
        else:
            print ('shrimp missing...')

        # if (pose[1, 2] > self.lik_thresh) and (pose[6, 2] > self.lik_thresh):
        if (is_cutlef > 3):
            if pose[1, 0] < self.jose_x_pars and pose[1, 1] < self.jose_y_pars:
                self.L_buffer += 1
                self.R_buffer = 0
            elif pose[1, 0] < self.jose_x_pars and pose[1, 1] > self.jose_y_pars:
                self.L_buffer = 0
                self.R_buffer += 1
            elif pose[1, 0] > self.jose_x_pars and pose[1, 1] < self.soft_y_up:
                self.L_buffer = 0
                self.R_buffer += 1
            elif pose[1, 0] > self.jose_x_pars and pose[1, 1] > self.soft_y_dn:
                self.L_buffer += 1
                self.R_buffer = 0
            elif pose[1, 0] > self.jose_x_pars and pose[1, 1] > self.soft_y_up and pose[0, 1] < self.soft_y_up:
                if self.shrimp_loc == 0:
                    if pose[1, 1] < self.jose_y_pars:
                        self.L_buffer = 0
                        self.R_buffer += 1
                    elif pose[1, 1] > self.jose_y_pars:
                        self.L_buffer += 1
                        self.R_buffer = 0
        else:
            self.L_buffer = 0
            self.R_buffer = 0

        print ('L_buffer ', self.L_buffer)
        print ('R_buffer ', self.R_buffer)

        if self.L_buffer > 5:
            self.switch_to_left_swing()
        elif self.R_buffer > 5:
            self.switch_to_right_swing()

        self.frame_t1 = time.time()
        frametime = self.frame_t1 - self.frame_t0
        print ('process fps, fnumber, frame time: ', int(1/frametime), self.frame_num, frametime)
        self.frame_t0 = time.time()
        print ('===========')

        return pose

    def save(self, filename):

        ### save stim on and stim off times

        filename += "_savdata.npy"

        try:
            np.savez(
                filename, frame_num=np.array(self.frame_numls), frame_time=np.array(self.frame_time), pose=np.array(self.posels)
            )
            save_code = True
        except Exception:
            save_code = False

        return save_code