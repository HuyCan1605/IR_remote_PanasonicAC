const char MainPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<head>
    <title>WiFi and MQTT config</title>
    <style>
        /* Body và tiêu đề chính */
        body {
            font-family: Arial, sans-serif;
            display: flex;
            flex-direction: column;
            align-items: center;
            margin: 0;
            padding: 0;
            background-color: #f4f4f4;
            color: #333;
        }

        #mainTitle {
            font-size: 2em;
            font-weight: bold;
            color: #0066cc;
            margin: 20px;
        }

        #deviceName {
            font-size: 1.7em;
            color: #333;
            font-weight: 550;
            margin-bottom: 20px;
        }

        

        /* Style cho từng khung cấu hình */
        .formContainer {
            width: 80%;
            max-width: 400px;
            background-color: white;
            border: 1px solid #ddd;
            border-radius: 10px;
            box-shadow: 0px 4px 10px rgba(0, 0, 0, 0.1);
            padding: 20px;
            margin-bottom: 20px;
        }

        .formTitle {
            font-size: 1.4em;
            color: #0066cc;
            font-weight: bold;
            margin-bottom: 15px;
            text-align: center;

        }

        /* Style cho các label và input */
        .formLabel {
            font-size: 1em;
            font-weight: bold;
            color: #333;
            display: block;
            margin-top: 10px;
        }

        .formInput {
            width: 100%;
            padding: 10px;
            margin: 8px 0 15px;
            border: 1px solid #ddd;
            border-radius: 5px;
            box-sizing: border-box;
            font-size: 1em;
        }

        /* Style cho các nút lưu và upload */
        .saveButton {
            width: 100%;
            background-color: #0066cc;
            color: white;
            padding: 10px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 1em;
            font-weight: bold;
            transition: background-color 0.3s;
        }

        .saveButton:hover {
            background-color: #0055a5;
        }


        #otaForm,
        #wifiForm,
        #mqttForm,
        #deviceNameForm {
            border: 1px solid #ddd;
            padding: 15px 25px;
            border-radius: 8px;
            margin-top: 20px;
            margin-bottom: 50px;
            width: 80%;
            max-width: 400px;
            background-color: #ffffff;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        }

        .deviceInfo {
            font-size: 16px;
            color: #0066cc;
            margin: 2px 0;
        }

    </style>
</head>

<body>

    <div id="mainTitle">WiFi and MQTT Config</div>
    <div id="deviceName">%devicename%</div>
    <div id="macDevice" class="deviceInfo">MAC Address: %macdevice%</div>
    <div id="ipDevice" class="deviceInfo">IP Address: %ipdevice%</div>
    <!-- Form cấu hình WiFi -->

    <!-- Device Name Configuration Form -->
    <div id="deviceNameForm">
        <div class="formTitle">Device Name</div>
        <form id="changenameForm" action="/changedevicename" method="POST">
            <label for="deviceNameInput" class="formLabel">Device Name</label>
            <input type="text" id="deviceNameInput" class="formInput" name="deviceName" placeholder="Enter Device Name"
                maxlength="20" required>
            <button type="submit" id="btnSaveDeviceName" class="saveButton">Save Device Name</button>
        </form>
    </div>
    <div id="wifiForm">
        <div class="formTitle">WiFi Parameters</div>
        <form id="wifiFormsub" action="/savewifi" method="POST">
            <!-- Sử dụng form và phương thức POST -->
            <label for="wifiSSID" class="formLabel">WiFi SSID</label>
            <input type="text" id="wifiSSID" class="formInput" placeholder="Enter WiFi SSID" name="ssid">

            <label for="wifiPassword" class="formLabel">WiFi Password</label>
            <input type="password" id="wifiPassword" class="formInput" placeholder="Enter WiFi Password" name="password"
                minlength="8" required>

            <button type="submit" id="btnSaveWifi" class="saveButton">Save WiFi</button>
        </form>
    </div>

    <!-- Form cấu hình MQTT -->
    <div id="mqttForm">
        <div class="formTitle">MQTT Parameters</div>
        <form id="mqttFormsub" action="/savemqtt" method="POST">
            <!-- Sử dụng form và phương thức POST -->
            <label for="mqttHost" class="formLabel">Host</label>
            <input type="text" id="mqttHost" class="formInput" placeholder="Enter MQTT Host" name="server"
                maxlength="40">

            <label for="mqttPort" class="formLabel">Port <span>(1883)</span></label>
            <input type="text" id="mqttPort" class="formInput" placeholder="1883" name="port" maxlength="5">

            <label for="mqttClient" class="formLabel">Client</label>
            <input type="text" id="mqttClient" class="formInput" placeholder="Enter Client ID" name="clientID"
                maxlength="20">

            <label for="mqttTopic" class="formLabel">Topic Set <span>(ESP -> HA)</span></label>
            <input type="text" id="mqttTopic" class="formInput" placeholder="panasonic/ac/set" name="topicset"
                maxlength="30">

            <label for="mqttTopic" class="formLabel">Topic Get <span>(HA -> ESP)</span></label>
            <input type="text" id="mqttTopic" class="formInput" placeholder="panasonic/ac/get" name="topicget"
                maxlength="30">

            <label for="mqttUser" class="formLabel">User</label>
            <input type="text" id="mqttUser" class="formInput" placeholder="Enter Username" name="user" maxlength="20">

            <label for="mqttPassword" class="formLabel">Password</label>
            <input type="password" id="mqttPassword" class="formInput" placeholder="Enter Password" name="pass"
                maxlength="20">

            <button type="submit" id="btnSaveMqtt" class="saveButton">Save MQTT</button>
        </form>
    </div>


    <!-- Form OTA Upload -->
    <div id="otaForm">
        <div class="formTitle">OTA Firmware Update</div>
        <form id="otaFormsub" action="/update" method="POST" enctype="multipart/form-data">
            <label for="firmwareFile" class="formLabel">Choose Firmware File (.bin)</label>
            <input type="file" id="firmwareFile" class="formInput" name="firmware" accept=".bin" required>

            <button type="submit" id="btnUploadOTA" class="saveButton">Upload Firmware</button>
        </form>
        <div id="otaStatus" class="statusMessage"></div>
    </div>
</body>

</html>
)=====";

const char wifiSuccessfulConnectionWeb[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
    <head>
        <style>
        /* Thiết lập chung cho body */
        body {
            font-family: Arial, sans-serif;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            margin: 0;
            background-color: #e6f2ff;
            color: #333;
        }

        /* Style cho tên thiết bị */
        #device {
            font-size: 2em;
            font-weight: bold;
            color: #0066cc;
            margin-bottom: 10px;
            text-align: center;
        }

        /* Style cho trạng thái kết nối */
        #status {
            font-size: 1.5em;
            color: #33cc33;
            margin: 10px 0;
            font-style: italic;
            text-align: center;
        }

        /* Style cho thông báo chuyển hướng */
        #noti {
            font-size: 1em;
            color: #333;
            background-color: #cce7ff;
            padding: 10px 20px;
            border-radius: 5px;
            text-align: center;
            box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.2);
            margin-top: 20px;
        }
    </style>
    </head>
    <body>
        <div id="device">ESP8266</div>
        <div id = "status">Successful MQTT Connection</div>
    </body>
</html>
)=====";

const char mqttSuccessfulConnectionWeb[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <style>
        /* Body */
        body {
            font-family: Arial, sans-serif;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
            margin: 0;
            background-color: #f0f8ff;
            color: #333;
        }

        /* Thiết lập cho tên thiết bị */
        #device {
            font-size: 2em;
            font-weight: bold;
            color: #0066cc;
            margin-bottom: 10px;
            text-align: center;
        }

        /* Thiết lập cho trạng thái kết nối */
        #status {
            font-size: 1.5em;
            color: #4CAF50;
            margin: 10px 0;
            font-style: italic;
            text-align: center;
        }
    </style>
</head>
<body>
    <div id="device">%devicename%</div>
    <div id="status">Successful Configuration</div>
</body>
</html>

)=====";