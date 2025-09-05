# 🚀 ESP32 MQTT IR Sensor with HiveMQ Cloud

This project connects an **ESP32** to **HiveMQ Cloud MQTT broker** over a **secure TLS connection**.
The ESP32 reads an **IR sensor** and publishes `"fwd"` to MQTT when the sensor is triggered.

---

## 📌 Features

* ✅ Wi-Fi STA mode connection
* ✅ Secure MQTT (`mqtts://`) with TLS using HiveMQ Cloud Root CA
* ✅ Automatic Wi-Fi & MQTT reconnect
* ✅ Reads IR sensor (GPIO 4 by default)
* ✅ Publishes `"fwd"` message to topic `unity/conveyor/control`

---

## 🛠️ Requirements

* [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) (v5.x recommended)
* ESP32 Dev Board
* A [HiveMQ Cloud](https://www.hivemq.com/mqtt-cloud-broker/) account with:

  * A **TLS-enabled cluster** (required for secure MQTT over port `8883`)
  * MQTT **username/password** (created in the HiveMQ Cloud Console)
  * HiveMQ **Cloud endpoint** (e.g., `xxxxxx.s1.eu.hivemq.cloud`)
  * HiveMQ **Root CA Certificate (ISRG Root X1)** for TLS authentication


---

## 📂 Project Setup
## If you are on windows please make all commands on esp-idf cmd

### 1️⃣ Create a new ESP-IDF project

```bash
idf.py create-project mqtt_ir_sensor
cd mqtt_ir_sensor
```
then paste the codes in repo in the main file in the project

---

### 2️⃣ Set up ESP-IDF environment

Go to where you install esp-idf and run export.sh in linux and export.bat for windows
you mostly will find at `esp_idf\Espressif\frameworks\esp-idf-v5.5`
#### On **Linux/macOS**:
```bash
./export.sh
```

#### On **Windows (cmd of esp-idf)**:

```powershell
.\export.bat
```

---

### 3️⃣ Configure Wi-Fi & MQTT credentials
Put any of mains in repo in your main of project
Edit `main/mqtt_ir_sensor.c`:

```c
#define WIFI_SSID "YourWiFi"
#define WIFI_PASS "YourPassword"
#define MQTT_BROKER "mqtts://<your-hivemq-host>.s1.eu.hivemq.cloud:8883"
#define MQTT_USERNAME "YourHiveMQUser"
#define MQTT_PASSWORD "YourHiveMQPass"
#define IR_SENSOR_GPIO GPIO_NUM_4
```

### 4️⃣ Get the HiveMQ Cloud Root CA Certificate

HiveMQ Cloud uses **ISRG Root X1** (Let’s Encrypt).
You must download it and paste into `mqtt_ir_sensor.c`.

#### Option 1: Download from Let’s Encrypt

* Open: [https://letsencrypt.org/certs/isrgrootx1.pem](https://letsencrypt.org/certs/isrgrootx1.pem)
* Copy everything between:

  ```
  -----BEGIN CERTIFICATE-----
  ...
  -----END CERTIFICATE-----
  ```
* Paste into your code:

  ```c
  static const char *mqtt_ca_cert = "-----BEGIN CERTIFICATE-----\n"
  "... certificate contents ..."
  "-----END CERTIFICATE-----\n";
  ```

#### Option 2: Export from HiveMQ Cloud

* Log in to [HiveMQ Cloud Console](https://console.hivemq.cloud)
* Go to **Cluster Details → Connection Information**
* Copy the **CA Certificate** shown

⚠️ If you don’t add this certificate, ESP32 will fail with:

```
mbedtls_x509_crt_parse returned -0x112C
ESP_ERR_MBEDTLS_X509_CRT_PARSE_FAILED
```

---

---

### 5️⃣ Build the project

```bash
idf.py build
```

---

### 6️⃣ Flash to ESP32

#### On **Linux/macOS**:
To know the tty of your esp 

```bash
ls /dev/ttyUSB*
```
Then

```bash
idf.py -p /dev/ttyUSB0 flash
```

#### On **Windows**:

Find your COM port in *Device Manager → Ports (COM & LPT)*, e.g., `COM5`:

```powershell
idf.py -p COM5 flash
```

---

### 7️⃣ Monitor logs

After flashing, view logs:

```bash
idf.py -p /dev/ttyUSB0 monitor   # Linux/macOS
```
```bash
idf.py -p COM5 monitor           # Windows
```

Or flash & monitor in one step:

```bash
idf.py -p COM5 flash monitor
```

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

---

## 📡 How It Works

1. ESP32 connects to your Wi-Fi.
2. A **secure TLS session** is established with HiveMQ Cloud.
3. On IR sensor detection (`GPIO 4 → LOW`), ESP32 publishes:

```
Topic: unity/conveyor/control
Message: "fwd"
```

4. You can subscribe with any MQTT client (MQTT Explorer, HiveMQ Web Client, etc.) to see the messages.

---

## 📝 Example Log Output

```
Wi-Fi connected and IP received.
✅ Connected to MQTT broker.
📡 IR detected → Sent 'fwd'
📡 IR detected → Sent 'fwd'
```

---

## 🛠️ Troubleshooting

* **`mbedtls_x509_crt_parse failed`** → Wrong/missing CA certificate. Paste the correct **ISRG Root X1**.
* **Wi-Fi stuck** → Check SSID/PASSWORD and router frequency (ESP32 supports 2.4GHz, not 5GHz).
* **No MQTT messages** → Ensure correct broker URI, username, and password.


