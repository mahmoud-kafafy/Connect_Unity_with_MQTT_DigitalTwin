
# 🚀 Unity + MQTT Conveyor & Sensor System

This project integrates **Unity** with an **MQTT broker** (HiveMQ Cloud in our case) to:

* Control a **conveyor belt** with MQTT messages.
* Detect objects with **sensors** and publish MQTT events.
* Safely bridge **MQTT background threads** into **Unity’s main thread** using a **dispatcher**.

---

## 📌 Features

* Conveyor control via MQTT (`fwd`, `back`, `stop`).
* Sensors that light up LEDs and publish messages (`sensor1:on`, `sensor1:off`).
* **Thread-safe bridge** so Unity updates always happen on the main thread.
* Status messages published back to MQTT (e.g., `"moving forward"`, `"stopped"`).

---

## ⚠️ The Core Problem

Unity is **not thread-safe**.
Any call that changes a `GameObject`, `Transform`, `Renderer`, `Rigidbody`, etc. must be executed on Unity’s **main thread**.

* **Manual button clicks** worked fine (already on Unity main thread).
* **MQTT callbacks** run on a **background thread** (managed by the MQTTnet library).

So when we tried this:

```csharp
_client.UseApplicationMessageReceivedHandler(e =>
{
    if (payload == "fwd")
        conveyor.MoveForward();  // ❌ called from MQTT thread, Unity ignored it
});
```

Unity silently ignored the command.
The logs appeared, but nothing in the scene updated.

---

## ✅ The Fix: UnityMainThreadDispatcher

We added a **dispatcher** script that queues actions and executes them in Unity’s `Update()` loop.

### Dispatcher Code

```csharp
using System;
using System.Collections.Concurrent;
using UnityEngine;

public class UnityMainThreadDispatcher : MonoBehaviour
{
    private static readonly ConcurrentQueue<Action> _actions = new ConcurrentQueue<Action>();
    private static UnityMainThreadDispatcher _instance;

    void Awake()
    {
        _instance = this;
    }

    public static void Enqueue(Action action)
    {
        if (_instance == null)
        {
            Debug.LogError("UnityMainThreadDispatcher not present in scene!");
            return;
        }
        _actions.Enqueue(action);
    }

    void Update()
    {
        while (_actions.TryDequeue(out var action))
        {
            action?.Invoke();
        }
    }
}
```

---

## 🛠️ How to Add the Dispatcher in Unity

1. Create a new script in Unity called **`UnityMainThreadDispatcher.cs`**.

   * Paste the code above inside.
   * Make sure the file name and class name match.

2. Add the script to an **empty GameObject** in your scene (e.g., name it `Dispatcher`).

   * This ensures its `Update()` runs every frame.

3. Now, any background thread (like MQTT) can safely update Unity by calling:

   ```csharp
   UnityMainThreadDispatcher.Enqueue(() => { /* Unity code */ });
   ```

---

## 🔄 Modify MQTT Handler (HiveMQ.cs)

Inside `MqttConveyorController` (your HiveMQ client), replace the **message handler** with this version:

```csharp
_client.UseApplicationMessageReceivedHandler(e =>
{
    string topic = e.ApplicationMessage.Topic;
    string payload = Encoding.UTF8.GetString(e.ApplicationMessage.Payload).Trim().ToLower();

    Debug.Log($"[MQTT RAW] '{payload}' (length={payload.Length})");

    if (topic == controlTopic && conveyor != null)
    {
        // ✅ Enqueue Unity work onto main thread
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
```

---

## 📡 Topics Used
# You can change the names of topics in the code
* **Control Topic** → `unity/conveyor/control`

  * `fwd` → move forward
  * `back` → move backward
  * `stop` → stop

* **Status Topic** → `unity/conveyor/status`

  * Publishes `"moving forward"`, `"moving backward"`, `"stopped"`, or `"unknown command"`.

* **Sensor Topic** → `unity/sensors`

  * Each sensor publishes its state, e.g.:

    * `"sensor1:on"`
    * `"sensor2:off"`

---


## 🟢 All Setup Steps

1. **Clone the repo**

   ```bash
   https://github.com/mahmoud-kafafy/Connect_Unity_with_MQTT_DIigitalTwin.git
   ```
   Or download it manually

   Open the project in **Unity Hub** or **Unity Editor**.

2. **Scene Setup**

   * Open or create a new Unity scene.
   * All required scripts are already included in this repo:

     * `ConveyorBelt.cs`
     * `MqttConveyorController.cs`
     * `UnityMainThreadDispatcher.cs`
     * `SensorLED.cs`

3. **Add GameObjects & Attach Scripts**

   * Create a **Conveyor Belt** GameObject → attach `ConveyorBelt.cs`.
   * Create a **GameObject** (e.g., `MQTTClient`) → attach `MqttConveyorController.cs`.

     * In the Inspector, configure broker `host`, `port`, `username`, and `password`.
   * Create an empty GameObject called **Dispatcher** → attach `UnityMainThreadDispatcher.cs`.
   * For each sensor in your scene:

     * Add the `SensorLED.cs` component.
     * Assign LED materials (`ledOnMaterial`, `ledOffMaterial`).
     * Give each sensor a unique `sensorId`.

4. **Run the Scene**

   * Press ▶ Run in Unity.
   * Send MQTT messages to control the conveyor via the topic:

     * `unity/conveyor/control` with payloads `fwd`, `back`, or `stop`.
   * Watch Unity publish status updates to:

     * `unity/conveyor/status`.
   * Trigger a sensor in Unity and see it publish MQTT events:

     * `"sensorX:on"` when an object enters.
     * `"sensorX:off"` when an object exits.


---

## 📊 Threading Analogy

Unity = chef 👨‍🍳

* Buttons = you tell the chef directly → he cooks.
* MQTT = a waiter calls on the phone → waiter can’t cook, only write order.
* Dispatcher = chef checks order queue every frame → then cooks safely.

---

## 🔮 Future Work

* Add more conveyor controls (speed, emergency stop).
* Display MQTT messages on a Unity UI panel.
* Support multiple conveyors.


