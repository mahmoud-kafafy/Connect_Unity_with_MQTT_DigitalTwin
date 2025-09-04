// using UnityEngine;

// public class SensorLED : MonoBehaviour
// {
//     public Renderer ledRenderer;     // drag the LED sphere here
//     public Material ledOnMaterial;   // bright material
//     public Material ledOffMaterial;  // dark material

//     private void Start()
//     {
//         if (ledRenderer != null && ledOffMaterial != null)
//         {
//             ledRenderer.material = ledOffMaterial; // start off
//         }
//     }

//     private void OnTriggerEnter(Collider other)
//     {
//         if (other.attachedRigidbody != null) // only respond to objects with Rigidbody (like your cube)
//         {
//             Debug.Log($"{gameObject.name} triggered ON by {other.name}");
//             if (ledRenderer != null && ledOnMaterial != null)
//                 ledRenderer.material = ledOnMaterial;
//         }
//     }

//     private void OnTriggerExit(Collider other)
//     {
//         if (other.attachedRigidbody != null)
//         {
//             Debug.Log($"{gameObject.name} triggered OFF by {other.name}");
//             if (ledRenderer != null && ledOffMaterial != null)
//                 ledRenderer.material = ledOffMaterial;
//         }
//     }
// }


using UnityEngine;

public class SensorLED : MonoBehaviour
{
    public Renderer ledRenderer;
    public Material ledOnMaterial;
    public Material ledOffMaterial;

    [Header("MQTT Settings")]
    public string topic = "unity/sensors"; // you can set different topic per sensor
    public string sensorId = "sensor1";    // make this unique for each sensor

    private MqttConveyorController mqttController;

    private void Start()
    {
        if (ledRenderer != null && ledOffMaterial != null)
            ledRenderer.material = ledOffMaterial;

        // find the MQTT controller in the scene
        mqttController = FindObjectOfType<MqttConveyorController>();
    }

    private async void OnTriggerEnter(Collider other)
    {
        if (other.attachedRigidbody != null)
        {
            Debug.Log($"{gameObject.name} triggered ON by {other.name}");
            if (ledRenderer != null && ledOnMaterial != null)
                ledRenderer.material = ledOnMaterial;

            if (mqttController != null)
                await mqttController.PublishMessage(topic, $"{sensorId}:on");
        }
    }

    private async void OnTriggerExit(Collider other)
    {
        if (other.attachedRigidbody != null)
        {
            Debug.Log($"{gameObject.name} triggered OFF by {other.name}");
            if (ledRenderer != null && ledOffMaterial != null)
                ledRenderer.material = ledOffMaterial;

            if (mqttController != null)
                await mqttController.PublishMessage(topic, $"{sensorId}:off");
        }
    }
}

