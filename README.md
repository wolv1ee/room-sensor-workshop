# Room Sensor Network Workshop

A hands-on workshop by **Dev Club WMU**. Your kit is already wired: an ESP32 with a
temperature and humidity sensor (DHT22), a motion sensor (PIR), a small OLED screen
and an LED. You upload this code, and your readings show up live on a shared
dashboard with everyone else's.

## What you need

- A laptop (Windows, Mac or Linux)
- A USB-C **data** cable (some cables only charge)
- [Arduino IDE 2](https://www.arduino.cc/en/software) (free)

## 1. Set up Arduino IDE (do this before the workshop)

1. Install Arduino IDE 2.
2. Open **File > Preferences** and paste this into **Additional boards manager URLs**:

   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```

3. Open **Boards Manager** (left sidebar), search `esp32`, and install **esp32 by Espressif Systems**.
4. Open **Library Manager** (left sidebar) and install these. Click **Install all** if it asks about extras:
   - **DHT sensor library** by Adafruit
   - **Adafruit SSD1306**
   - **PubSubClient** by Nick O'Leary

## 2. Download the code

1. On this page, click the green **Code** button, then **Download ZIP**.
2. Unzip it.
3. In Arduino IDE, open **File > Open** and choose `room_sensor/room_sensor.ino`.

## 3. Fill in the settings

At the top of the code, replace everything that says `PASTE_...` with the values
shown on the slide in the room. Then put your name in:

```cpp
String NAME = "yourname";
```

Use lowercase letters and no spaces. This becomes your label on the dashboard.

## 4. Upload

1. Plug in the ESP32 with your USB-C cable.
2. Go to **Tools > Board > esp32 > ESP32 Dev Module**.
3. Go to **Tools > Port** and pick the port that appears when you plug the board in.
4. Click **Upload** (the right arrow button).

When it works, the screen shows **Joining Wi-Fi...**, then **Connecting to broker...**,
then your readings.

## Troubleshooting

| Problem | Fix |
|---|---|
| No port in **Tools > Port** | Try another USB-C cable, or install the [CP210x driver](https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers) |
| "Failed to connect to ESP32" | Hold the **BOOT** button on the board when the upload starts |
| Error about `DHT.h`, `Adafruit_SSD1306.h` or `PubSubClient.h` | Install that library (step 1.4) |
| Screen says **Fill in settings!** | You still have `PASTE_...` values at the top of the code |
| Stuck on **Joining Wi-Fi...** | Check the Wi-Fi name and password |
| Stuck on **Connecting to broker...** | Check the broker address. Ask a helper. |
| Screen shows `--.- C` | A wire may be loose. Ask a helper (please don't re-plug wires yourself). |

To see more detail, open **Tools > Serial Monitor** and set it to **115200**.

## Challenges

1. **Tune the alarm:** lower `HEAT_LIMIT_C`, upload, then breathe on the DHT22 until the LED blinks.
2. **Own the screen:** edit `drawScreen()` to show Fahrenheit, a custom message or a bigger motion alert.
3. **Remote control:** send `on` or `off` to `room/<name>/led` from the dashboard to switch an LED.

## How your kit is wired (for reference)

| Part | Connection |
|---|---|
| DHT22 | Data to GPIO 4, power to 3V3, ground to GND |
| PIR sensor | Out to GPIO 27, power to VIN (5V), ground to GND |
| OLED | SDA to GPIO 21, SCL to GPIO 22, power to 3V3, ground to GND |
| LED | GPIO 2 through a 220 ohm resistor to the long leg, short leg to GND |

## Topics this device publishes

| Topic | Value |
|---|---|
| `room/<name>/temp` | Temperature in Celsius |
| `room/<name>/hum` | Humidity in percent |
| `room/<name>/motion` | `1` for motion, `0` for still |
| `room/<name>/status` | `online` or `offline` |

It also listens on `room/<name>/led` for `on` or `off`.

## Join us

- Discord: https://discord.gg/wmu-dev-club
- Instagram: [@developerclubwmu](https://www.instagram.com/developerclubwmu)
- Website: https://devwmu.com
