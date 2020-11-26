// ESP32_cam webcam server with servo control for 1. servo
// OTA update implemented successful - UID:jopare PW:just4fun 
// Compiler data - Board: ESP32 Wroover - minimal SPIFFS 1.9MB App with OTA 190/kb SPIFFS QIO
// WiFi multi takes sometimes time to connect - needs to be investigated

#include "esp_camera.h"
#include <WiFiMulti.h>
#include <Servo.h>
#include "soc/soc.h"                              //disable brownout detection
#include "soc/rtc_cntl_reg.h"                     //disable brownout detection

                                                  //OTA stuff  start **********
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
                                                  //OTA stuff   end  **********
Servo myservo;

WiFiMulti wifiMulti;
int ledPin = 4;

#define CAMERA_MODEL_AI_THINKER

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// GPIO Setting
int servoPin  =  2;
int posDegrees = 90;                                //servo initial position 90°
int posDegreesStep = 30;                            //30° moved each step 

                                                    //OTA stuff  start **********
const char* host = "esp32";
WebServer server(83);

const char* loginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='jopare' && form.pwd.value=='just4fun')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";

const char* serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";
                                                    //OTA stuff   end  **********

extern String WiFiAddr ="";

void startCameraServer();

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);        //disable brownout detectio
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  
  myservo.attach(servoPin, 2, 0, 180);              //(pin, channel, min, max) degrees
  myservo.write(posDegrees); 

  pinMode(servoPin, OUTPUT); 
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_SXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  wifiMulti.addAP("SW LXIV", "diebrummsummselbrummt1.undsummt2.herum");
  wifiMulti.addAP("NSA Sniffer", "irgend1oderso");
  //wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

  Serial.println("Connecting Wifi...");
  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("SSID:       ");
    Serial.println(WiFi.SSID());
  }
                                                    //OTA stuff  start **********
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
                                                      //OTA stuff  end  ***********   
  startCameraServer();

  Serial.print("Camera Ready! ");
  Serial.print(WiFi.localIP());
  WiFiAddr = WiFi.localIP().toString();
  Serial.println("' to connect");
  Serial.print("OTA ready!");
 }

void wifireconnect() {

  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    delay(1000);
    WiFi.disconnect();
    Serial.println("\nReboot");
    ESP.restart();
  }
}

void links() {

  if (posDegrees >= 0 && posDegrees <= 180) {
    posDegrees = posDegrees + posDegreesStep;
  }
  if (posDegrees > 180) {
    posDegrees = 180;
  }
  else {
    myservo.write(posDegrees);                      // move the servo to calculated angle
    Serial.print("Moved to: ");
    Serial.print(posDegrees);                       // print the angle
    Serial.println(" degree");
       }
  }
void rechts() {
 
  if (posDegrees > 0 && posDegrees <= 180) {
    posDegrees = posDegrees - posDegreesStep;
  }
  if (posDegrees < 0) {
    posDegrees = 0;
  } else {
    myservo.write(posDegrees);                      // move the servo to calculated angle
    Serial.print("Moved to: ");
    Serial.print(posDegrees);                       // print the angle
    Serial.println(" degree");
         }
 }
void loop() {
 
   wifireconnect();

                                                    //OTA stuff start  ***********
  server.handleClient();
  delay(1);
                                                    // OTA stuff  end  ***********
}
