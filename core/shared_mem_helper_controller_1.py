import struct
import time


class channel:
    def __init__(self, name):
        self.name = name

    def __set__(self, instance, value):
        c = instance.channels[self.name]
        instance[c] = value

    def __get__(self, instance, owner):
        c = instance.channels[self.name]
        return instance[c]


class channel_vector(list):
    throttle = channel("THROTTLE")
    elevator = channel("ELEVATOR")
    aileron = channel("AILERON")
    rudder = channel("RUDDER")
    flaps = channel("FLAPS")

    def __init__(self, channels):
        self.channels = channels

    def __getitem__(self, key):
        return super(channel_vector, self).__getitem__(key)


class helper:
    def __init__(self, shm, keys, vector, channels):
        self.shm = shm
        self.keys = keys
        self.vector = vector  # identifies which xh vector is being used.
        self.rcin = channel_vector(channels)
        self.servos = channel_vector(channels)
        self.updates = 0

    def read_xh(self):
        indexes = [
            self.keys[key]
            for key in self.keys.keys()
            if key.startswith(f"xh_{self.vector}_")
        ]
        while True:
            xh = [
                struct.unpack("d", self.shm.read(8, index * 8))[0] for index in indexes
            ]
            if xh[0] == self.updates:
                time.sleep(0.001)  # wait if no new data
                continue
            self.updates = xh[0]
            return xh[1:]

    def read_rcin(self):
        indexes = [
            self.keys[key]
            for key in self.keys.keys()
            if key.startswith(f"rcin_CHANNEL_")
        ]
        self.rcin[:] = [
            struct.unpack("d", self.shm.read(8, index * 8))[0] for index in indexes
        ]
        return self.rcin

    def read_servos(self):
        indexes = [
            self.keys[key]
            for key in self.keys.keys()
            if key.startswith(f"servo_CHANNEL_")
        ]
        self.servos[:] = [
            struct.unpack("d", self.shm.read(8, index * 8))[0] for index in indexes
        ]
        return self.servos

    def write_controller_1(self):
        indexes = [
            self.keys[key]
            for key in self.keys.keys()
            if key.startswith("controller_1_")
        ]
        for i in range(len(indexes)):
            self.shm.write(struct.pack("d", self.servos[i]), indexes[i] * 8)
