import sysv_ipc as ipc
import struct
import time


path = "/tmp"
key = ipc.ftok(path, 2333)
shm = ipc.SharedMemory(key, 0, 0)

# I found if we do not attach ourselves it will attach as ReadOnly.
shm.attach(0, 0)
for i in range(1000000):
    buf = shm.read(8, 24)
    buf2 = shm.read(8, 96)
    value = struct.unpack("d", buf)[0]
    value2 = struct.unpack("d", buf2)[0]
    print(value, value2)
    time.sleep(0.1)
# shm.write(struct.pack("d", 3.3))
shm.detach()
