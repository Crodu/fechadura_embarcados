#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

const char *ssid = "Domotica";
const char *password = "32132132";

WiFiServer server(80);
IPAddress local_IP(192, 168, 0, 200);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

String header;
String biometria;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 3000;

const uint32_t passwordFingerprint = 0x0;
Adafruit_Fingerprint fingerprintSensor = Adafruit_Fingerprint(&Serial2, passwordFingerprint);

LiquidCrystal_I2C lcd(0x27,16,2);

//endereço do servidor na qual sera enviado as informações de cadastro
String ipServer = "http://192.168.0.101:8080";

void setupFingerprintSensor();
boolean cadastroBiometria();
boolean uploadBiometria(int userId);
void printLcd(String msg, byte linha);
boolean cadastroRfid();

void setup()
{
    Serial.begin(9600);

    setupFingerprintSensor();

    lcd.init();
    lcd.backlight();
    lcd.clear();

    // configurar para escutar as requisiçoes sempre no mesmo ip
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
    {
        Serial.println("STA Failed to configure");
    }

    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    // server.begin();

    // esclui banco de dados das biometrias
    if (fingerprintSensor.emptyDatabase() != FINGERPRINT_OK)
    {
        Serial.println(F("Erro ao apagar banco de digitais"));
    }

    printLcd("Modo Standby",0);
}

void loop()
{
    currentTime = millis();  
    if (WiFi.status() == WL_CONNECTED && currentTime - previousTime > timeoutTime)
    {
        previousTime = currentTime;
        HTTPClient http;
        
        http.begin(ipServer+"/command");
        int httpCode = http.GET();

        if (httpCode > 0)
        {
            Serial.print("httpCode: ");
            Serial.println(httpCode);
            String payload = http.getString();
            Serial.print("payload: ");
            Serial.println(payload);

            DynamicJsonDocument doc(1024);
            doc["heap"] = ESP.getFreeHeap();
            deserializeJson(doc, payload);
            boolean cadastro = doc["isRegistering"];
            String sensor = doc["currentSensor"];
            int userId = doc["userId"];

            Serial.print("cadastro: ");
            Serial.println(cadastro);
            Serial.print("sensor: ");
            Serial.println(sensor);
            Serial.print("userId: ");
            Serial.println(userId);

            if (cadastro)
            {
                if (sensor == "biometria")
                {
                    boolean result = false;
                    while(!result){
                        result = cadastroBiometria();
                        if (result){
                            result = uploadBiometria(userId);                    
                        }
                        if (result){
                            http.end();
                            printLcd("Sucesso !!!!",0);
                            delay(2000);
                            // ESP.restart();
                        }
                        else {
                            printLcd("Erro !!!!",0);
                            printLcd("Tente novamente",1);
                            delay(2000);
                        }
                    }
                }
                else if (sensor == "rfid")
                {

                }
            }
        }
        else
        {
            Serial.println("Error on HTTP request");
        }
        http.end(); 
    }
}

void setupFingerprintSensor()
{
    fingerprintSensor.begin(57600);

    if (!fingerprintSensor.verifyPassword())
    {
        Serial.println(F("Não foi possível conectar ao sensor. Verifique a senha ou a conexão"));
        while (true)
            ;
    }
}

boolean cadastroBiometria(){
    int location = 1;
    printLcd("Encoste o dedo no",0);
    printLcd("sensor",1);
    while (fingerprintSensor.getImage() != FINGERPRINT_OK)
        ;

    if (fingerprintSensor.image2Tz(1) != FINGERPRINT_OK)
    {
        printLcd("Erro image2Tz 1",0);
        return false;
    }

    printLcd("Tire o dedo no",0);
    printLcd("sensor",1);
    delay(2000);

    while (fingerprintSensor.getImage() != FINGERPRINT_NOFINGER)
        ;

    printLcd("Encoste o mesmo",0);
    printLcd("dedo no sensor",1);

    while (fingerprintSensor.getImage() != FINGERPRINT_OK)
        ;

    if (fingerprintSensor.image2Tz(2) != FINGERPRINT_OK)
    {
        printLcd("Erro image2Tz 2",0);
        return false;
    }

    if (fingerprintSensor.createModel() != FINGERPRINT_OK)
    {
        printLcd("Erro createModel",0);
        return false;
    }

    if (fingerprintSensor.storeModel(location) != FINGERPRINT_OK)
    {
        printLcd("Erro storeModel",0);
        return false;
    }

    printLcd("Sucesso!!!",0);
    return true;
}

boolean uploadBiometria(int userId){
    int location = 1;
    uint8_t p = fingerprintSensor.loadModel(location);
    switch (p)
    {
    case FINGERPRINT_OK:
        break;
    case FINGERPRINT_PACKETRECIEVEERR:
        return false;
    default:
        return false;
    }

    p = fingerprintSensor.getModel();
    switch (p)
    {
    case FINGERPRINT_OK:
        break;
    default:
        return p;
    }

    uint32_t starttime = millis();
    int i = 0;
    while (i < 534 && (millis() - starttime) < 20000)
    {
        if (Serial2.available())
        {
            biometria += Serial2.read();
            biometria += ",";
            i++;
        }
    }
    // Serial2.flush();
    Serial2.end();
    delay(500);
    fingerprintSensor.begin(57600);

    if ((WiFi.status() == WL_CONNECTED))
    {
        HTTPClient http;

        //enviara a posição da digital que sera enviada e o template
        //o tempate tem o formato seguinte:
        //3,3,92,36,100,1,112,1,146,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,0,0,0,151,0,0,0,0,0,0,204,204,252,255,255,63,243,255,63,186,187,254,235,190,238,238,170,170,166,153,153,153,101,86,85,69,85,85,81,85,81,84,64,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,23,109,52,5,92,254,99,13,34,254,111,141,100,94,79,24,4,158,123,159,101,126,38,32,220,126,120,44,19,254,109,184,170,254,78,190,218,126,49,23,7,95,104,24,101,159,62,25,30,63,114,153,229,159,89,26,233,127,91,160,234,191,115,33,19,63,57,171,196,255,102,180,107,95,91,7,159,92,98,38,170,29,90,18,29,186,91,139,32,91,89,20,38,123,99,171,43,24,95,171,88,184,135,150,206,190,129,58,106,60,98,199,192,92,104,200,22,220,144,170,38,88,138,57,130,118,138,187,85,86,55,243,148,51,129,84,140,49,107,15,58,202,67,254,49,77,68,121,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,197,1,32,140,175,1,32,144,175,1,32,36,0,0,0,10,171,3,3,83,31,0,1,32,1,132,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,125,0,0,0,0,0,12,207,207,255,243,255,63,243,251,171,191,238,186,234,170,154,166,102,101,89,85,85,85,85,85,69,68,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        //no backend sera utilizada as seguintes funções para transformar o template recebido em 4 pacotes:
        //const split = queryParameters.templates.split('239,1,255,255,255,255,2,0,130,');
        //biometria = '?biometria='+split[1].slice(0,-1)+'/'+split[2].slice(0,-1)+'/'+split[3].slice(0,-1)+'/'+split[4].slice(0,-1)
        
        http.begin(ipServer+"/register?userId="+String(userId)+"&sensor=biometria&data="+biometria);
        int httpCode = http.GET();

        if (httpCode > 0)
        {
            Serial.print("HTTP Response code: ");
            Serial.println(httpCode);
            String payload = http.getString();
            Serial.println(payload);
        }
        else
        {
            Serial.println("Error on HTTP request");
            return false;
        }

        http.end(); 
    }

    return true;
}

void printLcd(String msg, byte linha){
    Serial.println(msg);
    lcd.clear();
    lcd.setCursor(0,linha);
    lcd.print(msg);
}

boolean cadastroRfid(){

}