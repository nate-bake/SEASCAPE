import struct


class shared_memory_helper_controller_1:
    def __init__(self, shm, keys):
        self.shm = shm
        self.keys = keys

    def read(self, key):
        index = self.keys[key]
        return struct.unpack("d", self.shm.read(8, index * 8))[0]

    def write(self, key, value):
        if key not in self.keys:
            print(
                f"CONTROLLER_1 tried to write to unknown key '{key}'. Skipping write."
            )
            return
        if not key.startswith(f"CONTROLLER_1_"):
            print(f"CONTROLLER_1 not allowed to write to key '{key}'. Skipping write.")
            return
        index = self.keys[key]
        self.shm.write(struct.pack("d", value), index * 8)
