import struct


class helper:
    def __init__(self, shm, keys, vector):
        self.shm = shm
        self.keys = keys
        self.vector = vector  # identifies which xh vector is being used.

    def read_xh(self):
        indexes = [
            self.keys[key]
            for key in self.keys.keys()
            if key.startswith(f"xh_{self.vector}_")
        ]
        return [struct.unpack("d", self.shm.read(8, index * 8))[0] for index in indexes]

    def read_servo(self):
        indexes = [
            self.keys[key]
            for key in self.keys.keys()
            if key.startswith(f"servo_CHANNEL_")
        ]
        return [struct.unpack("d", self.shm.read(8, index * 8))[0] for index in indexes]

    def write_controller(self, values):
        indexes = [
            self.keys[key]
            for key in self.keys.keys()
            if key.startswith("controller_1_")
        ]
        if len(values) != len(indexes):
            print(
                f"ERROR [CONTROLLER_1]: Invalid number of values provided to write to controller_1 vector.\nExpected {len(indexes)} values, but received {len(values)}."
            )
            return
        for i in range(len(indexes)):
            self.shm.write(struct.pack("d", values[i]), indexes[i] * 8)
