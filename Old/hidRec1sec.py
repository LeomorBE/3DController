from pynput import keyboard
import hid
import time
print(hid.enumerate())

vid = 0x16c0	# Change it for your device
pid = 0x0488	# Change it for your device

h = hid.device()
h.open(vid, pid)  # TREZOR VendorID/ProductID
counter=0

print("Manufacturer: %s" % h.get_manufacturer_string())
print("Product: %s" % h.get_product_string())
print("Serial No: %s" % h.get_serial_number_string())

# enable non-blocking mode
h.set_nonblocking(1)

def on_press(key):
    try:
        if key == keyboard.Key.esc:  # Vous pouvez changer la touche ici
            print("Touche pressée - fermeture du device")
            h.close()
            return False  # Arrête le listener
    except AttributeError:
        pass

# Créer et démarrer le listener
listener = keyboard.Listener(on_press=on_press)
listener.start()
print("Read the data")
start_time = time.time()  # Temps de début
duration = 1.0  # 1 seconde

while True:
    d = h.read(64)
    if d:
        print(d)
        counter+=1
    
    # Vérifier si 1 seconde s'est écoulée
    if time.time() - start_time >= duration:
        print("1 seconde écoulée")
        print(str(counter)+" buffer reçus donc "+str(counter)+"Hz")
        h.close()
        break
        
    # Vérifier si l'utilisateur a appuyé sur Echap
    if not listener.running:
        break

print("Closing the device")
h.close()