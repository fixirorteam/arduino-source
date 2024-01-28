
#include <Firebase_ESP_Client.h>
#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>


//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define FLAME_SENSOR_PIN 12 // Sesuaikan dengan pin yang digunakan untuk flame sensor
#define BUZZ 4 // Sesuaikan dengan pin yang digunakan untuk flame sensor

//siapkan variabel menampung url
String url;
//siapkan variabel untuk wifi client
WiFiClient client;



// Insert Firebase project API Key
#define API_KEY "AIzaSyAkSEJwRn_QFaz_wFxyvKlbtCtH365OiP8"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://fixiot-a24cd-default-rtdb.asia-southeast1.firebasedatabase.app/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

  bool res;
  bool signupOK = false;



  void readDeviceInfo() {
    //siapkan barcode
    String barcodeData = "fixiroriot";
  Serial.println("\nDevice Information:");
  Serial.println("===================");

  // Membaca informasi jenis chip
  Serial.print("Chip ID: ");
  Serial.println(ESP.getChipId(), HEX);

  // Membaca informasi Flash Size
  Serial.print("Flash Size: ");
  Serial.println(ESP.getFlashChipSize() / (1024 * 1024), DEC);
  Serial.println("MB");

  // Membaca informasi Free Sketch Space
  Serial.print("Free Sketch Space: ");
  Serial.println(ESP.getFreeSketchSpace(), DEC);
  Serial.println("bytes");

  // Kirim informasi ke Firebase
  if (Firebase.ready() && signupOK) {
    if (Firebase.RTDB.setInt(&fbdo, "/device_info/about/chip_id", ESP.getChipId())) {
      Serial.println("Chip ID Sent to Firebase");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    } else {
      Serial.println("Failed to Send Chip ID to Firebase");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setInt(&fbdo, "/device_info/about/flash_size", ESP.getFlashChipSize())) {
      Serial.println("Flash Size Sent to Firebase");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    } else {
      Serial.println("Failed to Send Flash Size to Firebase");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setInt(&fbdo, "/device_info/about/free_sketch_space", ESP.getFreeSketchSpace())) {
      Serial.println("Free Sketch Space Sent to Firebase");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    } else {
      Serial.println("Failed to Send Free Sketch Space to Firebase");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}

void setup() {
  
  pinMode(FLAME_SENSOR_PIN, INPUT);
  pinMode(BUZZ, OUTPUT);
  WiFi.mode(WIFI_STA);  // explicitly set mode, esp defaults to STA+AP
  // it is a good practice to make sure your code sets wifi mode how you want it.

  // put your setup code here, to run once:
  Serial.begin(9600);

  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;
  WiFi.hostname("Jecembery IOT");
  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
  wm.resetSettings();

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm.autoConnect("FIXIROR IOT", "password");  // password protected ap

  if (!res) {
    Serial.println("Failed to connect");
    // ESP.restart();
  } else {
    //if you get here you have connected to the WiFi
    Serial.println("WiFi Sudah Terkoneksi)");
  }
  

  /* Assign the api key (required) */
    config.api_key = API_KEY;
    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

   /* Sign up */
    if (Firebase.signUp(&config, &auth, "", "")){
      Serial.println("ok");
      signupOK = true;
    }
    else{
      Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h


    Firebase.begin(&config, &auth);

    // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
    Firebase.reconnectNetwork(true);

  readDeviceInfo();
}

void loop() {
   int flameValue = digitalRead(FLAME_SENSOR_PIN);

   if(flameValue == 0){
    digitalWrite(BUZZ, HIGH);
    kirim_wa("Pesan Otomatis Perangkat Fixiror Flame Sensor \n\nBahaya!!! telah ditemukan titik api kebakaran. \nSilahkan evakuasi keluarga dan barang didalam rumah anda");
   }else{
    digitalWrite(BUZZ, LOW);
   }


  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1500 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int
    if (flameValue == 0) {
      // If there is fire, send 0 to Firebase
      
      if (Firebase.RTDB.setInt(&fbdo, "/device_info/flame_sensor/zone_1", flameValue)) {
        Serial.println("Fire Detected - Value Set to 0");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      } else {
        Serial.println("Failed to Set Value to 0 in Firebase");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    } else {
      // If there is no fire, send the actual flame value to Firebase
      if (Firebase.RTDB.setInt(&fbdo, "/device_info/flame_sensor/zone_1", flameValue)) {
        Serial.println("Flame Value Sent to Firebase");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      } else {
        Serial.println("Failed to Send Flame Value to Firebase");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    }
  }
  sendMemoryInfoToFirebase();
}


void kirim_wa(String pesan) {
  url = "http://api.callmebot.com/whatsapp.php?phone=+6285218817974&text="+ urlencode(pesan) +"&apikey=6979553";
  //kirim pesan
  postData();
}

void postData() {
  //siapkan untuk menampung pesan terkirim atau tidak ke whatsapp
  int httpCode;
  //siapkan variabel untuk protokol http yang akan terkoneksi ke server callme
  HTTPClient http;
  //eksekusi link url
  http.begin(client, url);
  httpCode = http.POST(url);
  
  //uji nilai variabel httpCode
  if (httpCode==200) {
    Serial.println("Notifikasi berhasil terkirim");
  } else {
    Serial.println("Notifikasi tidak terkirim");
  }
  http.end();
}

void sendMemoryInfoToFirebase() {
  // Dapatkan informasi memori
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t maxFreeBlockSize = ESP.getMaxFreeBlockSize();
  
  // Kirim informasi memori ke Firebase
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1500 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    
    // Kirim nilai freeHeap ke Firebase
    if (Firebase.RTDB.setInt(&fbdo, "/device_info/memory/freeHeap", freeHeap)) {
      Serial.println("Memory Free Sent to Firebase");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    } else {
      Serial.println("Failed to Send Memory Free to Firebase");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    // Kirim nilai maxFreeBlockSize ke Firebase
    if (Firebase.RTDB.setInt(&fbdo, "/device_info/memory/maxFreeBlockSize", maxFreeBlockSize)) {
      Serial.println("Max Free Block Size Sent to Firebase");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    } else {
      Serial.println("Failed to Send Max Free Block Size to Firebase");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}




String urlencode(String str) {
  String encodedString = "";
  char c;
  char code0, code1, code2;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    // jika ada spasi diantara pesan yang akan dikirim, maka ganti dengan karakter +

    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += '0';
      encodedString += '1';
    }
    yield();
  }
  Serial.println(encodedString);
  return encodedString;
}
