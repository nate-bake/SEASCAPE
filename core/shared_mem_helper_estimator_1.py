import struct


class helper:
    def __init__(self, shm, keys):
        self.shm = shm
        self.keys = keys

    def read_y(self):
        indexes = [self.keys[key] for key in self.keys.keys() if key.startswith("y_")]
        return [struct.unpack("d", self.shm.read(8, index * 8))[0] for index in indexes]

    def read_xh(self):
        indexes = [
            self.keys[key] for key in self.keys.keys() if key.startswith("xh_1_")
        ]
        return [struct.unpack("d", self.shm.read(8, index * 8))[0] for index in indexes]

    def write_xh(self, values):
        indexes = [
            self.keys[key] for key in self.keys.keys() if key.startswith("xh_1_")
        ]
        if len(values) != len(indexes):
            print(
                f"ERROR [ESTIMATOR_1]: Invalid number of values provided to write to xh_1 vector.\nExpected {len(indexes)} values, but received {len(values)}."
            )
            return
        for i in range(len(indexes)):
            self.shm.write(struct.pack("d", values[i]), indexes[i] * 8)
