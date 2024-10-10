#define BLYNK_TEMPLATE_ID "TMPL2DHiOIXEl"                    // Id do template
#define BLYNK_TEMPLATE_NAME "Micro Clima"                    // Nome
#define BLYNK_AUTH_TOKEN "EslLm5GJGVmnNwYZknU0bpn-xYejVJei"  // Auth Token

#include <ESP8266WiFi.h>        //Biblioteca para conectar o módulo ESP8266 a uma rede Wi-Fi
#include <BlynkSimpleEsp8266.h> //Biblioteca para uso do ESP8266 na plataforma Blynk
#include <DHT.h>                //Biblioteca para sensor DHT11
#include <SFE_BMP180.h>         //Biblioteca para sensor BMP180
#include <Wire.h>               //Biblioteca para comunicação I2C

// Configurações do aplicativo e da rede Wi-Fi
char auth[] = BLYNK_AUTH_TOKEN;           //Armazena o AuthToken no array auth
char ssid[] = "*****";                   //entre com a sua rede WiFi
char pass[] = "*****";  				//entre com a senha da rede WiFi

// Temperatura e umidade
#define DHTPIN D3          		//Pino D3 será responsável pela leitura do DHT11
#define DHTTYPE DHT11      		//Define o DHT11 como o sensor a ser utilizado pela biblioteca <DHT.h>
DHT dht(DHTPIN, DHTTYPE);  		//Inicializando o objeto dht do tipo DHT passando como parâmetro o pino (DHTPIN) e o tipo do sensor (DHTTYPE)
float u = 0.0;             		//Variável responsável por armazenar a umidade lida pelo DHT11
float t = 0.0;             		//Variável responsável por armazenar a temperatura lida pelo DHT11

// Pressão atmosfética
SFE_BMP180 sensorP;             //Define objeto sensorP na classe SFE_BMP180 da biblioteca
#define ALTITUDE 727            // Altitude da casa da robótica em metros
char status;                    //Variável auxiliar para verificação do resultado
double temperatura;             //variável para armazenar a temperatura
double pressao_abs;             //variável para armazenar a pressão absoluta
double pressao_relativa = 0.0;  //variável para armazenar a pressão relativa

// Qualidade do ar
#define sensorAr A0  			//Atribui o analógico A0 à variável sensorPin
float m = -0.3376;   			//Parâmetro Slope
float b = 0.7165;    			//Parâmetro Y-Intercept
float R0 = 12.89;    			//Resistência R0 encontrada no código de calibração
float sensor_volt;   			//Define variável sensor_volt para armazenar a tensão do sensor
float Rs;            			//Define variavél Rs para armazenar a resistência do sensor
float razao;         			//Define variavel para armazenar o valor da razão entre Rs e R0
float ppm_log;       			//Variável para armazenar o valor de ppm em escala linear
float ppm = 0.0;     			// Variável para armazenar o valor de ppm em escala logarítmica

//alarmes
float pinGreen = D5;
float pinYellow = D6;
int pinRed = D7;
int pinBuzzer = D4;
WidgetLED ledG(V5);
WidgetLED ledY(V6);
WidgetLED ledR(V7);

// Função para leitura da temperatura e umidade - Sensor DHT11
void sensorDHT() {
  u = dht.readHumidity();       //Realiza a leitura da umidade
  t = dht.readTemperature();    //Realiza a leitura da temperatura
  Serial.print("Umidade: ");
  Serial.println(u);  		    //Imprime na serial a umidade
  Serial.print("Temperatura: ");
  Serial.println(t);            //Imprime na serial a temperatura
  Blynk.virtualWrite(V0, t);    //Escreve no pino virtual V0 a temperatura em graus Celsius
  Blynk.virtualWrite(V1, u);    //Escreve no pino virtual V1 a umidade em porcentagem
}

// Função para leitura da pressão absoluta e relativa - Sensor BMP180
void Pressao() {
  status = sensorP.startTemperature();                                 //Inicializa a leitura da temperatura
  if (status != 0) {                                                   //Se status for diferente de zero (sem erro de leitura)
    delay(status);                                                     //Realiza uma pequena pausa para que a leitura seja finalizada
    status = sensorP.getTemperature(temperatura);                      //Armazena o valor da temperatura na variável temperatura
    if (status != 0) {                             					   //se status for diferente de zero (sem erro de leitura)

      //Leitura da Pressão Absoluta
      status = sensorP.startPressure(3);                               //Inicializa a leitura
      if (status != 0) {                                               //se status for diferente de zero (sem erro de leitura)
        delay(status);                                                 //Realiza uma pequena pausa para que a leitura seja finalizada
        status = sensorP.getPressure(pressao_abs, temperatura);  	   //Atribui o valor medido de pressão à variável pressao, em função da variável temperatura
        Serial.print("Pressão absoluta: ");
        Serial.println(pressao_abs, 1);                                //Imprime na serial a pressão absoluta
        if (status != 0) {                                             //se status for diferente de zero (sem erro de leitura)
          pressao_relativa = sensorP.sealevel(pressao_abs, ALTITUDE);  //Atribui o valor medido de pressão relativa à variavel pressao_relativa, em função da ALTITUDE
          Serial.print("Pressão relativa: ");
          Serial.println(pressao_relativa, 1);  					   //Imprime na serial a pressão relativa
        }
      }
    }
  }
  Blynk.virtualWrite(V2, pressao_relativa);  //Escreve no pino virtual V2 a pressão relativa ao nível do mar em hpa
  Blynk.virtualWrite(V3, pressao_abs);       //Escreve no pino virtual V3 a pressão absoluta em
}

// Função para leitura da qualidade do ar - Sensor MQ-135
void qualidadeAr() {
  sensor_volt = analogRead(sensorAr) * (5.0 / 1023.0);
  Rs = ((5.0 * 10.0) / sensor_volt) - 10.0;  //Cálculo para obter a resistência Rs do sensor
  razao = Rs / R0;                           //Calcula a razão entre Rs/R0
  ppm_log = (log10(razao) - b) / m;          //Cálculo para obter o valor de ppm em escala linear de acordo com o valor de razao
  ppm = pow(10, ppm_log);                    //Converte o valor de ppm para escala logarítmica
  Serial.print("Qualidade do Ar: ");
  Serial.println(ppm);         				 //Imprime na serial a qualidade do ar
  Blynk.virtualWrite(V4, ppm); 				 //Escreve no pino virtual V4 a qualidade do ar em ppm
}

void setup() {
  Serial.begin(115200);          		     //Inicializa a comunicação serial
  Blynk.begin(auth, ssid, pass);		     //Inicializa o Blynk passando como parâmetros o auth token, o nome da rede Wi-Fi e a senha
  sensorP.begin();             			     //Inicializa o sensor de pressão atmosférica
  dht.begin();                		         //Inicializa o sensor DHT11
  pinMode(sensorAr, INPUT);	 		         //Configura de sensorAr como entrada
  pinMode(pinGreen, OUTPUT);				 //configura pino de saida para led
  pinMode(pinYellow, OUTPUT);				 //configura pino de saida para led
  pinMode(pinRed, OUTPUT);					 //configura pino de saida para led
  pinMode(pinBuzzer, OUTPUT);				 //configura pino de saida para alarme
  WidgetLED ledG(V5);						 //configura porta escuta no Blynk
  WidgetLED ledY(V6);						 //configura porta escuta no Blynk
  WidgetLED ledR(V7);						 //configura porta escuta no Blynk
}

// logica do loop para monitoramento e alarme das condições de clima

void loop() {

//comparador para o primeiro nivél de clima, condição boa
  if ((u > 40) && (35 > t)) {
    digitalWrite(pinGreen, HIGH);
    digitalWrite(pinYellow, LOW);
    digitalWrite(pinRed, LOW);
  }

//comparador para o segundo nivel de clima, condição de alerta 
  if (((40 > u) && (u > 20)) || ((40 > t) && (t > 35))) {
    digitalWrite(pinGreen, LOW);
    digitalWrite(pinYellow, HIGH);
    digitalWrite(pinRed, LOW);
  }

//comparador para o terceiro nivel de clima, condição de risco
  if ((20 > u) || (t > 40)) {
    digitalWrite(pinGreen, LOW);
    digitalWrite(pinYellow, LOW);
    digitalWrite(pinRed, HIGH);
  }


//sequencia para comparador de qualidade do ar, presensa de gás glp com integração de alarme sonoro e optico
  if (ppm > 100) {
    digitalWrite(pinGreen, LOW);
    digitalWrite(pinYellow, HIGH);
    delay(150);
    digitalWrite(pinYellow, LOW);
    digitalWrite(pinRed, HIGH);
    delay(150);
    digitalWrite(pinRed, LOW);
    digitalWrite(pinBuzzer, HIGH);
  }

  if (100 > ppm) {
    digitalWrite(pinBuzzer, LOW);
  }

  if (digitalRead(pinGreen)) {
    ledG.on();
  } else {
    ledG.off();
  }

  if (digitalRead(pinGreen)) {
    ledY.on();
  } else {
    ledY.off();
  }

  if (digitalRead(pinGreen)) {
    ledR.on();
  } else {
    ledR.off();
  }


  Blynk.run();  //Chama a função Blynk.run
  Serial.println("====== Estação Meteorológica IoT ======");
  sensorDHT();    //Chama a função sensorDHT
  qualidadeAr();  //Chama a função qualidadeAr
  Pressao();      //Chama a função Pressao
  Serial.println("");
}
