
using UnityEngine;
using UnityEngine.InputSystem;
using UnityEngine.InputSystem.Controls;
using UnityEngine.InputSystem.Layouts;
using UnityEngine.InputSystem.LowLevel;
using UnityEngine.InputSystem.Utilities;

using System.Runtime.InteropServices;

#if UNITY_EDITOR
using UnityEditor;
#endif

[StructLayout(LayoutKind.Explicit, Size = 13)]
public struct TeensyControllerState : IInputStateTypeInfo
{
    public FourCC format => new FourCC('H', 'I', 'D');

    // Octet réservé au Report ID dans l'état HID Unity
    [FieldOffset(0)]
    public byte reportId;

    [InputControl(name = "pitchLeft", layout = "SHRT")]
    [FieldOffset(1)]
    public short pitchLeft;

    [InputControl(name = "rollLeft", layout = "SHRT")]
    [FieldOffset(3)]
    public short rollLeft;

    [InputControl(name = "pitchRight", layout = "SHRT")]
    [FieldOffset(5)]
    public short pitchRight;

    [InputControl(name = "rollRight", layout = "SHRT")]
    [FieldOffset(7)]
    public short rollRight;

    [InputControl(name = "weightLeft", layout = "USHT")]
    [FieldOffset(9)]
    public ushort weightLeft;

    [InputControl(name = "weightRight", layout = "USHT")]
    [FieldOffset(11)]
    public ushort weightRight;
}

[InputControlLayout(
    displayName = "Teensy Controller",
    stateType = typeof(TeensyControllerState)
)]
public class TeensyController : InputDevice
{
    public IntegerControl pitchLeft { get; private set; }
    public IntegerControl rollLeft { get; private set; }

    public IntegerControl pitchRight { get; private set; }
    public IntegerControl rollRight { get; private set; }

    public IntegerControl weightLeft { get; private set; }
    public IntegerControl weightRight { get; private set; }

    protected override void FinishSetup()
    {
        base.FinishSetup();

        pitchLeft = GetChildControl<IntegerControl>("pitchLeft");
        rollLeft = GetChildControl<IntegerControl>("rollLeft");

        pitchRight = GetChildControl<IntegerControl>("pitchRight");
        rollRight = GetChildControl<IntegerControl>("rollRight");

        weightLeft = GetChildControl<IntegerControl>("weightLeft");
        weightRight = GetChildControl<IntegerControl>("weightRight");
    }

    private static void Register()
    {
        InputSystem.RegisterLayout<TeensyController>(
            matches: new InputDeviceMatcher()
                .WithInterface("HID")
                .WithProduct("Teensy Controller")
                .WithCapability("vendorId", 0x16C0)
                .WithCapability("productId", 0x0488)
                .WithCapability("usagePage", 0x01)
                .WithCapability("usage", 0x08)
        );
    }

#if UNITY_EDITOR
    [InitializeOnLoadMethod]
    private static void RegisterEditor()
    {
        Register();
    }
#endif

    [RuntimeInitializeOnLoadMethod(
        RuntimeInitializeLoadType.BeforeSceneLoad)]
    private static void RegisterRuntime()
    {
        Register();
    }
}
