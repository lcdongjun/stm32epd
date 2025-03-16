#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h> // 包含 ArduinoJson 库
// WiFi 配置
const char* ssid = "ESP8266_Hotspot";
const char* password = "12345678";

// 创建 WebServer 对象
ESP8266WebServer server(80);

// 变量定义
bool flage = false; // 开关初始状态
String currentCity = "zhengzhou"; // 默认城市

// 发送 reboot_to_bootloader 命令
void handleRebootToBootloader() {
  Serial.println("reboot_to_bootloader");
  server.send(200, "text/plain", "Command sent: reboot_to_bootloader");
}

// 开关操作
void handleToggleSwitch() {
  flage = !flage; // 切换开关状态
  Serial.println(flage ? "1" : "0");
  server.send(200, "text/plain", flage ? "1" : "0");
}

// 时间设置处理
void handleSetTime() {
  if (server.hasArg("h") && server.hasArg("m") && server.hasArg("s")) {
    int hour = server.arg("h").toInt();
    int minute = server.arg("m").toInt();
    int second = server.arg("s").toInt();
    // 输入范围校验
    if (hour >= 0 && hour < 24 && minute >= 0 && minute < 60 && second >= 0 && second < 60) {
      char timeStr[50];
      sprintf(timeStr, "h:%02d,m:%02d,s:%02d", hour, minute, second);
      Serial.println(timeStr); // 通过串口发送时间
      server.send(200, "text/plain", String("Time set: ") + timeStr);
      return;
    }
  }
  server.send(400, "text/plain", "Invalid time input"); // 输入无效
}

// 文件上传处理回调
void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    // Serial.printf("Start uploading: %s\n", upload.filename.c_str());
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Serial.write(upload.buf, upload.currentSize); // 直接发送接收到的数据块
    // Serial.printf("Uploaded %d bytes\n", upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    // Serial.printf("\nUpload finished: %s (%d bytes)\n", upload.filename.c_str(), upload.totalSize);
  }
}

// 提供上传页面
void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>ESP8266 Control</title>
      <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        button, input { margin: 5px; padding: 10px; font-size: 16px; }
        .container { margin: 20px; }
      </style>
    </head>
    <body>
      <div class="container">
        <h1>ESP8266 Control Panel</h1>
        <!-- 按钮 -->
        <button onclick="sendCommand('reboot')">Reboot to Bootloader</button>
        <br>
        <!-- 开关 -->
        <label for="toggleSwitch">Toggle Switch:</label>
        <button id="toggleSwitch" onclick="toggleSwitch()">OFF</button>
        <br><br>
        <!-- 时间设置 -->
        <h2>Set Time</h2>
        <label>Hour: <input id="hour" type="number" min="0" max="23"></label><br>
        <label>Minute: <input id="minute" type="number" min="0" max="59"></label><br>
        <label>Second: <input id="second" type="number" min="0" max="59"></label><br>
        <button onclick="setTime()">Set Time</button>
        <p id="message"></p>
        <br>
        <!-- 文件上传 -->
        <h2>File Upload</h2>
        <form method="POST" action="/upload" enctype="multipart/form-data">
          <input type="file" name="file">
          <input type="submit" value="Upload">
        </form>
        <h2>Weather Settings</h2>
        <label for="city">Enter City:</label>
        <input type="text" id="city" placeholder="Enter city name (e.g., zhengzhou)">
        <button onclick="submitCity()">Submit</button>
      </div>
      <script>
        // 初始化开关状态
        let switchState = )rawliteral" + String(flage ? "true" : "false") + R"rawliteral(;
        const toggleSwitchButton = document.getElementById('toggleSwitch');
        toggleSwitchButton.textContent = switchState ? "ON" : "OFF";

        // 发送命令
        function sendCommand(command) {
          fetch('/' + command).then(response => response.text()).then(alert);
        }

        // 切换开关
        function toggleSwitch() {
          fetch('/toggle').then(response => response.text()).then(state => {
            switchState = state === "1";
            toggleSwitchButton.textContent = switchState ? "ON" : "OFF";
          });
        }

        // 设置时间
        function setTime() {
          const h = document.getElementById('hour').value;
          const m = document.getElementById('minute').value;
          const s = document.getElementById('second').value;
          fetch(`/set_time?h=${h}&m=${m}&s=${s}`).then(response => {
            if (response.ok) {
              return response.text();
            } else {
              throw new Error("Invalid time input");
            }
          }).then(message => {
            document.getElementById('message').textContent = message;
          }).catch(error => {
            document.getElementById('message').textContent = error.message;
          });
        }
        function submitCity() {
          const city = document.getElementById("city").value;
          if (city.trim() === "") {
            alert("Please enter a valid city name.");
            return;
          }
          // 使用 AJAX 将城市名发送到服务器
          const xhr = new XMLHttpRequest();
          xhr.open("POST", "/set_city", true);
          xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
          xhr.send("city=" + encodeURIComponent(city));
          xhr.onload = function () {
            if (xhr.status === 200) {
              alert("City updated successfully!");
            } else {
              alert("Failed to update city. Try again.");
            }
          };
        }
      </script>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void connectToWiFiAndFetchWeather() {
  const char* wifiSSID = "Xiaomi_E42D";       // 替换为实际的 WiFi 名称
  const char* wifiPassword = "q7579364"; // 替换为实际的 WiFi 密码

  WiFi.begin(wifiSSID, wifiPassword);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi.");
  Serial.println("Local IP: " + WiFi.localIP().toString());
  
  // 获取天气信息
  String weather = getWeather();
  Serial.println("Weather Info:");
  Serial.println(weather); // 输出天气信息到串口
}


// 模式 2：恢复到热点模式
void setupHotspot() {
  WiFi.softAP(ssid, password);
  Serial.println("Hotspot mode enabled.");
}

// 获取天气信息
String getWeather() {
  WiFiClient client;
  const char* server = "api.seniverse.com";
  const int port = 80;
  const char* apiKey = "S0WNrNaWM7M7aqKcO";
  const char* language = "en";
  const char* unit = "c";


  // 使用用户输入的城市
  String url = String("/v3/weather/daily.json?key=") + apiKey +
               "&location=" + currentCity + 
               "&language=" + language +
               "&unit=" + unit+
               "&start=0"+
               "&days=3";

  if (client.connect(server, port)) {
    client.print(String("GET ") + url + " HTTP/1.1\r\n");
    client.print(String("Host: ") + server + "\r\n");
    client.print("Connection: close\r\n\r\n");

  String response = "";
    while (client.connected() || client.available()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        if (line.startsWith("{")) {
          response = line; // 获取 JSON 响应体
          break;
        }
      }
    }
    client.stop();

    if (response.length() > 0) {
      // 解析 JSON 数据
      StaticJsonDocument<2048> doc; // 确保足够大的缓冲区
      DeserializationError error = deserializeJson(doc, response);

      if (!error) {
        String result = "";
        const char* cityName = doc["results"][0]["location"]["name"];
        const char* lastUpdate = doc["results"][0]["last_update"];
        JsonArray dailyArray = doc["results"][0]["daily"];

        result += "City: " + String(cityName) + "\n";
        result += "Last Update: " + String(lastUpdate) + "\n";

        // 获取前三天的天气数据
        for (size_t i = 0; i < 3 && i < dailyArray.size(); i++) {
          JsonObject day = dailyArray[i];

          const char* date = day["date"];
          const char* textDay = day["text_day"];
          const char* textNight = day["text_night"];
          const char* high = day["high"];
          const char* low = day["low"];
          const char* precip = day["precip"];

          result += "Date: " + String(date) + "\n";
          result += "  Day: " + String(textDay) + "\n";
          result += "  Night: " + String(textNight) + "\n";
          result += "  High Temp: " + String(high) + "°C\n";
          result += "  Low Temp: " + String(low) + "°C\n";
          result += "  Precipitation: " + String(precip) + "%\n";
        }

        return result; // 返回格式化的结果
      } else {
        Serial.println("Failed to parse weather JSON.");
        return "Weather fetch failed.";
      }
    } else {
      Serial.println("No response from weather server.");
      return "Weather fetch failed.";
    }
  }

  return "Weather fetch failed.";
}



void setup() {
  // 初始化串口
  Serial.begin(921600);
  // 设置 WiFi 热点
  WiFi.softAP(ssid, password);
  Serial.println("Hotspot created. Connect to:");
  Serial.println(ssid);
  // 配置 WebServer 路由
  server.on("/", HTTP_GET, handleRoot);
  server.on("/reboot", HTTP_GET, handleRebootToBootloader);
  server.on("/toggle", HTTP_GET, handleToggleSwitch);
  server.on("/set_time", HTTP_GET, handleSetTime);
  server.on("/upload", HTTP_POST, []() {
    server.send(200, "text/plain", "File Uploaded.");
  }, handleFileUpload);
  server.on("/set_city", HTTP_POST, []() {
    if (server.hasArg("city")) {
      currentCity = server.arg("city"); // 获取用户输入的城市名
      Serial.println("City updated to: " + currentCity);
      server.send(200, "text/plain", "City updated");
    } else {
      server.send(400, "text/plain", "City name missing");
    }
  });

  // 启动服务器
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();

  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim(); // 去除空格和换行
    if (command == "Mode 1") {
      connectToWiFiAndFetchWeather();
    } else if (command == "Mode 2") {
      setupHotspot();
    }
  }
}

