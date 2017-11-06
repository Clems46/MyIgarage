/*
 * HTTP communication between ESP8266 and Jeedom Smart Home Server
 * Communication HTTP (TCP/IP) entre ESP8266 et le serveur domotique Jeedom
 * Copyright (C) 2017 https://www.projetsdiy.fr - http://www.diyprojects.io
 *   
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
// Temperature sensor
#include "DHT.h"

#define DHTPIN D1     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

float Lum = 0;
const float etalonnage = 0.91314;
int pos = 0;
float tmpmin;
float tmpmax;
const int led = 13;

int porte1 = 0; //Etat porte 1
int porte2 = 0; //Etat porte 2
int porte3 = 0; //Etat porte 3

int buffer_porte1 = 0;
int buffer_porte2 = 0;
int buffer_porte3 = 0;
 
const char* ssid     = "Livebox-bca8";
const char* password = "5794913D19FE17D5CD643F5572";
const char* host     = "192.168.1.83";  //IP du Jeedom
const int   port     = 80;
const char* apiKey   = "9NDFnv5vdjaDgdSV3q1DQyLreyqYAjxr";
const char* IDporte1 = "65";
const char* IDporte2 = "67";
const char* IDporte3 = "68";
const char* IDtemp   = "69";
const char* IDhum    = "70";
const char* IDlum    = "71";
const char* IDrssi   = "72";

const int   watchdog = 600000;
unsigned long previousMillis = millis(); 


ESP8266WebServer server ( 80 );
HTTPClient http;
 

 
void setup() {

   //Initialisation du capteur de temperature/humidité
  dht.begin();
 //Initialise les pins
  
  pinMode(LED_BUILTIN, OUTPUT);// Initialize the LED_BUILTIN pin as an output
  pinMode(D4, OUTPUT); //Led rouge
  pinMode(D2, OUTPUT); //Led verte
  pinMode(D3, OUTPUT); //Led Bleu
  pinMode(led, OUTPUT); //Led sur l arduino
  digitalWrite(led, 0);
  pinMode(D5, INPUT); //Capteur porte 1
  pinMode(D6, INPUT); //Capteur porte 2
  pinMode(D7, INPUT); //Capteur porte 3

  
  Serial.begin(115200);   //initialise le port serie
  delay(10);
  
  Serial.setDebugOutput(true);  
  Serial.println("Connecting Wifi...");
 
  WiFi.begin(ssid, password);   //Initialise la connection Wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.print(WiFi.localIP());
  for (int i = 0; i < 4; i++)  //Fait clignoter 4 fois la led Jaune si il y a une connection
  {
  analogWrite(D3, 250);
  delay(75);
  analogWrite(D3, 0);
  delay(75);
  }
 tmpmin = dht.readTemperature() * etalonnage;
 tmpmax = tmpmin; 
  
  server.begin();   //Initialise le server client

  //Initialise les buffers portes
  porte1 = digitalRead(D5);
  porte2 = digitalRead(D6);
  porte3 = digitalRead(D7);
  buffer_porte1 = porte1;
  buffer_porte2 = porte2;
  buffer_porte3 = porte3;
  
}
 
int value = 0;
 
void loop() {
  server.handleClient();


  //Gestion des E/S

  
   digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
                                    // but actually the LED is on; this is because 
                                    // it is acive low on the ESP-01)
  delay(1000);                      // Wait for a second
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  // Luminosité sensor
   Lum = analogRead(A0);
   Lum = map (Lum, 0, 1024, 0, 100);
   Serial.print("Luminosité =");
   Serial.println(Lum);
   //Eclairage des LED, Bleu : froid, rouge : Chaud
  
   if (Lum > 50) {
     digitalWrite(D4, HIGH);
     digitalWrite(D2, LOW);
     delay(100); 
   }
   else {
     digitalWrite(D4, LOW);
     digitalWrite(D2, HIGH);
   }
   delay(100);
  //Temperature sensor
   float h = dht.readHumidity(); //Read the humidity
   float tempraw = dht.readTemperature(); // Read temperature as Celsius (the default)
   float temp = tempraw * etalonnage;
   Serial.print("temp= ");
   Serial.print(temp);
   Serial.print(" tempraw= ");
   Serial.print(tempraw);
   Serial.print(" humdity = ");
   Serial.println(h);
 //  if (isnan(h) || isnan(tempraw) ) {
 //   Serial.println("Failed to read from DHT sensor!");
 //   return;
 // }

  //Find min and max
//Tableau pour faire une moyenne de la temperature
int NB_SAMPLE_temp = 15;
float tab_temp[NB_SAMPLE_temp];
int tab_index_temp = 0;
    for (int i = 0; i < NB_SAMPLE_temp; i++)  
    {
     tab_temp[tab_index_temp] = temp;
     tab_index_temp = tab_index_temp++;     //remplissage du tableau
    }

     tab_index_temp = 0;
     float somme_temp = 0.0 ;

    for (int i = 0 ; i < NB_SAMPLE_temp ; i++)
    {
     somme_temp += tab_temp[tab_index_temp] ; //somme des valeurs (temp) du tableau
     tab_index_temp = tab_index_temp++; 
    }   

    float moytemp = somme_temp / NB_SAMPLE_temp ; //valeur moyenne
    Serial.print("moyenne température = ");
    Serial.println(moytemp);
    //Cherche les min et max
    if(moytemp > tmpmax) { tmpmax = moytemp; }
    if(moytemp < tmpmin) { tmpmin = moytemp; }
    Serial.println(tmpmax);
    Serial.println(tmpmin);
  
  
  //Doors sensors
  porte1 = digitalRead(D5);
  porte2 = digitalRead(D6);
  porte3 = digitalRead(D7);
  delay(10);
  
  Serial.print("Porte 1 = "); //Ecrit sur le port série l'état des portes
  Serial.print(porte1);
  Serial.print(" ");
  Serial.print("Porte 2 = ");
  Serial.print(porte2);
  Serial.print(" ");
  Serial.print("Porte 3 = ");
  Serial.print(porte3);
  Serial.println(" ");

       //divers
       Serial.print("rssi = ");
       Serial.println(WiFi.RSSI());
int rssi = WiFi.RSSI();
delay(5);

//Envoie des infos au Jeedom touts les watchdogs ou modif état portes
  unsigned long currentMillis = millis();
 
 //   if ( currentMillis - previousMillis > watchdog ) {
 //   previousMillis = currentMillis;
    if (buffer_porte1 != porte1 || buffer_porte2 != porte2 || buffer_porte3 != porte3 || currentMillis - previousMillis > watchdog ) {
      
      previousMillis = currentMillis;
      buffer_porte1 = porte1; 
      buffer_porte2 = porte2;
      buffer_porte3 = porte3;
      
    if(WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi not connected !");
    } else {  
      Serial.println("Send data to Jeedom");
      
      delay(10);

      String baseurl = "/core/api/jeeApi.php?apikey="; 
      baseurl += apiKey;
      baseurl += "&type=virtual&id="; 
      String url = baseurl + IDporte1;
      url += url + "&value="; url += String(porte1); 
      sendToJeedom(url);
      digitalWrite(D3, LOW);
      delay(1500);

      url = baseurl + IDporte2;
      url += url + "&value="; url += String(porte2);
      sendToJeedom(url);
      digitalWrite(D3, HIGH);
      delay(1500);
 
      url = baseurl + IDporte3;
      url += url + "&value="; url += String(porte3);
      sendToJeedom(url);
      digitalWrite(D3, LOW);
      delay(1500);

      url = baseurl + IDtemp;
      url += url + "&value="; url += String(temp);
      sendToJeedom(url);
      digitalWrite(D3, HIGH);
      delay(1500);
      
      url = baseurl + IDhum;
      url += url + "&value="; url += String(h);
      sendToJeedom(url);
      digitalWrite(D3, LOW);
      delay(1500);

      url = baseurl + IDlum;
      url += url + "&value="; url += String(Lum);
      sendToJeedom(url);
      digitalWrite(D3, HIGH);
      delay(1500);

      url = baseurl + IDrssi;
      url += url + "&value="; url += String(rssi);
      sendToJeedom(url);
      digitalWrite(D3, LOW);
      delay(1500);

      
    }
  }
}
 
boolean sendToJeedom(String url){
  Serial.print("connecting to ");
  Serial.println(host);
  Serial.print("Requesting URL: ");
  Serial.println(url);
  http.begin(host,port,url);
  int httpCode = http.GET();
  Serial.println("closing connection");
  http.end();
}
