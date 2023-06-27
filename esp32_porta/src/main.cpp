#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Keypad.h>

#define BUZZER_PIN 15
#define RELE_PIN 13
#define PIR_PIN 4
#define LDR_PIN 34
#define LED_VERDE_PIN 34
#define LED_VERMELHO_PIN 34
#define LED_BRANCO_PIN 34

int LDR_Val = 0;

const uint8_t ROWS = 4;
const uint8_t COLS = 4;
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
uint8_t colPins[COLS] = {33, 32, 19, 23};
uint8_t rowPins[ROWS] = {14, 27, 26, 25};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const char *ssid = "Domotica";
const char *password = "32132132";

WiFiServer server(80);
IPAddress local_IP(192, 168, 0, 201);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

String header;
String biometria;
String senhaTeclado = "";
int lastpos = 1;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

const uint32_t passwordFingerprint = 0x0;
Adafruit_Fingerprint fingerprintSensor = Adafruit_Fingerprint(&Serial2, passwordFingerprint);
boolean cadastro = false;

TaskHandle_t Task2;

String ipServer = "http://192.168.0.101:3000";

void setupFingerprintSensor();
void getAllFingerprints();
void Task2code(void *pvParameters);
uint8_t uploadFingerprintTemplate(uint8_t packet1[130], uint8_t packet2[130], uint8_t packet3[130], uint8_t packet4[108], int posicao);
void beepSucesso();
void beepFracasso();

void setup()
{
    Serial.begin(9600);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, HIGH);
    pinMode(RELE_PIN, OUTPUT);
    pinMode(PIR_PIN, INPUT);
    pinMode(LDR_PIN, INPUT);
    pinMode(LED_BRANCO_PIN, OUTPUT);
    pinMode(LED_VERDE_PIN, OUTPUT);
    pinMode(LED_VERMELHO_PIN, OUTPUT);

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

    xTaskCreatePinnedToCore(
        Task2code,
        "Task2",
        10000,
        NULL,
        1,
        &Task2,
        1);

}

void loop()
{
    // digitalWrite(rele, HIGH);
    // Serial.println("HIGH");
    // delay(5000);
    // digitalWrite(rele, LOW);
    // Serial.println("LOW");
    // delay(5000);

    LDR_Val = analogRead(LDR_PIN);
    if (LDR_Val > 2000 && digitalRead(PIR_PIN) == HIGH)
    {
        digitalWrite(LED_BRANCO_PIN, HIGH);
    }

    char key = keypad.getKey();
    if (key)
    {
        if (key == '*')
        {
            senhaTeclado = "";
        }
        else if (key != '*' && key != '#')
        {
            senhaTeclado += key;
        }
        else if (key == '#' && senhaTeclado.length() > 0)
        {

            if (WiFi.status() == WL_CONNECTED)
            {
                HTTPClient http;

                http.begin(ipServer + "/command");
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
                }
                else
                {
                    Serial.println("Error on HTTP request");
                }
                http.end();
            }
            senhaTeclado = "";
        }
    }

    // if(!cadastro){
    //     Serial.println(F("Encoste o dedo no sensor"));

    //     while (fingerprintSensor.getImage() != FINGERPRINT_OK)
    //         ;

    //     if (fingerprintSensor.image2Tz() != FINGERPRINT_OK)
    //     {
    //         Serial.println(F("Erro image2Tz"));
    //         return;
    //     }

    //     if (fingerprintSensor.fingerFastSearch() != FINGERPRINT_OK)
    //     {
    //         Serial.println(F("Digital não encontrada"));
    //         return;
    //     }

    //     Serial.print(F("Digital encontrada com confiança de "));
    //     Serial.print(fingerprintSensor.confidence);
    //     Serial.print(F(" na posição "));
    //     Serial.println(fingerprintSensor.fingerID);
    // }
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

void Task2code(void *pvParameters)
{
    setupFingerprintSensor();
    getAllFingerprints();

    for (;;)
    {
        uint8_t result = fingerprintSensor.getImage();
        if (result == FINGERPRINT_OK){
            if (fingerprintSensor.image2Tz() != FINGERPRINT_OK)
            {
                Serial.println(F("Erro image2Tz"));
            }

            if (fingerprintSensor.fingerFastSearch() != FINGERPRINT_OK)
            {
                Serial.println(F("Digital não encontrada"));
            }

            Serial.print(F("Digital encontrada com confiança de "));
            Serial.print(fingerprintSensor.confidence);
            Serial.print(F(" na posição "));
            Serial.println(fingerprintSensor.fingerID);
        }

        // if (WiFi.status() == WL_CONNECTED && currentTime - previousTime > timeoutTime)
        if (WiFi.status() == WL_CONNECTED && digitalRead(PIR_PIN) == HIGH)
        {
            Serial.println("sensor pir acionado");
            previousTime = currentTime;
            HTTPClient http;

            http.begin(ipServer + "/lastpos");
            int httpCode = http.GET();
            int posicao = 0;

            if (httpCode > 0)
            {
                Serial.print("httpCode: ");
                Serial.println(httpCode);
                String payload = http.getString();
                Serial.print("payload: ");
                Serial.println(payload);

                DynamicJsonDocument doc(100);
                doc["heap"] = ESP.getFreeHeap();
                deserializeJson(doc, payload);
                posicao = doc["posicao"];

                Serial.print("posicao: ");
                Serial.println(posicao);                
            }
            else
            {
                Serial.println("Error on HTTP request");
            }
            http.end();

            if (posicao != lastpos)
            {
                for (int i = lastpos+1; i <= posicao; i++)
                {
                    Serial.println(ipServer + "/users/template?pos=" + String(i));
                    http.begin(ipServer + "/users/template?pos=" + String(i));
                    int httpCode = http.GET();

                    if (httpCode > 0)
                    {
                        Serial.print("httpCode: ");
                        Serial.println(httpCode);
                        String payload = http.getString();

                        DynamicJsonDocument doc(2048);
                        doc["heap"] = ESP.getFreeHeap();
                        deserializeJson(doc, payload);
                        int posicao = doc["posicao"];
                        String pacote1 = doc["pacote1"];
                        String pacote2 = doc["pacote2"];
                        String pacote3 = doc["pacote3"];
                        String pacote4 = doc["pacote4"];

                        uint8_t packed1[130];
                        uint8_t packed2[130];
                        uint8_t packed3[130];
                        uint8_t packed4[108];

                        int inicio = 0;
                        int fim = -1;
                        uint8_t uint = 0;
                        for (size_t i = 0; i < 130; i++)
                        {
                            inicio = fim;
                            fim = pacote1.indexOf(",",inicio+1);
                            uint = pacote1.substring(inicio+1,fim).toInt();
                            packed1[i]=uint;
                        }
                        
                        inicio = 0;
                        fim = -1;
                        for (size_t i = 0; i < 130; i++)
                        {
                            inicio = fim;
                            fim = pacote2.indexOf(",",inicio+1);
                            uint = pacote2.substring(inicio+1,fim).toInt();
                            packed2[i]=uint;
                        }

                        inicio = 0;
                        fim = -1;
                        for (size_t i = 0; i < 130; i++)
                        {
                            inicio = fim;
                            fim = pacote3.indexOf(",",inicio+1);
                            uint = pacote3.substring(inicio+1,fim).toInt();
                            packed3[i]=uint;
                        }

                        inicio = 0;
                        fim = -1;
                        for (size_t i = 0; i < 108; i++)
                        {
                            inicio = fim;
                            fim = pacote4.indexOf(",",inicio+1);
                            uint = pacote4.substring(inicio+1,fim).toInt();
                            packed4[i]=uint;
                        }

                        if (posicao == NULL)
                        {
                            Serial.println("posicao igual a null");
                            break;
                        }
                        else
                        {
                            lastpos = posicao;
                            uploadFingerprintTemplate(packed1, packed2, packed3, packed4, posicao);
                        }
                    }
                    else
                    {
                        Serial.println("Error on HTTP request");
                    }
                    http.end();
                }
            }
        }
    }
}

void getAllFingerprints()
{
    if (fingerprintSensor.emptyDatabase() != FINGERPRINT_OK)
    {
        Serial.println(F("Erro ao apagar banco de digitais"));
    }

    Serial.println("get all fingerprints");
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        for (int i = 1; i < 5; i++)
        {
            Serial.println(ipServer + "/users/template?pos=" + String(i));
            http.begin(ipServer + "/users/template?pos=" + String(i));
            int httpCode = http.GET();

            if (httpCode > 0)
            {
                Serial.print("httpCode: ");
                Serial.println(httpCode);
                String payload = http.getString();

                DynamicJsonDocument doc(2048);
                doc["heap"] = ESP.getFreeHeap();
                deserializeJson(doc, payload);
                int posicao = doc["posicao"];
                String pacote1 = doc["pacote1"];
                String pacote2 = doc["pacote2"];
                String pacote3 = doc["pacote3"];
                String pacote4 = doc["pacote4"];

                uint8_t packed1[130];
                uint8_t packed2[130];
                uint8_t packed3[130];
                uint8_t packed4[108];

                int inicio = 0;
                int fim = -1;
                uint8_t uint = 0;
                for (size_t i = 0; i < 130; i++)
                {
                    inicio = fim;
                    fim = pacote1.indexOf(",",inicio+1);
                    uint = pacote1.substring(inicio+1,fim).toInt();
                    packed1[i]=uint;
                }
                
                inicio = 0;
                fim = -1;
                for (size_t i = 0; i < 130; i++)
                {
                    inicio = fim;
                    fim = pacote2.indexOf(",",inicio+1);
                    uint = pacote2.substring(inicio+1,fim).toInt();
                    packed2[i]=uint;
                }

                inicio = 0;
                fim = -1;
                for (size_t i = 0; i < 130; i++)
                {
                    inicio = fim;
                    fim = pacote3.indexOf(",",inicio+1);
                    uint = pacote3.substring(inicio+1,fim).toInt();
                    packed3[i]=uint;
                }

                inicio = 0;
                fim = -1;
                for (size_t i = 0; i < 108; i++)
                {
                    inicio = fim;
                    fim = pacote4.indexOf(",",inicio+1);
                    uint = pacote4.substring(inicio+1,fim).toInt();
                    packed4[i]=uint;
                }

                if (posicao == NULL)
                {
                    Serial.println("posicao igual a null");
                    break;
                }
                else
                {
                    lastpos = posicao;
                    uploadFingerprintTemplate(packed1, packed2, packed3, packed4, posicao);
                }
            }
            else
            {
                Serial.println("Error on HTTP request");
            }
            http.end();
        }
    }
    Serial.println("saiu funcao");
}

uint8_t uploadFingerprintTemplate(uint8_t packet1[130], uint8_t packet2[130], uint8_t packet3[130], uint8_t packet4[108], int posicao)
{
    Serial.println("------------------------------------");
    Serial.println("upload biometria");
    setupFingerprintSensor();

    uint8_t p = fingerprintSensor.uploadModelPercobaan(packet1, packet2, packet3, packet4);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.print("Template ");
        Serial.println(" loaded");
        break;
    case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return p;
    default:
        Serial.print("Unknown error ");
        Serial.println(p);
        return p;
    }

    Serial.println("passou");
    p = fingerprintSensor.storeModel(posicao);
    switch (p)
    {
    case FINGERPRINT_OK:
        Serial.print("Template ");
        Serial.println(" transferring:");
        break;
    default:
        Serial.print("Unknown error ");
        Serial.println(p);
        return p;
    }
}

void beepSucesso()
{
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
    digitalWrite(BUZZER_PIN, HIGH);
}

void beepFracasso()
{
    digitalWrite(BUZZER_PIN, LOW);
    delay(1000);
    digitalWrite(BUZZER_PIN, HIGH);
}