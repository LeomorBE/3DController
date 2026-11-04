using UnityEngine;
using UnityEngine.InputSystem;
using UnityEngine.InputSystem.Controls;

public class JoystickFullInputDebug : MonoBehaviour
{
    public bool showOnlyChangingValues = false;
    public float changeThreshold = 0.001f;
    public float printInterval = 0.25f;

    private float nextPrintTime = 0f;

    void Update()
    {
        if (Time.time < nextPrintTime)
            return;

        nextPrintTime = Time.time + printInterval;

        foreach (var device in InputSystem.devices)
        {
            if (device is Joystick joystick)
            {
                Debug.Log("========== JOYSTICK ==========");
                Debug.Log("Name: " + joystick.name);
                Debug.Log("DisplayName: " + joystick.displayName);
                Debug.Log("Layout: " + joystick.layout);
                Debug.Log("Device path: " + joystick.path);

                foreach (InputControl control in joystick.allControls)
                {
                    if (control is AxisControl axis)
                    {
                        float value = axis.ReadValue();

                        if (showOnlyChangingValues && Mathf.Abs(value) < changeThreshold)
                            continue;

                        Debug.Log(
                            "AXIS"
                            + " | path: " + control.path
                            + " | name: " + control.name
                            + " | display: " + control.displayName
                            + " | value: " + value.ToString("F6")
                        );
                    }
                    else if (control is ButtonControl button)
                    {
                        float value = button.ReadValue();

                        if (showOnlyChangingValues && value < changeThreshold)
                            continue;

                        Debug.Log(
                            "BUTTON"
                            + " | path: " + control.path
                            + " | name: " + control.name
                            + " | display: " + control.displayName
                            + " | value: " + value.ToString("F6")
                        );
                    }
                }
            }
        }
    }
}