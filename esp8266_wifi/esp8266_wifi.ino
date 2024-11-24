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
bool fan = false; // 开关初始状态
bool camera = false; // 开关初始状态
String currentCity = "leshan"; // 默认城市

// 发送 reboot_to_bootloader 命令
void handleRebootToBootloader() {
  Serial.println("reboot_to_bootloader");
  server.send(200, "text/plain", "Command sent: reboot_to_bootloader");
}
// 发送 reboot_to_system 命令
void handleRebootToSystem() {
  Serial.println("reboot_to_system");
  server.send(200, "text/plain", "Command sent: reboot_to_system");
}

// 开关操作
void handleToggleSwitch() {
  flage = !flage; // 切换开关状态
  Serial.println("toggle:");
  Serial.println(flage ? "1" : "0");
  server.send(200, "text/plain", flage ? "1" : "0");
}
void handlefanSwitch() {
  fan = !fan; // 切换开关状态
  Serial.println("fan:");
  Serial.println(fan ? "1" : "0");
  server.send(200, "text/plain", fan ? "1" : "0");
}
void handlecameraSwitch() {
  camera = !camera; // 切换开关状态
  Serial.println("camera:");
  Serial.println(camera ? "1" : "0");
  server.send(200, "text/plain", camera ? "1" : "0");
}

// 时间设置处理
void handleSetTime() {
  if (server.hasArg("y") && server.hasArg("mh") && server.hasArg("d")&&server.hasArg("h") && server.hasArg("m") && server.hasArg("s")) {
    int year = server.arg("y").toInt();
    int month = server.arg("mh").toInt();
    int day = server.arg("d").toInt();
    int hour = server.arg("h").toInt();
    int minute = server.arg("m").toInt();
    int second = server.arg("s").toInt();
    // 输入范围校验
    if (hour >= 0 && hour < 24 && minute >= 0 && minute < 60 && second >= 0 && second < 60&&year >= 2000 && year < 2099 && month >= 1 && month <= 12 && day >= 1 && day <= 31) {
      char timeStr[100];
      sprintf(timeStr, "y:%02d,mh:%02d,d:%02d,h:%02d,m:%02d,s:%02d", year,month,day,hour, minute, second);
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
    Serial.printf("\nUpload end\n");
  }
}


// 提供上传页面
void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP8266 网页</title>
    <style>
      body {
        font-family: Arial, sans-serif;
        margin: 0;
        padding: 0;
        display: flex;
        justify-content: center;
        align-items: center;
        height: 100vh; /* 页面全屏高度 */
        background-color: #f9f9f9;
      }

      .container {
        text-align: center;
        background: #fff;
        padding: 10px 10px;
        border: 1px solid #ddd;
        border-radius: 10px;
        box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
      }

      h1 {
        font-size: 24px;
        margin-bottom: 20px;
      }

      .settings-container {
        display: flex; /* 使用 Flexbox 布局 */
        justify-content: space-between; /* 左右排列 */
        align-items: flex-start; /* 顶部对齐 */
        gap: 8px; /* 子元素间距 */
        margin: 8px 0;
      }

      .settings-section {
        flex: 1; /* 平分空间 */
        text-align: center; /* 居中子元素内容 */
        max-width: 240px; /* 最大宽度 */
        padding: 10px;
        box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); /* 卡片样式 */
        border-radius: 10px; /* 圆角 */
        background: #fff;
        border: 1px solid #ddd;
      }

      .settings-section input,
      .settings-section button {
        display: block;
        margin: 10px auto;
        padding: 10px;
        font-size: 16px;
        border: 1px solid #ccc;
        border-radius: 5px;
        text-align: center;
        width: 80%;
      }

      .time-container {
        display: grid;
        grid-template-columns: repeat(3, 1fr); /* 三列布局 */
        gap: 10px; /* 网格间距 */
      }

      .time-container label {
        display: flex;
        flex-direction: column;
        text-align: center;
      }

      progress {
        width: 100%;
      }
    </style>
  </head>
  <body>
    <div class="container">
      <h1>ESP8266 网页</h1>
      <h2>功能设置</h2>
      <div class="settings-container">
        <!-- 时间设置 -->
        <div class="settings-section">
          <h3>设置时间</h3>
          <div class="time-container">
            <label>年:
              <input id="year" type="number" min="2000" max="2099">
            </label>
            <label>月:
              <input id="month" type="number" min="1" max="12">
            </label>
            <label>日:
              <input id="day" type="number" min="1" max="31">
            </label>
            <label>小时:
              <input id="hour" type="number" min="0" max="23">
            </label>
            <label>分钟:
              <input id="minute" type="number" min="0" max="59">
            </label>
            <label>秒:
              <input id="second" type="number" min="0" max="59">
            </label>
          </div>
          <button onclick="setTime()">设置时间</button>
          <p id="message"></p>
        </div>

        <!-- 上传文件 -->
        <div class="settings-section">
          <h3>上传文件</h3>
          <input type="file" id="fileInput" onchange="sendFileSize()">
          <button onclick="uploadFile()">上传</button>
          <progress id="uploadProgress" value="0" max="100" style="display: none;"></progress>
        </div>

        <!-- 天气设置 -->
        <div class="settings-section">
          <h3>天气设置</h3>
          <label for="city">输入城市:</label>
          <input type="text" id="city" placeholder="请输入城市名 (e.g., zhengzhou)">
          <button onclick="submitCity()">提交</button>
        </div>
      </div>

      <h2>状态</h2>
      <div class="settings-container">
        <!-- 按钮 -->
        <div class="settings-section">
          <h3>重启管理</h3>
          <button onclick="sendCommand('Bootloader')">重启到Bootloader</button>
          <button onclick="sendCommand('System')">重启到System</button>
        </div>

        <!-- 开关 -->
        <div class="settings-section">
          <h3>开关控制</h3>
          <label for="toggleSwitch">灯:</label>
          <button id="toggleSwitch" onclick="toggleSwitch()">OFF</button>

          <label for="fanSwitch">风扇:</label>
          <button id="fanSwitch" onclick="fanSwitch()">OFF</button>

          <label for="cameraSwitch">摄像头:</label>
          <button id="cameraSwitch" onclick="cameraSwitch()">OFF</button>
        </div>
      </div>
    </div>

    <script>
      // 初始化开关状态
      let switchState = )rawliteral" + String(flage ? "true" : "false") + R"rawliteral(;
      const toggleSwitchButton = document.getElementById('toggleSwitch');
      toggleSwitchButton.textContent = switchState ? "ON" : "OFF";
      
      let fanswitchState = )rawliteral" + String(fan ? "true" : "false") + R"rawliteral(;
      const fanSwitchButton = document.getElementById('fanSwitch');
      fanSwitchButton.textContent = fanswitchState ? "ON" : "OFF";

      let cameraswitchState = )rawliteral" + String(camera ? "true" : "false") + R"rawliteral(;
      const cameraSwitchButton = document.getElementById('cameraSwitch');
      cameraSwitchButton.textContent = cameraswitchState ? "ON" : "OFF";
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
      function fanSwitch() {
        fetch('/fan').then(response => response.text()).then(state => {
          fanswitchState = state === "1";
          fanSwitchButton.textContent = fanswitchState ? "ON" : "OFF";
        });
      }
      function cameraSwitch() {
        fetch('/camera').then(response => response.text()).then(state => {
          cameraswitchState = state === "1";
          cameraSwitchButton.textContent = cameraswitchState ? "ON" : "OFF";
        });
      }
      function sendFileSize() {
        const fileInput = document.getElementById("fileInput");
        if (fileInput.files.length === 0) {
          alert("请选择一个文件！");
          return;
        }
        const file = fileInput.files[0];
        const fileSize = file.size; // 文件大小，单位字节
        console.log(`fileSize`);
        const xhr = new XMLHttpRequest();
        xhr.open("POST", "/fileSize", true);
        xhr.setRequestHeader("Content-Type", "application/json");
        xhr.send(JSON.stringify({ size: fileSize }));
      }

      function uploadFile() {
        const fileInput = document.getElementById("fileInput");
        const progress = document.getElementById("uploadProgress");
        if (fileInput.files.length === 0) {
          alert("请选择一个文件进行上传！");
          return;
        }
        const file = fileInput.files[0];
        const xhr = new XMLHttpRequest();
        progress.style.display = "block";
        xhr.open("POST", "/upload", true);
        xhr.upload.onprogress = function (event) {
          if (event.lengthComputable) {
            const percent = Math.round((event.loaded / event.total) * 100);
            progress.value = percent;
          }
        };
        xhr.onload = function () {
          if (xhr.status === 200) {
            alert("文件上传成功！");
          } else {
            alert("文件上传失败！");
          }
          progress.style.display = "none";
          progress.value = 0;
        };
        const formData = new FormData();
        formData.append("file", file);
        xhr.send(formData);
      }

      function setTime() {
        const y = document.getElementById('year').value;
        const mh = document.getElementById('month').value;
        const d = document.getElementById('day').value;
        const h = document.getElementById('hour').value;
        const m = document.getElementById('minute').value;
        const s = document.getElementById('second').value;
        if (!y || y < 1970 || y > 2099) {
          alert("请输入有效的年份(1970-2099)!");
          return;
        }
        if (!mh || mh < 1 || mh > 12) {
          alert("请输入有效的月份(1-12)!");
          return;
        }
        if (!d || d < 1 || d > 31) {
          alert("请输入有效的日期(1-31)!");
          return;
        }
        if (!h || h < 0 || h > 23) {
          alert("请输入有效的小时(0-23)!");
          return;
        }
        if (!m || m < 0 || m > 59) {
          alert("请输入有效的分钟(0-59)!");
          return;
        }
        if (!s || s < 0 || s > 59) {
          alert("请输入有效的秒数(0-59)!");
          return;
        }
        fetch(`/set_time?y=${y}&mh=${mh}&d=${d}&h=${h}&m=${m}&s=${s}`)
          .then(response => {
            if (response.ok) {
              return response.text();
            } else {
              throw new Error("设置时间失败，请检查输入！");
            }
          })
          .then(message => {
            document.getElementById('message').textContent = message;
          })
          .catch(error => {
            document.getElementById('message').textContent = error.message;
          });
      }

      function submitCity() {
        const city = document.getElementById("city").value;
        if (city.trim() === "") {
          alert("请输入有效的城市名称。");
          return;
        }
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
  // Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    // Serial.print(".");
  }
  // Serial.println("\nConnected to WiFi.");
  // Serial.println("Local IP: " + WiFi.localIP().toString());
  
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
          result += "  High Temp: " + String(high) + "\n";
          result += "  Low Temp: " + String(low) + "\n";
          result += "  Precipitation: " + String(precip) + "\n";
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

// 解析键值对数据的值
bool parseValue(String data, String key) {
  // 查找键的起始位置
  int keyIndex = data.indexOf(key);
  if (keyIndex != -1) {
    // 查找键后面的值部分
    int valueIndex = data.indexOf(" ", keyIndex + key.length() + 1);
    if (valueIndex == -1) {
      valueIndex = data.length(); // 如果没有空格，读取到字符串末尾
    }
    // 提取值
    String value = data.substring(keyIndex + key.length() + 1, valueIndex);
    return (value == "1"); // 如果值为 "1"，返回 true；否则返回 false
  }
  return false;
}
void Var_Init()
{
  Serial.println("Variable Init");
  while(!Serial.available());
  // 读取串口数据直到换行符结束
  String input = Serial.readStringUntil('\n');
  Serial.println("Variable: " + input);
  // 解析数据
  if (input.indexOf("flage") != -1) {
    flage = parseValue(input, "flage");
  }
  if (input.indexOf("fan") != -1) {
    fan = parseValue(input, "fan");
  }
  if (input.indexOf("camera") != -1) {
    camera = parseValue(input, "camera");
  }
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
  server.on("/Bootloader", HTTP_GET, handleRebootToBootloader);
  server.on("/System", HTTP_GET, handleRebootToSystem);
  server.on("/toggle", HTTP_GET, handleToggleSwitch);
  server.on("/fan", HTTP_GET, handlefanSwitch);
  server.on("/camera", HTTP_GET, handlecameraSwitch);
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
  // 设置路由
  server.on("/fileSize", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      Serial.println("File size: " + body);
      server.send(200, "application/json", "{\"status\":\"file size received\"}");
    } else {
      server.send(400, "application/json", "{\"error\":\"no data received\"}");
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
    }else if (command == "Var Init") {
      Var_Init();
    }
  }
}

