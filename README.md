# 🤖 Robot Mochi

Robot mini berbasis ESP32-C3 yang bisa dikontrol dari HP via WiFi, dilengkapi ekspresi wajah di layar OLED.

---

## Komponen

- ESP32-C3
- 2x Servo 360°
- OLED 0.96" (I2C)
- Step-Up Boost 3.7V → 5V
- Modul Charge TP4056
- Baterai Li-Ion 3.7V 1200mAh

---

## Fitur

- Kontrol gerak (maju, mundur, belok) dari browser HP
- Ekspresi wajah imut & marah berganti tiap 5 detik di OLED
- Sistem daya portable dengan baterai

---

## Koneksi Pin

| Pin ESP32-C3 | Komponen |
|-------------|----------|
| GPIO4 | Servo Kiri |
| GPIO5 | Servo Kanan |
| GPIO6 | OLED SDA |
| GPIO7 | OLED SCL |
| VIN | Step-Up 5V |

---

## Cara Pakai

1. Ganti WiFi di kode: `ssid` dan `password`
2. Upload ke ESP32-C3 via Arduino IDE
3. Lihat IP address di Serial Monitor
4. Buka IP tersebut di browser HP
5. Kontrol robot dari web controller

---

## Library

- Adafruit SSD1306
- Adafruit GFX
- ESP32Servo

---

Made by **Ziker** 🚀
