
using UnityEngine;
using UnityEngine.InputSystem;
using UnityEngine.InputSystem.Controls;
using System.Text;

public class HIDDebugger : MonoBehaviour
{
    private InputDevice teensy;
    private float nextLogTime;

    void Start()
    {
        // Afficher tous les périphériques détectés
        foreach (var device in InputSystem.devices)
        {
            Debug.Log(
                $"DEVICE: {device.displayName} | " +
                $"Layout: {device.layout} | " +
                $"Interface: {device.description.interfaceName} | " +
                $"Capabilities: {device.description.capabilities}"
            );

            // Recherche du Teensy VID=0x16C0, PID=0x0488
            if (device.description.interfaceName == "HID" &&
                device.description.capabilities.Contains(
                    "\"vendorId\":5824") &&
                device.description.capabilities.Contains(
                    "\"productId\":1160"))
            {
                teensy = device;
                Debug.Log("TEENSY IDENTIFIÉ !");
            }
        }

        if (teensy == null)
        {
            Debug.LogWarning("Teensy HID non identifié !");
        }
    }

    void Update()
    {
        if (teensy == null || !teensy.added)
            return;

        // Affichage toutes les 0,5 secondes
        if (Time.unscaledTime < nextLogTime)
            return;

        nextLogTime = Time.unscaledTime + 0.5f;

        StringBuilder sb = new StringBuilder();
        sb.AppendLine("=== TEENSY HID ===");

        foreach (var control in teensy.allControls)
        {
            if (control is AxisControl axis)
            {
                sb.AppendLine(
                    $"{axis.path} = {axis.ReadValue():F4}"
                );
            }
            else if (control is ButtonControl button)
            {
                sb.AppendLine(
                    $"{button.path} = {button.ReadValue():F0}"
                );
            }
        }

        Debug.Log(sb.ToString());
    }
}
