using System;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using MQTTnet;
using MQTTnet.Client;
using MQTTnet.Client.Options;
using UnityEngine;

public class MqttConveyorController : MonoBehaviour
{
    public ConveyorBelt conveyor;  // assign in Inspector
    public string host = "f446739580a645d9b26dc12166f78ed8.s1.eu.hivemq.cloud";
    public int port = 8883;
    public string username = "Mahmoud";
    public string password = "123456789mM";

    public string controlTopic = "unity/conveyor/control";
    public string statusTopic = "unity/conveyor/status";  // <-- NEW: for publishing status

    private IMqttClient _client;

    async void Start()
    {
        var factory = new MqttFactory();
        _client = factory.CreateMqttClient();

        _client.UseConnectedHandler(async e =>
        {
            Debug.Log("[MQTT] Connected to broker");
            await _client.SubscribeAsync(controlTopic);
            Debug.Log("[MQTT] Subscribed to " + controlTopic);
        });

        _client.UseApplicationMessageReceivedHandler(e =>
        {
            string topic = e.ApplicationMessage.Topic;
            string payload = Encoding.UTF8.GetString(e.ApplicationMessage.Payload).Trim().ToLower();

            Debug.Log($"[MQTT RAW] '{payload}' (length={payload.Length})");

            if (topic == controlTopic && conveyor != null)
            {
                // Enqueue on Unity's main thread
                UnityMainThreadDispatcher.Enqueue(async () =>
                {
                    Debug.Log("Na sm3t topic abl conditions (main thread)");

                    if (payload == "stop")
                    {
                        conveyor.Stop();
                        await PublishMessage(statusTopic, "stopped");
                        Debug.Log("[MQTT] Conveyor stopped via MQTT");
                    }
                    else if (payload == "fwd")
                    {
                        conveyor.MoveForward();
                        await PublishMessage(statusTopic, "moving forward");
                        Debug.Log("[MQTT] Conveyor moving forward via MQTT");
                    }
                    else if (payload == "back")
                    {
                        conveyor.MoveBackward();
                        await PublishMessage(statusTopic, "moving backward");
                        Debug.Log("[MQTT] Conveyor moving backward via MQTT");
                    }
                    else
                    {
                        Debug.LogWarning("[MQTT] Unknown command: " + payload);
                        await PublishMessage(statusTopic, "unknown command: " + payload);
                    }
                });
            }
        });

        var options = new MqttClientOptionsBuilder()
            .WithClientId("unity-conveyor-" + Guid.NewGuid())
            .WithTcpServer(host, port)
            .WithCredentials(username, password)
            .WithCleanSession()
            .WithTls(new MqttClientOptionsBuilderTlsParameters
            {
                UseTls = true,
                AllowUntrustedCertificates = true,
                IgnoreCertificateChainErrors = true,
                IgnoreCertificateRevocationErrors = true
            })
            .Build();

        await _client.ConnectAsync(options, CancellationToken.None);
    }

    // Helper method to publish messages
   public async Task PublishMessage(string topic, string message)
    {
        if (_client != null && _client.IsConnected)
        {
            var mqttMessage = new MqttApplicationMessageBuilder()
                .WithTopic(topic)
                .WithPayload(message)
                .WithExactlyOnceQoS()
                .WithRetainFlag(false)
                .Build();

            await _client.PublishAsync(mqttMessage);
            Debug.Log($"[MQTT] Published: {topic} -> {message}");
        }
        else
        {
            Debug.LogWarning("[MQTT] Client not connected, cannot publish");
        }
    }


}
