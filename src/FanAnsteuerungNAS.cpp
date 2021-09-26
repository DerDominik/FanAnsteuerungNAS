#include <Arduino.h>
#include <SoftwareSerial.h>

// Notwendige Einstellungen
word FanPin = 9;                    // Pin für Luefter
int PWMPercent = 25;                // PWM Wert in Prozent - Intialstart-Wert fuer Luefter
int PWMStandby = 25;                // PWM Wert fuer Standby Betrieb
int Setpoint = 25;                  // Soll-Temperatur (zur Berechnung der Luefter Geschwindkeit)
int FanMinValue = 3;                // PWMPercentWert für gestoppten Luefter
word FanMaxValue = 255;             // Maximal Wert fuer Luefter-Ansteuerung (muss getestet werden)
unsigned int WaitTime = 10;         // Wartezeit, bis der Luefter nach Ausschalten der HDD ausgeschaltet wird

// Variablen Deklaration
int RS232Value = 0;                 // RS232 Einganswert (Wert kleiner 100 = Temperatur; Wert = 100 -> HDD aus)
int HDDActive = 0;                  // Merker fuer HDD Zustand (HDDActive = 1 -> HDD läuft; HDDActive = 0 -> HDD aus)
int HDDTemp = 0;                    // Merker fuer HDD Temperatur - wird aus RS232Value gezogen
int PWM = 0;                        // PWM Wert roh
int Standby = 0;                    // Merker fuer Standby
String rs232_simulate;              // Merker fuer RS232 Auswertung
char puffer[50];                    // Puffer fuer RS232 Auswertung
unsigned long startTime ;           // Timer Startwert 
unsigned long elapsedTime ;         // Timer abgelaufene Zeit
int TimerStart = 0;                 // Merker fuer Timer
int DebugCount = 0;                 // Counter zur Verzoegerung der Debug Ausgabe

// Debug Einstellungen
int Debug = 1;                      // Debug aktivieren
SoftwareSerial debugSerial(12, 13); // RX TX

void setup() {

  // SoftwareSerial initialisieren - Wenn aktiviert wurde
  if (Debug == 1) {
    debugSerial.begin(9600);
    debugSerial.println("Debug Monitor");
  }

  // Serial initialisieren
  Serial.begin(9600);
  
  // Fan Pin als Ausgang deklarieren
  pinMode(FanPin, OUTPUT);

  // PWM Signal vorbereiten
  TCCR1B = TCCR1B & 0b11111000 | 0x01;
}

void loop() {

  // ------------ RS232 Inhalt empfangen ------------
  if(Serial.available()){
      rs232_simulate = Serial.readStringUntil('\n');
      rs232_simulate.toCharArray(puffer, sizeof puffer);
      RS232Value = atoi(puffer);
  }

  // ------------ RS232 Inhalt auswerten ------------
  // Wert 0-99 = Temperatur = HDD Aktiv
  // Wert 100  = HDD inaktiv

  if (RS232Value == 100) {
    HDDActive = 0;
  } else {
    HDDActive = 1;
  }

  if (HDDActive == 1) {
    HDDTemp = RS232Value;
  } else {
    HDDTemp = 0;
  }

  // ------------ Ansteuerung Luefter ------------

  // ------------ Zustand HDD ist eingeschaltet ------------
  if (HDDActive == 1) {

      Standby = 0;
      startTime = 0;
      elapsedTime = 0;
      TimerStart = 0;

  // ------------ Temperaturauswertung und PWM-Prozent zuweisen

      if ((HDDTemp >=0) && (HDDTemp<= 25)) {
        PWMPercent = 0;
      } else if ((HDDTemp >=26) && (HDDTemp<= 28)) {
        PWMPercent = 10;
      } else if ((HDDTemp >=29) && (HDDTemp<= 32)) {
        PWMPercent = 20;
      } else if ((HDDTemp >=33) && (HDDTemp<= 35)) {
        PWMPercent = 30;
      } else if ((HDDTemp >=36) && (HDDTemp<= 39)) {
        PWMPercent = 50;
      } else if ((HDDTemp >=40) && (HDDTemp<= 45)) {
        PWMPercent = 75;
      } else {
        PWMPercent = 100;
      }

  // ------------ Zustand HDD ist ausgeschaltet ------------
  } else {

      // ------------ Ist der vorherige Timer durchgelaufen, Lüfter ausschalten ------------
      if (Standby == 1) {
          PWMPercent = FanMinValue;
          
      // ------------ Luefter mit definiertem Standby Wert laufen lassen ------------
      } else {
          PWMPercent = PWMStandby;

          // ------------ Timer initialisieren und starten ------------
          if (TimerStart == 0) {
              startTime = millis();
              TimerStart = 1;
          }
          
          // ------------ Abgelaufene Zeit in Sekunden ermitteln ------------
          elapsedTime =  (millis() - startTime) / 1000 ;

          // ------------ Wenn Timer abgelaufen, den Luefter ausschalten ------------
          if (elapsedTime == WaitTime) {
              PWMPercent = 0;
              Standby = 1;
          }
      }
  }

// ------------ Eigentliche Ansteuerung des Luefters ------------

// Prozentzahl in Bereich FanMinValue <-> FanMaxValue umrechnen
PWM = map(PWMPercent, 0, 100, FanMinValue, FanMaxValue);

// PWM Wert auf Ausgang schreiben
analogWrite (FanPin, PWM);

// ------------ Debug Ausgaben erzeugen ------------
  if (Debug == 1) {
    if (DebugCount <= 100) {
      DebugCount = DebugCount + 1 ;
    } else {
      debugSerial.write(12);//ASCII for a Form feed
      debugSerial.print("RS232 Value: ");
      debugSerial.println(RS232Value);
      debugSerial.print("HDD aktiv? ");
      debugSerial.println(HDDActive);
      debugSerial.print("HDD Temperatur: ");
      debugSerial.println(HDDTemp);
      debugSerial.print("PWM (Percent): ");
      debugSerial.println(PWMPercent);
      debugSerial.print("PWM (Value): ");
      debugSerial.println(PWM);
      debugSerial.print("Start Time: ");
      debugSerial.println(startTime);
      debugSerial.print("Elapsed Time: ");
      debugSerial.println(elapsedTime);
      DebugCount = 0;
    }
}

}