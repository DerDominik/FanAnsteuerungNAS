#include <Arduino.h>
#include <SoftwareSerial.h>

// Notwendige Einstellungen
#define _SMARTDEBUG                 // Ausklammern, um Debug Ausgabe zu deaktiveren
int PWMPercent = 25;                // PWM Wert in Prozent - Intialstart-Wert fuer Luefter
int PWMStandby = 25;                // PWM Wert fuer Standby Betrieb
int Setpoint = 25;                  // Soll-Temperatur (zur Berechnung der Luefter Geschwindkeit)
word FanPin = 3;                    // Pin für Luefter
word FanMaxValue = 200;             // Maximal Wert fuer Luefter-Ansteuerung (muss getestet werden)
unsigned int WaitTime = 10;         // Wartezeit, bis der Luefter nach Ausschalten der HDD ausgeschaltet wird
SoftwareSerial debugSerial(12, 13); // RX TX
int Debug = 0;                      // Debug aktivieren
int FanStop = 2;                    // PWMPercentWert für gestoppten Luefter

// Variablen Deklaration
int RS232Value = 0;         // Eingang aus RS232 (Wert kleiner 100 = Temperatur; Wert = 100 -> HDD aus)
int HDDActive = 0;          // Merker fuer HDD Zustand (HDDActive = 1 -> HDD läuft; HDDActive = 0 -> HDD aus)
int HDDTemp = 0;            // Merker fuer HDD Temperatur
int PWM = 0;                // PWM Wert roh
int Standby = 0;            // Merker fuer Standby
word TempGap = 0;           // Temperatur-Unterschied ablegen
String rs232_simulate;      // Merker fuer RS232 Auswertung
char puffer[50];            // Puffer fuer RS232 Auswertung
unsigned long startTime ;   // Timer Startwert 
unsigned long elapsedTime ; // Timer abgelaufene Zeit
int TimerStart = 0;         // Merker fuer Timer

// ------------ PWMPercent Ansteuerung ------------

void PWMPercent25kHzBegin() {
  TCCR2A = 0;                               // TC2 Control Register A
  TCCR2B = 0;                               // TC2 Control Register B
  TIMSK2 = 0;                               // TC2 Interrupt Mask Register
  TIFR2 = 0;                                // TC2 Interrupt Flag Register
  TCCR2A |= (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);  // OC2B cleared/set on match when up/down counting, fast PWMPercent
  TCCR2B |= (1 << WGM22) | (1 << CS21);     // prescaler 8
  OCR2A = FanMaxValue;                               // TOP overflow value (Hz)
  OCR2B = 0;
}

void PWMPercentDuty(byte ocrb) {
  OCR2B = ocrb;                             // PWMPercent Width (duty)
}

// ------------ PWMPercent Ansteuerung ------------

void setup() {

    if (Debug == 1) {
      debugSerial.begin(9600);
      debugSerial.println("Debug Monitor");
    }
    Serial.begin(9600);
    pinMode(FanPin, OUTPUT);    // Pin fuer Luefter
    PWMPercent25kHzBegin();     // PWMPercent Ansteuerung anstoßen
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

    // ------------ Ansteuerung Luefer ------------

    // ------------ Zustand HDD ist eingeschaltet ------------
    if (HDDActive == 1) {

        Standby = 0;
        startTime = 0;
        elapsedTime = 0;
        TimerStart = 0;

    // ------------ Wenn Temperatur kleiner Soll-Temperatur -> Luefter dreht mit 15% ------------
    // ------------ Wenn Temperatur groesser Soll-Temperatur -> Luefter dreht abhaengig von Temperaturunterschied ------------

        if (HDDTemp <= Setpoint) {
          PWMPercent = FanStop;
        } else {
          TempGap = HDDTemp - Setpoint;
          PWMPercent = ( 15 + (TempGap * 2));
        }

    // ------------ Zustand HDD ist ausgeschaltet ------------
    } else {

        // ------------ Ist der vorherige Timer durchgelaufen, Lüfter ausschalten ------------
        if (Standby == 1) {
            PWMPercent = FanStop;
            
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
    PWM = FanMaxValue / 100 * PWMPercent;
    PWMPercentDuty(PWM);

    // ------------ Debug Ausgaben erzeugen ------------
    if (Debug == 1) {
      debugSerial.write(12);//ASCII for a Form feed
      debugSerial.print("RS232 Value: ");
      debugSerial.println(RS232Value);
      debugSerial.print("HDD aktiv? ");
      debugSerial.println(HDDActive);
      debugSerial.print("HDD Temperatur: ");
      debugSerial.println(HDDTemp);
      debugSerial.print("HDD Temp-Gap: ");
      debugSerial.println(TempGap);
      debugSerial.print("PWM (Percent): ");
      debugSerial.println(PWMPercent);
      debugSerial.print("PWM (Value): ");
      debugSerial.println(PWM);
      debugSerial.print("Start Time: ");
      debugSerial.println(startTime);
      debugSerial.print("Elapsed Time: ");
      debugSerial.println(elapsedTime);
    }

}