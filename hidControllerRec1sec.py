
import hid
import time
import struct
from pynput import keyboard

VID = 0x16C0
PID = 0x0488

USAGE_PAGE = 0x01
USAGE = 0x08

REPORT_SIZE = 12  # 6 valeurs de 16 bits

running = True


def on_press(key):
    global running

    if key == keyboard.Key.esc:
        print("\nFermeture demandée...")
        running = False
        return False


# Rechercher spécifiquement notre interface Controller
print("Recherche du Teensy Controller...")

devices = hid.enumerate(VID, PID)

controller = None

for device in devices:
    print(
        f"Interface={device.get('interface_number')} "
        f"UsagePage={device.get('usage_page')} "
        f"Usage={device.get('usage')} "
        f"Product={device.get('product_string')}"
    )

    if (
        device.get("usage_page") == USAGE_PAGE
        and device.get("usage") == USAGE
    ):
        controller = device
        break

if controller is None:
    raise RuntimeError("Teensy Multi-Axis Controller introuvable !")


h = hid.device()
listener = None

try:
    # Ouvrir le chemin exact de l'interface HID
    h.open_path(controller["path"])

    print("\nPériphérique connecté :")
    print("Manufacturer:", h.get_manufacturer_string())
    print("Product:", h.get_product_string())
    print("Serial No:", h.get_serial_number_string())

    # Lecture avec timeout de 100 ms
    h.set_nonblocking(0)

    listener = keyboard.Listener(on_press=on_press)
    listener.start()

    print("\nLecture des 6 valeurs HID (Échap pour quitter)\n")

    counter = 0
    start_time = time.perf_counter()

    while running:

        # Windows/hidapi renvoie normalement 12 octets
        # utiles pour ce rapport sans Report ID explicite.
        data = h.read(REPORT_SIZE, 100)

        if data:
            if len(data) != REPORT_SIZE:
                print(f"Rapport inattendu : {len(data)} octets : {data}")
                continue

            # 4 entiers signés + 2 entiers non signés
            (
                pitch_left,
                roll_left,
                pitch_right,
                roll_right,
                weight_left,
                weight_right
            ) = struct.unpack("<hhhhHH", bytes(data))

            counter += 1

            print(
                f"Pitch L: {pitch_left:7d} | "
                f"Roll L: {roll_left:7d} | "
                f"Pitch R: {pitch_right:7d} | "
                f"Roll R: {roll_right:7d} | "
                f"Weight L: {weight_left:5d} | "
                f"Weight R: {weight_right:5d}",
                end="\r",
                flush=True
            )

        # Calcul de la fréquence de réception
        elapsed = time.perf_counter() - start_time

        if elapsed >= 1.0:
            frequency = counter / elapsed

            print(
                f"\nFréquence de réception : {frequency:.1f} Hz "
                f"({counter} rapports en {elapsed:.2f} s)"
            )

            counter = 0
            start_time = time.perf_counter()

except OSError as e:
    print("\nErreur HID :", e)

except KeyboardInterrupt:
    print("\nInterruption clavier.")

finally:
    running = False
    h.close()

    if listener is not None:
        listener.stop()

    print("\nPériphérique fermé.")
