// ***************************************************************************************************
// *  Programa de teste para transmitir um dado via POST para ProIoT                                 *
// *                                                                                                 *
// *  Desenvolvido por: David Souza - SmartMosaic - smartmosaic.com.br                               *
// *                    Henrique Romera Salvador                                                     *
// *                                                                                                 *
// *  Versão 1.0 - Julho/2020                                                                        *
// *                                                                                                 *
// ***************************************************************************************************

// ***************************************************************************************************
// *  Definições auxiliares                                                                          *
// ***************************************************************************************************
#define OFF           0
#define ON            1

// ***************************************************************************************************
// *  Definições de Operação                                                                         *
// ***************************************************************************************************
#define BASE_TIME_SEND     20        // Tempo entre transmissões automáticas (s)
#define BASE_TIME_SENSOR   10        // Tempo entre leitura do sensores (s)

#if (BASE_TIME_SENSOR>BASE_TIME_SEND)
  #error (Base de leitura deve ser menor que a base de envio)                             //Base de leitura deve ser menor que a base de envio
#endif

#define NUM_ERROR_ESP 3              // Numero de tentativas quando o ESP dá erro de comunicação

#define USE_WDT       OFF            // Ativa o uso do WDT

#define USE_ESP       ON             // Ativa o uso do módulo ESP8266
#define USE_RESET_ESP ON             // Ativa o Reset do módulo ESP8266

#define DEBUG         ON             //Ativa a opçõa e DEBUG

// ***************************************************************************************************
// *  Definições dos sensores                                                                        *
// ***************************************************************************************************

#define RAND          2
#define LM35          3
#define DHT11         4
#define DHT22         5

// ***************************************************************************************************
// *  Definição da origem dos dados e Alias                                                          *
// ***************************************************************************************************

#define TEMP          DHT22     // Origem da temperatura (RAND. LM35, DHT11, DHT22, OFF)
#define TEMP_ALIAS    "01"      // Alias da temperatura
#define HUMI          DHT22     // Origem da temperatura (RAND. DHT11, DHT22, OFF)
#define HUMI_ALIAS    "02"      // Alias da umidade

// ***************************************************************************************************
// *  Bibliotecas e includes                                                                         *
// ***************************************************************************************************
#if (USE_WDT == ON)
  #include <avr/wdt.h>
#endif

#if (USE_ESP == ON)
  #include "SoftwareSerial.h"
  #define ESP8266_USE_SOFTWARE_SERIAL // Define necessário para que a biblioteca ESP8266 funcione com Software Serial
  #include "ESP8266.h"
#endif

// ***************************************************************************************************
// *  Portas e comunicação com os periféricos                                                        *
// ***************************************************************************************************

#if (USE_ESP == ON)
  //Definição da porta de comunicação com ESP8226
  SoftwareSerial ESP_Serial(10, 11); // RX, TX
  
  //Cria objeto de conexão wifi com o módulo, usando a Serial1 do Mega.
  ESP8266 wifi(ESP_Serial);

  #if (USE_RESET_ESP == ON)
    //Define o pino 9 do Arduino como o pino de RESET para o módulo ESP
    #define PIN_RESET_ESP  9
  #endif
#endif

// ***************************************************************************************************
// *  Dados fixos da plataforma PROIOT                                                               *
// ***************************************************************************************************
//Definição das variáveis do servidor
#define HOST_NAME  "things.proiot.network"
#define HOST_PORT   (80)

// ***************************************************************************************************
// *  Arquivo de chaves externas (caso exista)                                                       *
// ***************************************************************************************************
#include "chaves.h"                    //Use esse arquivo adicinal para deixar suas chaves separadas 
                                       //separadas do código principal

// ***************************************************************************************************
// *  Definição das chaves internas (caso não exista arquivo externo)                                *
// ***************************************************************************************************
//As strings abaixo devem ser alteradas de acordo com os dados de sua rede
#ifndef SSID
  #define SSID        "SUA_REDE"
#endif
#ifndef PASSWORD
  #define PASSWORD    "SUA_SENHA"
#endif

//A strings abaixo devem ser alteradas de acordo com as configrações da sua conta na PROIOT
#ifndef TOKEN
  #define TOKEN     "SEU_TOKEN"
#endif
#ifndef NODE
  #define NODE      "SEU_NODE"
#endif

// ***************************************************************************************************
// *  Variáveis globais                                                                              *
// ***************************************************************************************************
int  base_time_send = BASE_TIME_SEND;   // Contador de tempo para transmissão base de tempo (1 seg)
int  base_time_sensor = BASE_TIME_SENSOR;   // Contador de tempo para transmissão base de tempo (1 seg)

float temperature = 0;        // Valor da temperatura
float humidity = 0;           // valor da umidade

int num_var = 3;

// ***************************************************************************************************
// *  Variáveis do Timer2  (Temporizador)                                                            *
// ***************************************************************************************************
byte t2_ps;                   // Prescalar do Timer2 para contagem de 1 segundo
bool t2_fs;                   // Flag indicando que passou 1 segundo
bool t2_fm;                   // Flag indicando que passou 1 minuto
bool t2_fh;                   // Flag indicando que passou 1 hora
byte t2_seg;                  // Contador de segundos
byte t2_min;                  // Contador de minutos

// ***************************************************************************************************
// *  Estrutura de dados para informações dos sensores                                               *
// ***************************************************************************************************
typedef struct data_var
{
  String Alias;
  float valor;            //A variável value já existe, para não causar confusão, foi usado em pt-br valor
  int sensor;
} data_var;
data_var var[num_var];

// ***************************************************************************************************
// *  Função de SETUP do sistema                                                                     *
// ***************************************************************************************************
void setup(void)
{
  #if (USE_WDT == ON)
    // Ativa o WDT
    wdt_enable(WDTO_2S);
  #endif

  // Cria uma semente para o gerador de números randômicos
  randomSeed(500);
  
  //Configura timer2
  setupTimer2();

  //Inicializa porta serial do DEBUG
  Serial.begin(9600);

  #if (USE_ESP == ON)
    #if (USE_RESET_ESP==ON)
      // Ativa o pino de reset do ESP
      digitalWrite (PIN_RESET_ESP, HIGH);
      pinMode(PIN_RESET_ESP, OUTPUT);
    #endif
    
    // Inicializa o módulo ESP8266
    reset_esp();
  #endif

  #if (USE_LM35 == ON)
    // Acerta pinagem para sensor LM25
    pinMode(PIN_LM35, INPUT); 
  #endif

  #if (USE_DHT11 == ON || USE_DHT22 == ON)
    // Acerta pinagem para sensor DHT
    pinMode(PIN_DHT, INPUT_PULLUP); 
  #endif            
}

// ***************************************************************************************************
// *  LOOP principal                                                                                 *
// ***************************************************************************************************
// Quando a base de tempo é atingida, lê sensores e envia os dados

void loop(void)
{
  #if (USE_WDT == ON)
    // Reset de WDT
    wdt_reset();
  #endif

  // Base de tempo ajustada
  // Checa temporizador para 1 segundo
  if (t2_fs) {
    // Limpa flag de segundos
    t2_fs = false;

    // Checa inicio da base de tempo
    if (base_time_send == BASE_TIME_SEND) {
      #if (DEBUG == ON)
        Serial.print(F("Aguarda "));
        Serial.print(BASE_TIME_SEND);
        Serial.print("s:");
     #endif
    } else {
      // Immprime "." para mostrar que esta rodando
      #if (DEBUG == ON)
        Serial.print((base_time_send));
        Serial.print(F("."));
      #endif
    }
    
    if (base_time_sensor == 0)
    {
      base_time_sensor = BASE_TIME_SENSOR-1;  // Reinicia base de tempo

            // *********** Comando repetidos na base de tempo *************
            
      // Leitura dos sensores
      #if (DEBUG == ON)
        Serial.println("\n\rLendo sensores");
      #endif
      read_sensors();
    }
    else
    {
      base_time_sensor--;
    }
    
    //Checa término da base de tempo
    if (base_time_send == 0) {
      base_time_send = BASE_TIME_SEND;   // Reinicia base de tempo
      base_time_sensor = BASE_TIME_SENSOR;  // Reinicia base de tempo

      // *********** Comando repetidos na base de tempo *************
   
      // Transmissão dos dados
      send_data();

      // ************************************************************
      
    } else {
      base_time_send--;             // Decrementa contador de tempo
    }
  }
}

// ***************************************************************************************************
// *  Função de leitura de sensores                                                                  *
// ***************************************************************************************************
void send_data(void)
{

  var[0].Alias = TEMP_ALIAS;
  var[0].valor = temperature;
  var[0].sensor = DHT22;

  var[1].Alias = HUMI_ALIAS;
  var[1].valor = humidity;
  var[1].sensor = LM35;
  
  // Envio da temperatura
  if (TEMP != OFF)
    Serial.println(var[0].Alias);
    Serial.println(var[0].valor);
    Serial.println(var[0].sensor);

    Serial.println(var[1].Alias);
    Serial.println(var[1].valor);
    Serial.println(var[1].sensor);

    send_TCP(var[0].valor, var[0].Alias);
    //send_TCP(temperature, TEMP_ALIAS);

  // Envio da umidade
  if (HUMI != OFF)
    send_TCP(humidity,HUMI_ALIAS);
}

// ***************************************************************************************************
// *  Função de envio de dados TCP                                                                   *
// ***************************************************************************************************
void send_TCP(float value, String ALIAS)
{
  //Variável para buffer de dados de recepção/trasmissão
  uint8_t buffer[300] = {0};
  
  //Cria conexão TCP
  if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
    #if (DEBUG == ON)
      Serial.print(F("Conexão TCP criada com sucesso na porta "));
      Serial.print(HOST_PORT);
      Serial.println(F("."));
    #endif
  } else {
    #if (DEBUG == ON)
      Serial.println(F("Erro ao criar conexão TCP."));
    #endif
    #if(USE_RESET_ESP == ON)        //Se a opção do pino de reset estiver ON
      reset_esp();                  //Reseta o módulo ESP
    #endif
  }

  //Preparação do pacote de dados
  String dados = "POST ";
  dados += "/stream/device/";
  dados += NODE;
  dados += "/variable/";
  dados += ALIAS;
  dados += "/";
  dados += value;
  dados += " HTTP/1.1\r\n";
  dados += "Host: ";
  dados += HOST_NAME;
  dados += "\r\n";
  dados += "Authorization: ";
  dados += TOKEN;
  dados += "\r\n";
  //dados += "Connection: Keep-Alive\r\n"; //mantem a conexão ativa para o envio do segundo header
  dados += "\r\n\r\n";

  //Envio do pacote de dados
  #if (DEBUG == ON)
    Serial.println(F("Enviando pacote de dados..."));
    Serial.print(dados);
  #endif
  int num = NUM_ERROR_ESP;
  char resp = true;
  do {
    resp = wifi.send((const uint8_t*)dados.c_str(), dados.length());
    if (resp==false) {
      #if (DEBUG == ON)
        Serial.print(F("Erro na tentativa de envio de dados pelo ESP8266"));
      #endif
    } 
    num--;
  } while (resp==false && num>0);
  
  //Recepção da resposta de retorno
  uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
  if (len > 0) {
    #if (DEBUG == ON)
      Serial.println(F("Recebido retorno:"));
    #endif
    for (uint32_t i = 0; i < len; i++) {
      #if (DEBUG == ON)
        Serial.print((char)buffer[i]);
      #endif
    }
    #if (DEBUG == ON)
      Serial.print(F("\r\n"));
    #endif
  }
  else {
    #if (DEBUG == ON)
      Serial.println(F("Não recebido retorno."));
    #endif
  }

  delay(1000);

  //Liberação da conexão TCP
  if (wifi.releaseTCP()) {
    #if (DEBUG == ON)
      Serial.println(F("Conexao TCP liberada com sucesso."));
    #endif
  } else {
    #if (DEBUG == ON)
      Serial.println(F("Erro ao liberar conexao TCP."));
    #endif
    #if(USE_RESET_ESP == ON)        //Se a opção do pino de reset estiver ON
      reset_esp();                  //Reseta o módulo ESP
    #endif
  }
}

// ***************************************************************************************************
// *  Função de reset do módulo ESP8266                                                              *
// ***************************************************************************************************
#if (USE_ESP == ON)
void reset_esp(void)
{
  int num = NUM_ERROR_ESP;
  while (!init_esp() && num>0){
    #if (DEBUG == ON)
      Serial.println(F("Erro ao conectar, resetando o ESP8266!"));  //Mensagem de erro
    #endif
    #if (USE_RESET_ESP)
      #if (DEBUG == ON)
        digitalWrite(PIN_RESET_ESP, LOW);                             //Manda sinal 1 para o pino de reset
      #endif
        delay(200);                                                   //Espera 200ms
      #if (DEBUG == ON)
        digitalWrite(PIN_RESET_ESP, HIGH);                            //Manda o sinal 0 para o pino de reset
      #endif
        #endif
    delay(2000);                                                  //Espera 2s
    num--;
  }
}
#endif

// ***************************************************************************************************
// *  Função de inicialização do módulo EP8266                                                       *
// ***************************************************************************************************
#if (USE_ESP == ON)
char init_esp(void) {
  //#if (DEBUG == ON)
    Serial.println("\nIniciando Setup.");
  //#endif

  //Checa versão do ESP8266
  #if (DEBUG == ON)
    Serial.print("Versao de Firmware do ESP8266: ");
  #endif
  //A funcao wifi.getVersion() retorna a versao de firmware informada pelo modulo no inicio da comunicacao
  #if (DEBUG == ON)
    Serial.println(wifi.getVersion().c_str());
  #endif

  //Setup do módulo para operar em modo Station (conecta em WiFi)
  if (wifi.setOprToStation()) {
    #if (DEBUG == ON)
      Serial.println("Modo Station configurado com sucesso.");
    #endif
  }
  else {
    #if (DEBUG == ON)
      Serial.println("Erro em configurar Station.");
    #endif
    #if(USE_RESET_ESP == ON)        //Se a opção do pino de reset estiver ON
      return false;
    #endif
  }

  //Estabelece conexão com a rede WIFI
  if (wifi.joinAP(SSID, PASSWORD)) {
    #if (DEBUG == ON)
      Serial.println("Conectado no WiFi com Sucesso.");
      Serial.print("IP: ");
      Serial.println(wifi.getLocalIP().c_str());
    #endif
  } else {
    #if (DEBUG == ON)
      Serial.println("Falha na conexao WiFi.");
    #endif
      #if(USE_RESET_ESP == ON)        //Se a opção do pino de reset estiver ON
        return false;
      #endif
  }

  // desabilita funcionalidade MUX
  if (wifi.disableMUX()) {
    #if (DEBUG == ON)
      Serial.println("Multiplas conexoes desabilitadas.");
    #endif
  } else {
    #if (DEBUG == ON)
      Serial.println("Erro ao desabilitar multiplas conexoes.");
    #endif
    #if(USE_RESET_ESP == ON)        //Se a opção do pino de reset estiver ON
      return false;
    #endif
  }
  #if (DEBUG == ON)
    Serial.println("Setup finalizado!");
    Serial.println("***********************************");
  #endif
  return true;
}
#endif
