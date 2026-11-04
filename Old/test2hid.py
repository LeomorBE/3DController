import hid
import time

VID = 0x16C0
PID = 0x0488

devices = hid.enumerate(VID, PID)

print(f"{len(devices)} HID trouvé(s)\n")

for i, dev in enumerate(devices):
    print(f"HID {i}")
    print(" Path      :", dev["path"])
    print(" Interface :", dev.get("interface_number"))
    print(" Usage Page:", dev.get("usage_page"))
    print(" Usage     :", dev.get("usage"))
    print()


if len(devices) < 2:
    print("Moins de 2 HID détectés.")
    exit()


hid1 = hid.device()
hid2 = hid.device()

hid1.open_path(devices[0]["path"])
hid2.open_path(devices[1]["path"])

hid1.set_nonblocking(1)
hid2.set_nonblocking(1)


counter1 = 0
counter2 = 0

start = time.perf_counter()

while time.perf_counter() - start < 1.0:

    d1 = hid1.read(64)
    if d1:
        counter1 += 1

    d2 = hid2.read(64)
    if d2:
        counter2 += 1


hid1.close()
hid2.close()

print()
print("HID 1 :", counter1, "buffers/s")
print("HID 2 :", counter2, "buffers/s")