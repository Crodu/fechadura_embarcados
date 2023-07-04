#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Keypad.h>
#include <MFRC522.h>
#include "api.h"

#define BUZZER_PIN 15
#define RELE_PIN 2

#define LDR_PIN 35
#define BUTTON_PIN 34

#define SS_PIN 21
#define RST_PIN 22
#define LED_BRANCO 4

int LDR_Val = 0;

const uint8_t ROWS = 4;
const uint8_t COLS = 4;
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
uint8_t colPins[COLS] = { 26, 25, 33, 32 };
uint8_t rowPins[ROWS] = { 13, 12, 14, 27 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const char *ssid = "Soratto C12";
const char *password = "gatopreto";

IPAddress local_IP(192, 168, 0, 201);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

String header;
String biometria;
String userRFID;
String senhaTeclado = "";
int posicaoBiometria = 0;
int lastpos = 1;

MFRC522 mfrc522(SS_PIN, RST_PIN);

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

const uint32_t passwordFingerprint = 0x0;
Adafruit_Fingerprint fingerprintSensor = Adafruit_Fingerprint(&Serial2, passwordFingerprint);
boolean cadastro = false;

TaskHandle_t Task2;

String ipServer = "http://192.168.0.88:8080";

HTTPClient httpClient;

Api api(ipServer, httpClient);

void setupFingerprintSensor();
void getFingerprints(int firstPos, int lastPos);
void Task2code(void *pvParameters);
uint8_t uploadFingerprintTemplate(uint8_t packet1[130], uint8_t packet2[130], uint8_t packet3[130], uint8_t packet4[108], int posicao);
void beepSucesso();
void beepFracasso();
void beepSincroniza();
void setupRfid();
boolean handleRfid();

void autentica(int tipo);

void setup()
{
    Serial.begin(9600);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, HIGH);
    pinMode(RELE_PIN, OUTPUT);

    pinMode(LDR_PIN, INPUT);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_BRANCO, OUTPUT);

    setupRfid();

    httpClient.setTimeout(5000);

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

    // HTTPClient http;

    boolean hasCard = handleRfid();
    if (hasCard){
        Serial.println("entrou rfid");
        autentica(1);
    }

    LDR_Val = analogRead(LDR_PIN);
    if (LDR_Val > 600)
    {
        digitalWrite(LED_BRANCO, HIGH);
    }
    else{
        digitalWrite(LED_BRANCO, LOW);
    }

    char key = keypad.getKey();
    if (key)
    {
        Serial.print("Tecla: ");
        Serial.println(key);
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
            autentica(0);
        }
    }
}

void setupRfid(){
    SPI.begin();
    mfrc522.PCD_Init();
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
    if (fingerprintSensor.emptyDatabase() != FINGERPRINT_OK)
    {
        Serial.println(F("Erro ao apagar banco de digitais"));
    }
    // getFingerprints(1, 150);

    for (;;)
    {
        uint8_t result = fingerprintSensor.getImage();
        if (result == FINGERPRINT_OK){
            if (fingerprintSensor.image2Tz() != FINGERPRINT_OK)
            {
                Serial.println(F("Erro image2Tz"));
                beepFracasso();
            }

            if (fingerprintSensor.fingerFastSearch() != FINGERPRINT_OK)
            {
                Serial.println(F("Digital não encontrada"));
                beepFracasso();
            }
            else{
                posicaoBiometria = fingerprintSensor.fingerID;
                autentica(2);
                Serial.print(F("Digital encontrada com confiança de "));
                Serial.print(fingerprintSensor.confidence);
                Serial.print(F(" na posição "));
                Serial.println(fingerprintSensor.fingerID); 
            }
        }

        // if (WiFi.status() == WL_CONNECTED && currentTime - previousTime > timeoutTime)
        if (WiFi.status() == WL_CONNECTED && digitalRead(BUTTON_PIN) == HIGH)
        {
            Serial.println("sensor pir acionado");
            previousTime = currentTime;

            String payload = api.get("/users/lastpos", "");
            int posicao = 0;

            if (payload != NULL)
            {
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

            if (posicao != lastpos)
            {
                getFingerprints(lastpos, posicao);
                beepSincroniza();
            }
        }
    }
}

void getFingerprints(int firstPos, int lastPos)
{
    Serial.println("get all fingerprints");
    if (WiFi.status() == WL_CONNECTED)
    {
        for (int i = firstPos; i < lastPos; i++)
        {
            String payload = api.get("/users/template?pos=", String(i));

            if (payload != NULL)
            {
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
        }
    }
    Serial.println("saiu funcao");
    beepSincroniza();
}

uint8_t uploadFingerprintTemplate(uint8_t packet1[130], uint8_t packet2[130], uint8_t packet3[130], uint8_t packet4[108], int posicao)
{
    Serial.println("------------------------------------");
    Serial.println("upload biometria");

    setupFingerprintSensor();

    uint8_t pfp = fingerprintSensor.uploadModelPercobaan(packet1, packet2, packet3, packet4);

    Serial.println(pfp);

    // change switch to if else
    if (pfp == FINGERPRINT_OK){
        Serial.print("Template ");
        Serial.println(" loaded");
    }else if (pfp == FINGERPRINT_PACKETRECIEVEERR){
        Serial.println("Communication error");
        return pfp;
    }else {
        Serial.print("Unknown error ");
        Serial.println(pfp);
        return pfp;
    }

    Serial.println("passou");
    uint16_t p = fingerprintSensor.storeModel(posicao);
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
    digitalWrite(RELE_PIN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(2000);
    digitalWrite(RELE_PIN, LOW);
}

void beepFracasso()
{
    digitalWrite(BUZZER_PIN, LOW);
    delay(1000);
    digitalWrite(BUZZER_PIN, HIGH);
}

void beepSincroniza()
{
    digitalWrite(BUZZER_PIN, LOW);
    delay(50);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(50);
    digitalWrite(BUZZER_PIN, LOW);
    delay(50);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(50);
    digitalWrite(RELE_PIN, LOW);
}

boolean handleRfid(){
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
    byte block;
    byte len;
    MFRC522::StatusCode status;
    if (! mfrc522.PICC_IsNewCardPresent()) {
        // Serial.println("erro is new card");
        return false;
    }

    if ( ! mfrc522.PICC_ReadCardSerial()) {
        // Serial.println("erro read card");
        return false;
    }
    String userid;
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        userid += String(mfrc522.uid.uidByte[i], HEX);
    }
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    userRFID = userid;
    return true;
}

void autentica(int tipo){

    if (WiFi.status() == WL_CONNECTED)
    {

        bool result = false;

        if (tipo == 0){
            result = api.send("/users/auth?password=", senhaTeclado);
        }
        else if(tipo == 1){
            result = api.send("/users/auth?rfid=", userRFID);
        }
        else if(tipo == 2){
            result = api.send("/users/auth?posicao=", String(posicaoBiometria));
        }

        if (result)
        {
            if (result){
                Serial.println("Sucesso");
                beepSucesso();
            }
            else{
                Serial.println("Erro");
                beepFracasso();
            }
        }
        else
        {
            Serial.println("Error on HTTP request");
        }
    }
    senhaTeclado = "";
}