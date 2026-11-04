
using UnityEngine;
using UnityEngine.InputSystem;

public class HandMovement : MonoBehaviour
{
    [Header("Cubes")]
    public Transform leftCube;
    public Transform rightCube;

    [Header("Orientation")]
    public bool invertPitch = false;
    public bool invertRoll = false;

    // false : -32767 à +32767 => -90° à +90°
    // true  : -9000 à +9000 => -90° à +90°
    public bool anglesInCentidegrees = false;

    [Header("Force / Agrandissement")]
    [Min(1)]
    public int weightMaxRaw = 65535;

    [Min(0)]
    public int weightThresholdRaw = 1000;

    [Min(1f)]
    public float maxScaleMultiplier = 2.0f;

    [Min(0f)]
    public float scaleSpeed = 8.0f;

    [Header("Debug")]
    public bool debugValues = false;

    private TeensyController teensy;

    private Vector3 leftOriginalScale;
    private Vector3 rightOriginalScale;

    private float nextDebugTime;
    private float nextSearchTime;

    void Start()
    {
        // Sauvegarder la taille originale des cubes
        if (leftCube != null)
            leftOriginalScale = leftCube.localScale;

        if (rightCube != null)
            rightOriginalScale = rightCube.localScale;

        FindTeensy();
    }

    void FindTeensy()
    {
        teensy = InputSystem.GetDevice<TeensyController>();

        if (teensy == null)
        {
            Debug.LogError("HandMovement : TeensyController NULL");

            foreach (var device in InputSystem.devices)
            {
                Debug.Log(
                    $"Device: {device.displayName} | " +
                    $"Layout: {device.layout} | " +
                    $"Type: {device.GetType().Name}"
                );
            }

            return;
        }

        Debug.Log(
            $"HandMovement : Teensy trouvé ! " +
            $"Nom={teensy.displayName} | " +
            $"Layout={teensy.layout} | " +
            $"ID={teensy.deviceId}"
        );
    }

    // Conversion de la valeur HID signée vers un angle
    float ReadAngle(int raw)
    {
        if (anglesInCentidegrees)
        {
            return Mathf.Clamp(raw / 100.0f, -90f, 90f);
        }

        float angle = raw * 90.0f / 32767.0f;

        return Mathf.Clamp(angle, -90f, 90f);
    }

    // Normalisation de la force sur [0 ; 1]
    float ReadWeightNormalized(int raw)
    {
        int minimum = Mathf.Max(0, weightThresholdRaw);
        int maximum = Mathf.Max(weightMaxRaw, minimum + 1);

        if (raw <= minimum)
            return 0f;

        return Mathf.Clamp01(
            (float)(raw - minimum) / (maximum - minimum)
        );
    }

    // Agrandissement progressif d'un cube
    void UpdateCubeScale(
        Transform cube,
        Vector3 originalScale,
        float normalizedWeight)
    {
        if (cube == null)
            return;

        // 0 => taille originale
        // 1 => taille maximale
        float multiplier = Mathf.Lerp(
            1.0f,
            maxScaleMultiplier,
            normalizedWeight
        );

        Vector3 targetScale = originalScale * multiplier;

        // Transition progressive, indépendante du framerate
        float t = 1.0f - Mathf.Exp(
            -scaleSpeed * Time.deltaTime
        );

        cube.localScale = Vector3.Lerp(
            cube.localScale,
            targetScale,
            t
        );
    }

    void Update()
    {
        // Rechercher à nouveau le Teensy si déconnecté
        if (teensy == null || !teensy.added)
        {
            if (Time.unscaledTime >= nextSearchTime)
            {
                nextSearchTime = Time.unscaledTime + 1.0f;
                FindTeensy();
            }

            // Restaurer la taille des cubes
            UpdateCubeScale(leftCube, leftOriginalScale, 0f);
            UpdateCubeScale(rightCube, rightOriginalScale, 0f);

            return;
        }

        // Lecture brute des six contrôles
        int rawPitchL = teensy.pitchLeft.ReadValue();
        int rawRollL = teensy.rollLeft.ReadValue();

        int rawPitchR = teensy.pitchRight.ReadValue();
        int rawRollR = teensy.rollRight.ReadValue();

        int rawWeightL = teensy.weightLeft.ReadValue();
        int rawWeightR = teensy.weightRight.ReadValue();
        Debug.Log("readunprocessedValue: " + teensy.weightLeft.ReadUnprocessedValue());
        Debug.Log(
                $"LEFT : rawPitch={rawPitchL}° rawRoll={rawRollL}° " +
                $"rawScale={rawWeightL} | " +
                $"RIGHT : rawPitch={rawPitchR}° rawRoll={rawRollR}° " +
                $"rawScale={rawWeightR}"
            );

        // Conversion des angles
        float pL = ReadAngle(rawPitchL);
        float rL = ReadAngle(rawRollL);

        float pR = ReadAngle(rawPitchR);
        float rR = ReadAngle(rawRollR);

        // Normalisation des forces
        float weightL = ReadWeightNormalized(rawWeightL);
        float weightR = ReadWeightNormalized(rawWeightR);

        // Inversion éventuelle
        if (invertPitch)
        {
            pL = -pL;
            pR = -pR;
        }

        if (invertRoll)
        {
            rL = -rL;
            rR = -rR;
        }

        // Orientation des deux cubes
        if (leftCube != null)
        {
            leftCube.localRotation =
                Quaternion.Euler(pL, 0f, rL);
        }

        if (rightCube != null)
        {
            rightCube.localRotation =
                Quaternion.Euler(pR, 0f, rR);
        }

        // Agrandissement proportionnel à la force
        UpdateCubeScale(
            leftCube,
            leftOriginalScale,
            weightL
        );

        UpdateCubeScale(
            rightCube,
            rightOriginalScale,
            weightR
        );

        // Diagnostic quatre fois par seconde
        if (debugValues && Time.unscaledTime >= nextDebugTime)
        {
            nextDebugTime = Time.unscaledTime + 0.25f;

            Debug.Log(
                $"LEFT : Pitch={pL:F1}° Roll={rL:F1}° " +
                $"WeightRaw={rawWeightL} Scale={weightL:F3} | " +
                $"RIGHT : Pitch={pR:F1}° Roll={rR:F1}° " +
                $"WeightRaw={rawWeightR} Scale={weightR:F3}"
            );
        }
    }
}
