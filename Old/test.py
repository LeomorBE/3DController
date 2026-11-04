
import hid
import time
import struct

VID = 0x16C0
PID = 0x0488

DURATION = 10.0

AXES = [
    "Pitch Left",
    "Roll Left",
    "Pitch Right",
    "Roll Right",
    "Slider Left",
    "Slider Right"
]

def main():
    device = hid.device()

    try:
        device.open(VID, PID)
        device.set_nonblocking(1)

        print("Teensy connecté")
        print("Fabricant :", device.get_manufacturer_string())
        print("Produit   :", device.get_product_string())
        print("\nLecture pendant 10 secondes...\n")

        start = time.monotonic()
        counter = 0

        while time.monotonic() - start < DURATION:
            data = device.read(64)

            if not data:
                time.sleep(0.001)
                continue

            counter += 1

            # Certains rapports incluent un Report ID.
            # Ici on suppose 1 octet d'identifiant,
            # suivi de 6 valeurs uint16 little-endian.
            if len(data) < 13:
                print("Rapport brut :", data)
                continue

            values = struct.unpack_from(
                "<6H", bytes(data), 1
            )

            print("\033[H\033[J", end="")

            print("=== TEENSY HID ===")
            print(f"Temps : {time.monotonic() - start:.2f} / 10.00 s")
            print()

            for name, value in zip(AXES, values):
                print(f"{name:<15} : {value:5d}")

            print()
            print(f"Rapports reçus : {counter}")

        elapsed = time.monotonic() - start

        print("\n=== FIN DU TEST ===")
        print(f"Durée : {elapsed:.2f} s")
        print(f"Rapports : {counter}")
        print(f"Fréquence moyenne : {counter / elapsed:.1f} Hz")

    except OSError as error:
        print("Erreur HID :", error)

    finally:
        device.close()
        print("Périphérique fermé.")


if __name__ == "__main__":
    main()

