// ***************************************************************************************************
// *  Programa de teste para transmitir um dado via POST para ProIoT                                 *
// *                                                                                                 *
// *  Desenvolvido por David Souza - SmartMosaic - smartmosaic.com.br                                *
// *  Versão 1.0 - Junho/2020                                                                        *
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
#define TCP_SERVER    OFF       // Inicia servidor TCP
#define CLOSE_TCP     ON        // Fecha conexão TCP após retransmissão dos dados (ECHO)
#define BASE_TIME     30        // Tempo entre transmissões automáticas (s)

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
// *  Dados da rede WiFi                                                                             *
// ***************************************************************************************************
//As strings abaixo devem ser alteradas de acordo com os dados de sua rede
#define SSID        "SUA_REDE"
#define PASSWORD    "SUA_SENHA"

// ***************************************************************************************************
// *  Dados da plataforma PROIOT                                                                     *
// ***************************************************************************************************
//Definição das variáveis do servidor
#define HOST_NAME  "things.proiot.network"
#define HOST_PORT   (80)

//Constantes para o envio do Header HTTP
//todas as strings abaixo devem ser alteradas de acordo com as configracoes de seu projeto
#define TOKEN     "SEU_TOKEM"
#define NODE      "SEU_NODE"
#define ALIAS     "01"
#define VAL_MIN   (0)        // valor minimo para o gerador randômico
#define VAL_MAX   (40)       // valor máximo para o gerador randômico
#define VAL_FAC   (1)        // valor do fator multiplicado

// ***************************************************************************************************
// *  Arquivo de chaves externas que sobreescreve dados da rede e do node                            *
// ***************************************************************************************************
#include "chaves.h"                  //Comente essa linha se inserir os dados nas linhas superiores

//definição da variável de leitura da resposta do servidor
String resposta = "";

// ***************************************************************************************************
// *  Variáveis globais                                                                              *
// ***************************************************************************************************
int  base_time = BASE_TIME;   // Contador de tempo para transmissão base de tempo (1 seg)           

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
// *  Função de SETUP do sistema                                                                     *
// ***************************************************************************************************
void setup(void)
{
  // Cria uma semente para o gerador de números randômicos
  randomSeed(500);
  //Configura timer2
  setupTimer2();  
   
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
      Serial.println("Modo Station configurado com sucesso.");
  } else {
      Serial.println("Erro em configurar Station.");
  }
  
  //Estabelece conexão com a rede WIFI
  if (wifi.joinAP(SSID, PASSWORD)) {
      Serial.println("Conectado no WiFi com Sucesso.");
      Serial.print("IP: ");
      Serial.println(wifi.getLocalIP().c_str());    
  } else {
      Serial.println("Falha na conexao WiFi.");
  }
  
  #if (TCP_SERVER==ON)
    //Habilita a funcionalidade MUX, que permite a realização de várias conexõess TCP/UDP simultâneas
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

  #else
    // desabilita funcionalidade MUX
    if (wifi.disableMUX()) {
        Serial.println("Multiplas conexoes desabilitadas.");
    } else {
        Serial.println("Erro ao desabilitar multiplas conexoes.");
    }
  #endif
  
  Serial.println("Setup finalizado!");
  Serial.println("***********************************");

}
 

// ***************************************************************************************************
// *  LOOP principal                                                                                 *
// ***************************************************************************************************
//Na conexão TCP, basicamente a funcionalidade a ser mostrada será a de "echo", ou seja,
//a aplicação irá retornar todos os dados enviados para ela via socket TCP.

void loop(void)
{
    //Caso o servidor TCP esteja habilitado, precisa monitorar a recepção de dados
    #if (TCP_SERVER==ON)
      //Variável para buffer de dados de recepção/trasmissão
      uint8_t buf[100] = {0};
    
      //Cada conexão TCP tem sua ID, e precisa ser armazenada para referência no programa. 
      //Usamos essa variavel para isso.
      uint8_t mux_id;
  
      //A variável len serve para armazenar o comprimento de dados recebidos por meio da rotina wifi.recv(), 
      //que também associa ao buffer os dados recebidos e ao mux_id a id responsável pela transmissão
      uint32_t len = wifi.recv(&mux_id, buf, sizeof(buf), 100);
      
      //Caso tenha recebido algum dado
      if (len > 0) {
          //Mostra no terminal (DEBUG) o dado recebido
          Serial.print(F("Status:["));
          Serial.print(wifi.getIPStatus().c_str());
          Serial.println(F("]"));
          
          Serial.print(F("Recebido de :"));
          Serial.print(mux_id);
          Serial.print("[");
          for(uint32_t i = 0; i < len; i++) {
              Serial.print((char)buf[i]);
          }
          Serial.print(F("]\r\n"));
          
          //Envia o mesmo dado de volta para quem abriu a conexão TCP.
          //A referência para o socket TCP criado é o mux_id, ou id da conexão, usado na rotina wifi.send
          if(wifi.send(mux_id, buf, len)) {
              Serial.println(F("Enviado de volta..."));
          } else {
              Serial.println(F("Erro ao enviar de volta"));
          }
  
          #if (CLOSE_TPC==ON)
            //Liberação da conexão TCP, de modo a permitir que conexões diferentes sejam realizadas.
            if (wifi.releaseTCP(mux_id)) {
                Serial.print(F("Liberando conexao TCP com ID: "));
                Serial.print(mux_id);
                Serial.println(" OK");
            } else {
                Serial.print(F("Erro ao liberar TCP com ID: "));
                Serial.print(mux_id);
            }
            //Finaliza mostrando o Status de liberação [4]
            Serial.print(F("Status:["));
            Serial.print(wifi.getIPStatus().c_str());
            Serial.println(F("]"));
          #else
            Serial.println(F("***********************************"));
            Serial.println(F("Aguardando nova recepção de dados"));
          #endif
    }

  #else
  
    // Base de tempo ajustada
    // Checa temporizador para 1 segundo
    if(t2_fs){
      // Limpa flag de segundos
      t2_fs=false;

      // Checa inicio d base de tempo
      if (base_time==BASE_TIME){
        Serial.print(F("Aguarda "));
        Serial.print(BASE_TIME);
        Serial.print("s:");
      } else {
        // Immprime "." para mostrar que esta rodando
        Serial.print(F("."));
      }
      
      //Checa término da base de tempo
      if (base_time==0){        
        base_time = BASE_TIME;   // Reinicia base de tempo
        Serial.println(F("."));
        send_data();             // Executa comandos da base de tempo
      } else {
        base_time--;             // Decrementa contador de tem
      }
    }
    
  #endif
}

// ***************************************************************************************************
// *  Função de envio de dados TCP                                                                   *
// ***************************************************************************************************
void send_data(void)
{
  //Variável para buffer de dados de recepção/trasmissão
  uint8_t buffer[300] = {0};

  // Cria número randômico
  float value = random(VAL_MIN, VAL_MAX)*VAL_FAC;
     
  //Cria conexão TCP
  if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
      Serial.print(F("Conexão TCP criada com sucesso na porta "));
      Serial.print(HOST_PORT);
      Serial.println(F("."));
  } else {
      Serial.println(F("Erro ao criar conexão TCP."));
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
  Serial.println(F("Enviando pacote de dados..."));
  Serial.print(dados);
  wifi.send((const uint8_t*)dados.c_str(), dados.length());
  
  //Recepção da resposta de retorno
  uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
  if (len > 0) {
      Serial.println(F("Recebido retorno:"));
      for(uint32_t i = 0; i < len; i++) {
          Serial.print((char)buffer[i]);
      }
      Serial.print(F("\r\n"));
  }
  else{
    Serial.println(F("Não recebido retorno."));
  }

  delay(1000);

  //Liberação da conexão TCP
  if (wifi.releaseTCP()) {
      Serial.println(F("Conexao TCP liberada com sucesso."));
  } else {
      Serial.println(F("Erro ao liberar conexao TCP."));
  }

}
