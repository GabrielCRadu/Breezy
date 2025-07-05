
# 🌬️ Breezy – Air Quality Monitor

**Breezy** este un dispozitiv hardware portabil care monitorizează în timp real calitatea aerului, oferind utilizatorului informații esențiale despre atmosferă și avertismente în cazuri periculoase (ex: potențial de asfixiere).

> Realizat ca proiect educațional în clasa a IX-a, cu scopul de a crește conștientizarea asupra sănătății respiratorii și a mediului.

---

## 📦 Funcționalități principale

- 📊 Măsurarea temperaturii, umidității și presiunii atmosferice  
- 🧪 Detectarea compușilor volatili (VOC): CO, metan, fum, alcool etc.  
- 🔔 Sistem de alarme sonore în caz de depășire a limitelor sigure  
- 🖥️ Interfață grafică pe ecran TFT 480x320  
- ⚙️ Meniu de setări pentru praguri de alarmă și preferințe  
- 🔄 Actualizare automată a valorilor în timp real (buclă la 1 sec / 1 min)

---

## 🛠️ Hardware

| Componentă       | Descriere                                                                 |
|------------------|---------------------------------------------------------------------------|
| **WT32-SC01**     | ESP32 cu ecran TFT integrat (480x320px), interfață tactilă               |
| **BME280**        | Senzor de temperatură, presiune și umiditate (precizie ridicată, I2C)    |
| **MQ-2**          | Senzor de gaze (VOC, CO, metan, fum etc.)                                |
| **Buzzer**        | Emite alarme sonore în cazuri critice                                    |
| **Breadboard**    | Montare experimentală a circuitului                                       |

---

## 💻 Software

- Scris în C++ pentru **platforma Arduino**  
- Afișare pe TFT folosind librăria `TFT_eSPI`  
- Citire și procesare date de la senzori  
- Meniuri interactive + setări cu butoane (navigare, praguri alarmă)  
- Structuri logice: `loop()`, `setup()`, bucle controlate pe timp

---

## 📷 Imagini

> *(Adaugă aici capturi de ecran sau fotografii ale prototipului în funcțiune)*

---

## 📈 Exemple de funcții importante

```cpp
void loop() {
  // Actualizare date senzori
  temperatura = bme.readTemperature();
  presiune = bme.readPressure();
  voc = MQ2.readSensor();

  // Afișare pe ecran
  tft.drawFloat(temperatura, 1, 240, 60, 4);

  // Alerte
  if (temperatura > prag_temperatura_max) {
    digitalWrite(buzzerPin, HIGH);
  }
}
```

---

## 🔮 Dezvoltări viitoare

- Integrare senzor praf (PM2.5)  
- Versiune portabilă cu baterie Li-ion și carcasă 3D  
- Salvare date în memorie și analiză istorică  
- Integrare cu aplicație mobilă via Wi-Fi/Bluetooth

---

## 👨‍💻 Autori

- **Radu Gabriel Claudiu** – dezvoltare hardware & software  
- **Trîmbițaș George Bogdan** – UI și design meniuri  
- **Prof. Humeniuc Ramona** – coordonator proiect

---

## 📜 Licență

Proiect educațional open-source – disponibil sub licență MIT.

---
