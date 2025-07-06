
# 🌬️ Breezy - Monitor de Calitate a Aerului

**Breezy Air** este un dispozitiv IoT portabil creat pentru a monitoriza calitatea aerului în spații interioare, oferind utilizatorului informații în timp real despre parametrii esențiali ai atmosferei și avertismente sonore/luminoase în situații critice.

> Proiect educațional dezvoltat în clasa a IX-a, cu scopul de a promova sănătatea respiratorie și utilizarea tehnologiilor embedded în viața de zi cu zi.

---

## 📦 Funcționalități principale

- 📊 Măsurare: temperatură, umiditate, presiune atmosferică, concentrație de gaze (VOC), praf
- 📡 Transmisie date către platforma IoT ThingSpeak
- 🔔 Alarme sonore și luminoase în cazuri de depășire a pragurilor critice
- 🖥️ Interfață grafică pe ecran TFT (480x320px), cu meniuri interactive și butoane touch
- 🔄 Afișare dinamică a valorilor și salvare automată a min/max
- 📈 Calcule de altitudine și conversie automată a unităților de presiune
- 🌐 Serviciu IoT pentru control și vizualizare la distanță
- 📱 Aplicație de mobil ce permite afișarea valorilor în timp real de pe cele două module

---

## 🛠️ Componente Hardware

| Componentă          | Descriere                                                                 |
|---------------------|---------------------------------------------------------------------------|
| **WT32-SC01**        | ESP32 cu touchscreen TFT 480x320px                                        |
| **BME280**           | Măsoară temperatură, umiditate, presiune (precizie 0.03 hPa)              |
| **MQ-2**             | Senzor gaze inflamabile (CO, metan, alcool, etc.)                         |
| **GP2Y1010AU0F**     | Senzor de praf optic – detectează particule în suspensie                  |
| **Buzzer**           | Semnal sonor în caz de pericol                                            |
| **LED-uri**          | Semnalizare vizuală pentru evenimente critice                            |
| **Breadboard**       | Asamblare modulară rapidă                                                 |
| **WeMos D1 mini**    | ESP8266 cu WiFi – conectare la ThingSpeak                                |

---

## 💻 Software și Librării

- **Limbaj:** C++ (Arduino)
- **Platforme:** Arduino IDE, ThingSpeak
- **Librării:**  
  - `TFT_eSPI`, `FT62XXTouchScreen`, `Free_Fonts`  
  - `MQUnifiedsensor`, `Adafruit_BME280`, `ThingSpeak`

---

## ☁️ Funcționalități IoT (ThingSpeak)

- Vizualizare în timp real a valorilor senzorilor
- Reacții automate la depășirea pragurilor (ex: alarme)
- Posibilitate extindere: sistem de alertă GSM, server web, control ESP-NOW

---

## 🧠 Exemple de funcții

```cpp
if (temperature > thresholdTemp) {
  digitalWrite(buzzerPin, HIGH);
  digitalWrite(ledRed, HIGH);
}
```

```cpp
float pressure_hPa = bme.readPressure() / 100.0;
float pressure_mmHg = pressure_hPa * 0.75006;
```

---

## 🔮 Dezvoltări viitoare

- Versiune portabilă cu baterie Li-ion
- Sistem GSM pentru alerte remote
- Integrare senzor ploaie (agricultură)
- Server web cu dashboard HTML
- Clasificare a calității aerului pe bază de scor
- Casă inteligentă (ESP-NOW)

---

## 👨‍💻 Autori

- **Radu Gabriel Claudiu** – programare, integrare senzori, logică alarme  
- **Trîmbițaș George Bogdan** – interfață grafică, meniuri, testare  
- **Prof. Ramona Humeniuc** – coordonare tehnică, suport fizică & IoT
