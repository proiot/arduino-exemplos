// ***************************************************************************************************
// *  Programa de teste para receber qualquer dado no endereço de IP do node e repetir no terminal   *
// *                                                                                                 *
// *  Enviar os dados através do PUTTY, porta HOST_PORT, tipo RAW                                    *
// ***************************************************************************************************

// ***************************************************************************************************
// *  Definições auxiliares                                                                          *
// ***************************************************************************************************
#define OFF           0
#define ON            1

// ***************************************************************************************************
// *  Definições de Operação                                                                         *
// ***************************************************************************************************
#define CLOSE_TCP OFF


// ***************************************************************************************************
// *  Bibliotecas                                                                                    *
// ***************************************************************************************************
#include "SoftwareSerial.h"
#include "ESP8266.h"

// ***************************************************************************************************
// *  Portas e comunicação com os periféricos                                                        *
// ***************************************************************************************************
//definição da porta de comunicação com ESP8226
SoftwareSerial ESP_Serial(10, 11); // RX, TX
//Cria objeto de conexão wifi com o módulo, usando a Serial1 do Mega.
ESP8266 wifi(ESP_Serial);

// ***************************************************************************************************
// *  Dados da rede Wi-Fi                                                                            *
// ***************************************************************************************************
//As strings abaixo devem ser alteradas de acordo com os dados de sua rede
#define SSID        ""
#define PASSWORD    ""
nclude "wifi.h"                  //Comente essa linha se inserir os dados nas linhas superiores

// ***************************************************************************************************
// *  Dados da plataforma PROIOT                                                                     *
// ***************************************************************************************************
//Definição das variáveis do servidor
#define HOST_NAME  "things.proiot.network";
#define HOST_PORT  8090

//Constantes para o envio do Header HTTP
//todas as strings abaixo devem ser alteradas de acordo com as configracoes de seu projeto
#define TOKEN     "cWX3Aw2cE6G87d22sjJJtWAIWxO03q"
#define NODE      "ard-http-04"
#define ALIAS     "01"
#define VALUE     "25"
#define PAUSE      (5)

//definição da variável de leitura da resposta do servidor
String resposta = "";

// ***************************************************************************************************
// *  Função de SETUP do sistema                                                                     *
// ***************************************************************************************************
void setup(void)
{
    //Aguarda estabilização do módulo ESP8266
    delay(500);

    //Inicializa porta serial do DEBUG
    Serial.begin(9600);
    Serial.println("Iniciando Setup.");

    //Checa versão do ESP8266
    Serial.print("Versao de Firmware do ESP8266: ");
    //A funcao wifi.getVersion() retorna a versao de firmware informada pelo modulo no inicio da comunicacao
    Serial.println(wifi.getVersion().c_str());

    //Setup do módulo para operar em modo Station (conecta em WiFi)
    if (wifi.setOprToStation()) {
        Serial.println("Station OK.");
    } else {
        Serial.println("Erro em configurar Station.");
    }
 
    //Estabelece conexão com a rede WIFI
    if (wifi.joinAP(SSID, PASSWORD)) {
        Serial.println("Conectado com Sucesso.");
        Serial.print("IP: ");
        Serial.println(wifi.getLocalIP().c_str());    
    } else {
        Serial.println("Falha na conexao AP.");
    }
    
    //Habilia a funcionalidade MUX, que permite a realização de várias conexõess TCP/UDP simultâneas
    if (wifi.enableMUX()) {
        Serial.println("Multiplas conexoes OK.");
    } else {
        Serial.println("Erro ao configurar multiplas conexoes.");
    }
    
    //Inicia servidor TCP na porta correta (função "startServer(numero_porta)" serve para UDP!
    if (wifi.startTCPServer(HOST_PORT)) {
        Serial.print("Servidor TPC iniciado com sucesso na porta: ");
        Serial.println(HOST_PORT);
    } else {
        Serial.println("Erro ao iniciar servidor.");
    }    
    Serial.println("Setup finalizado!");
    Serial.println("***********************************");
    Serial.println("Aguardando recepção de dados");
}
 

// ***************************************************************************************************
// *  LOOP principal                                                                                 *
// ***************************************************************************************************
//Na conexão TCP, basicamente a funcionalidade a ser mostrada será a de "echo", ou seja,
//a aplicação irá retornar todos os dados enviados para ela via socket TCP.

void loop(void)
{
    //Variável para buffer de dados de recepção/trasmissão
    uint8_t buffer[128] = {0};

    //Cada conexão TCP tem sua ID, e precisa ser armazenada para referência no programa. 
    //Usamos essa variavel para isso.
    uint8_t mux_id;

    //A variável len serve para armazenar o comprimento de dados recebidos por meio da rotina wifi.recv(), 
    //que também associa ao buffer os dados recebidos e ao mux_id a id responsável pela transmissão
    uint32_t len = wifi.recv(&mux_id, buffer, sizeof(buffer), 100);
    
    //Caso tenha recebido algum dado
    if (len > 0) {
        //Mostra no terminal (DEBUG) o dado recebido
        Serial.print("Status:[");
        Serial.print(wifi.getIPStatus().c_str());
        Serial.println("]");
        
        Serial.print("Recebido de :");
        Serial.print(mux_id);
        Serial.print("[");
        for(uint32_t i = 0; i < len; i++) {
            Serial.print((char)buffer[i]);
        }
        Serial.print("]\r\n");
        
        //Envia o mesmo dado de volta para quem abriu a conexão TCP.
        //A referência para o socket TCP criado é o mux_id, ou id da conexão, usado na rotina wifi.send
        if(wifi.send(mux_id, buffer, len)) {
            Serial.print("Enviado de volta...\r\n");
        } else {
            Serial.print("Erro ao enviar de volta\r\n");
        }

        #if (CLOSE_TPC==ON)
          //Liberação da conexão TCP, de modo a permitir que conexões diferentes sejam realizadas.
          if (wifi.releaseTCP(mux_id)) {
              Serial.print("Liberando conexao TCP com ID: ");
              Serial.print(mux_id);
              Serial.println(" OK");
          } else {
              Serial.print("Erro ao liberar TCP com ID: ");
              Serial.print(mux_id);
          }
          //Finaliza mostrando o Status de liberação [4]
          Serial.print("Status:[");
          Serial.print(wifi.getIPStatus().c_str());
          Serial.println("]");
        #else
          Serial.println("***********************************");
          Serial.println("Aguardando nova recepção de dados");
        #endif

    }
}
