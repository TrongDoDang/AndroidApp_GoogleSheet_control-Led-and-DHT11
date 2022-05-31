 #include<ESP8266WiFi.h>  
 #include "HTTPSRedirect.h"  
 #include "DHT.h" 
 #define DHTTYPE DHT11 // type of the temperature sensor
const int DHTPin = 5; //--> The pin used for the DHT11 sensor is Pin D1 = GPIO5
DHT dht(DHTPin, DHTTYPE);
 //Khai báo thông tin về Wifi  
const char* ssid = "Wifi Free";  //Thay  ten_wifi bang ten wifi nha ban
const char* password = "abcdefgh123@@"; //Thay mat_khau_wifi bang mat khau cua ban  
   
 //Khai báo thông tin về server của google  
 const char *GScriptID = "AKfycbz94fDrYwj3gb2RtHSzyp0m19uorhoci3DFeKtX5-CAl8erXpMmaD83Z5BpXNj22P00"; //Google Script ID đã publish trước đó  
 const char* host = "script.google.com";  
 const char *googleRedirectHost = "script.googleusercontent.com";  
 const int httpsPort = 443;  
 const char *cellAddress = "E2";
 String GAS_ID = "AKfycbz94fDrYwj3gb2RtHSzyp0m19uorhoci3DFeKtX5-CAl8erXpMmaD83Z5BpXNj22P00";  
 HTTPSRedirect client(httpsPort);  
 //HTTPSRedirect* client = nullptr;
 //Khai báo URL chứa thông tin gủi dữ liệu  
 String url = String("/macros/s/") + GScriptID + "/exec?";  
 String url3 = String("/macros/s/") + GScriptID + "/exec?read=" + cellAddress;
 String payload = "";

 //Khai báo biến chứa thời gian lặp để gửi dữ liệu vào google sheet (ở đây biến tính bằng mili giay)  
 unsigned long preMillis = 0;  
 const long interval = 5000; // ví dụ mình post 1 phút 1 lần sẽ bằng 1 * 60 * 1000 = 60000  

 void setup() {  
    //init the relay control pin
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH);
  // put your setup code here, to run once:  
  Serial.begin(115200);  
  Serial.println("Connecting to wifi: ");  
  Serial.println(ssid);  
  Serial.flush();  
  WiFi.begin(ssid, password); //Kết nói wifi  
  
  dht.begin();  //--> Start reading DHT11 sensors
  delay(500);
  while (WiFi.status() != WL_CONNECTED){  
   delay(500);  
   Serial.print(".");  
  }  
  Serial.print(" IP Address: ");  
  Serial.println(WiFi.localIP());  
   
  //Do sử dụng HTTPS nên mình sẽ tạo 1 TLS connection  
  client.setInsecure();  
  client.setPrintResponseBody(false);  
  client.setContentTypeHeader("application/json");  
  Serial.print(String("Kết nối tới "));  
  Serial.println(host); //Hiển thị thông tin kết nối  
   
  //Lấy thông tin trạng thái kết nối với server của google  
  bool flag = false;  
  for (int i=0; i<5; i++){  
   int retval = client.connect(host, httpsPort);  
   if(retval == 1){  
    flag = true; //nếu kết nối thành công sẽ trả về trạng thái  
    break;  
   }else{  
    Serial.println("Kết nối thất bại. Đang thử kết nối lại ....");  
   }  
  }  
   
  Serial.println("Trạng thái kết nối là: " + String(client.connected()));//1 là thành công, 0 là thất bại  
  if(!flag){  
   Serial.print("Không thể kết nối tới server");  
   Serial.println(host);  
   Serial.println("Thoát....");  
   Serial.flush();  
   return;  
  }  
 }  
   

   
 //Khai báo hàm postData  
 void postData(float tem, int hum){  
  if(!client.connected()){ //Thực hiện kết nối lại google server khi mất kết nối  
   Serial.println("Kết nối tới server...");  
   client.connect(host, httpsPort);  
  }  
   String string_temperature =  String(tem);
  // String string_temperature =  String(tem, DEC); 
  String string_humidity =  String(hum, DEC); 
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&humidity=" + string_humidity;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //----------------------------------------

  //----------------------------------------Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();  
 }  
   
   
 void loop() {  

  
    //Do mình sử dụng biến đếm thời gian của arduino để cho gửi chính xác hơn hàm delay()  
  unsigned long currentMillis = millis();  
  if(currentMillis - preMillis >= interval){  
   preMillis = currentMillis;  
   
    // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  int h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor !");
    delay(500);
    return;
  }
  String Temp = "Temperature : " + String(t) + " °C";
  String Humi = "Humidity : " + String(h) + " %";
  Serial.println(Temp);
  Serial.println(Humi);
 
 postData(t, h); //Thực hiện gửi dữ liệu vào google sheet  

  }  

  }

  
