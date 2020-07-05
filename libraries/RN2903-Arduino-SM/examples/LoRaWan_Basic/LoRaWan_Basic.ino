// ***************************************************************************************************
// *  Template padrão para Arduino com Shield de Radio LoRaWan RN2903                                *
// *    1. Botões da ProtoFull                                                                       *
// *    2. Módulo LoRaWan RN2903                                                                     *
// *    3. Sensor de temperatura e umidade DHT22                                                     *
// *    4. Sensor de anteparo para contador                                                          *
// *                                                                                                 *
// *  Desenvolvido por David Souza - SmartMosaic - smartmosaic.com.br                                *
// *  Versão 1.0 - Agosto/2019                                                                       *
// *                                                                                                 *
// ***************************************************************************************************
 
// ***************************************************************************************************
// *  Arquivos de include básicos                                                                    *  
// ***************************************************************************************************
#include <avr/wdt.h>          // Biblioteca do WatchDog Timer do Microcontrolador AVR

// ***************************************************************************************************
// *  Definições auxiliares                                                                          *
// ***************************************************************************************************
#define OFF           0
#define ON            1
#define UNO           0
#define MEGA          1

// ***************************************************************************************************
// *  Definições de Operação                                                                         *
// ***************************************************************************************************
#define ARDUINO       MEGA      // Mega ou Uno
#define LORA          ON        // Uso do sistema LoRa
#define DHT22         OFF       // Uso do sensor DHT22
#define COUNTER       OFF       // Uso do sensor de presença

#define PRESCALE      900       // Constante de tempo do prescale (1 segundo)   
#define BASE_TIME     30        // Tempo entre transmissões automáticas (s), 0 = Não transmite
#define DEB_BT        10        // Debounce para Contador
#define DEB_CT        10        // Debounce para Botões
#define DEBUG         ON        // Imprime mensagens de Debug

// ***************************************************************************************************
// *  Definição da pinagem                                                                           *
// ***************************************************************************************************
// Digitais
#if (LORA==ON)
  #if (ARDUINO==UNO)
    #define RX_PIN        2       // Pino RX ligado no RN2903
    #define TX_PIN        3       // Pino TX ligado no RN2903
                                  // ATENÇÃO: NO CASO DO MEGA, LIGAR RX no PINO 19 e TX no PINO 18
  #endif
  #define LORA_RST_PIN    4       // Pino RESET ligado no RN2903
#endif
#if (COUNTER==ON)
  #define CNT_PIN         5       // Pino ligado no módulo do contador
#endif
#if (DHT22==ON)
  #define DHT_PIN         6       // Pino de comuniação com o DHT22
#endif
#define LED1_PIN          7       // Pino do LED Extra

// Analógicas
#define BT_PIN          A0      // Pino ligado nos botões do Shield

// ***************************************************************************************************
// *  Variáveis globais                                                                              *
// ***************************************************************************************************
byte button;                    // Número do botão pressionado
int  num_bt = 0;                // Contador do número de vezes que 1 botão é pressionado
byte b_filter = 5;              // Filtro para detecção do botão
byte num;                       // Número randômico
bool sw=0;                      // Switch (invertido a cada botão pressionado)
int  prescaler = PRESCALE;      // Contador de presscaler para base de tempo
int  base_time = BASE_TIME;     // Contador de tempo para transmissão base de tempo (1 seg)

// ***************************************************************************************************
// *  Variáveis do contador (sensor de presença)                                                     *
// ***************************************************************************************************
#if (COUNTER==ON)
  bool counter_sensor;          // Flag do sensor contador
  int  counter = 0;             // Acumulador do sensor contador 
  byte c_filter = 5;            // Filtro de leitura do sensor contador
#endif

// ***************************************************************************************************
// *  Cria instância para leitura do sensor DHT22                                                    *
// ***************************************************************************************************
#if (DHT22==ON)
  #include <SimpleDHT.h>
  float temperature = 0;        // Valor da temperatura
  float humidity = 0;           // Valor da umidade
  SimpleDHT22 dht22(DHT_PIN);   // Cria instância para o sensor vinculado ao pino correto
#endif

// ***************************************************************************************************
// *  Variáveis do RN2903 para transmissão no payload                                                *
// ***************************************************************************************************
#if (LORA==ON)
  byte tx_errors = 0;           // Contador de transmissões com erro
  int vdd = 0;                  // Tensão de alimentação do Módulo RN2903
  signed int rssi = 0;          // Indicador de intensidade de sinal do rádio
  signed int snr = 0;           // Indicador da relação Sinal/Ruído do rádio
#endif

// ***************************************************************************************************
// *  Cria porta serial por software para comunicação com RN2903 e Instancia do objeto               *
// ***************************************************************************************************
#if (LORA==ON)
  #include <rn2903.h>                           // Biblioteca do módulo LoRaWan RN2903
  #if (ARDUINO==UNO)
    #include <SoftwareSerial.h>                 // Biblioteca da porta Serial extra por Software
    SoftwareSerial LoraSerial(RX_PIN, TX_PIN);  // RX, TX
  #elif (ARDUINO==MEGA)
    #define LoraSerial Serial1
  #endif
  rn2903 myLora(LoraSerial, LORA_RST_PIN);
#endif

// ***************************************************************************************************
// *  Função: read_button                                                                            *
// *  Descrição: Função auxiliar para ler os botões                                                  *
// *  Argumentos: Nenhum                                                                             *
// *  Retorno: Número do botão de 1 a 4, 0 para nenhum botão pressionado                             *
// ***************************************************************************************************
byte read_button(void)
{
  // Efetua leitura analógica do pino dos botôes
  int leitura = analogRead(BT_PIN);

  // Checa valor da tensão devido aos divisores resistivos dos botões
  if(leitura < 120){
    return 4;
  } else if(leitura < 250){
    return 3;
  } else if(leitura < 450){
    return 2;
  } else if(leitura < 600){
    return 1;
  } else {
    return 0;
  }
}

// ***************************************************************************************************
// *  Função: led_on                                                                                 *
// *  Descrição: Função para ligar o LED                                                             *
// *  Argumentos: Nenhum                                                                             *
// *  Retorno: Nenhum                                                                                *
// ***************************************************************************************************
void led_on(void)
{
  digitalWrite(LED1_PIN, HIGH);       // Pino do LED=1
}

// ***************************************************************************************************
// *  Função: led_off                                                                                *
// *  Descrição: Função para desligar o LED                                                          *
// *  Argumentos: Nenhum                                                                             *
// *  Retorno: Nenhum                                                                                *
// ***************************************************************************************************
void led_off(void){
  digitalWrite(LED1_PIN, LOW);        // Pino do LED=0
}

// ***************************************************************************************************
// *  Função: blink_led                                                                              *
// *  Descrição: Função para piscar o LED                                                            *
// *  Argumentos: Número e piscadas e base de tempo                                                  *
// *  Retorno: Nenhum                                                                                *
// ***************************************************************************************************
void blink_led(byte n, int t){
  for (byte i=0; i<n; i++){
    led_on();
    delay(t);
    led_off();
    delay(t);
  }
}

// ***************************************************************************************************
// *  Função: format_zero                                                                            *
// *  Descrição: Função que ajusta tamanho do número com zeros a esquerda                            *
// *  Argumentos: Número (String) e tamanho total                                                    *
// *  Retorno: String formatada                                                                      *
// ***************************************************************************************************
// Ajusta tamanho do número com zeros a esquerda
String format_zero(String str, byte len)
{
  byte str_len = str.length();
  if(str_len >= len) return str;
  for(int i=1; i<=(len-str_len);i++){
    str = "0" + str;
  }
  return str;
}


// ***************************************************************************************************
// *  Função: button_press                                                                           *
// *  Descrição: Função para tratamento de algum botão pressionado                                   *
// *  Argumentos: Nenhum                                                                             *
// *  Retorno: Nenhum                                                                                *
// ***************************************************************************************************
void button_press(void)
{
  #if (DEBUG==ON)
    Serial.println(F(""));
    Serial.println(F("*** Rotina de Botão Pressionado ***"));
  #endif

  // Incrementa contador de botão pressionado
  num_bt++;
  
  #if (LORA==ON)
    // Transmissão de dados manual
    manual_tx();
  #endif  

  #if (DEBUG==ON)
    Serial.println(F("***********************************"));
  #endif

}

// ***************************************************************************************************
// *  Função: time_auto                                                                              *
// *  Descrição: Função para tratamento por base de tempo                                            *
// *  Argumentos: Nenhum                                                                             *
// *  Retorno: Nenhum                                                                                *
// ***************************************************************************************************
void time_auto(void)
{
  #if (DEBUG==ON)
    Serial.println(F(""));
    Serial.println(F("*** Rotina de Base de Tempo ***"));
  #endif

  // Leitura do Sensor de Temperatura e Umidade DHT22 - Grava valores em temperature e humidity
  #if (DHT22==ON)
    int err = SimpleDHTErrSuccess;
    if ((err = dht22.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess)
    {
      #if (DEBUG==ON)
        Serial.print(F("Leitura do DHT22 falhou, err=")); Serial.println(err);delay(2000);
      #endif
    }
  #endif

  // Cria número randômico
  num = random(0, 9);
  
  // Inverte o valor de sw    
  sw = !sw;            

  #if (LORA==ON)
    // Transmissão de dados automática
    auto_tx();
  #endif  

  #if (DEBUG==ON)
    Serial.println(F("*******************************"));
  #endif

}

// ***************************************************************************************************
// *  Função: setup (obrigatória)                                                                    *
// *  Descrição: Função de inicialização do sistema (após energização ou reset)                      *
// *  Argumentos: Nenhum                                                                             *
// *  Retorno: Nenhum                                                                                *
// ***************************************************************************************************
void setup(void)
{
  // Desativa WDT
  wdt_disable();

  //Configura pinagem
  #if (DHT22==ON)
    pinMode(DHT_PIN, INPUT_PULLUP);             //Habilita porta como saída
  #endif
  #if (COUNTER==ON)
    pinMode(CNT_PIN, INPUT_PULLUP);             //Habilita porta como saída
  #endif
  #if (LORA==ON)
    pinMode(LORA_RST_PIN, OUTPUT);              //Habilita porta como saída
  #endif
  pinMode(LED1_PIN, OUTPUT);                  //Habilita porta como saída

  // Cria uma semente para o gerador de números randômicos
  randomSeed(500);
  
  #if (DEBUG==ON)
    // Configura porta serial para DEBUG
    Serial.begin(57600);
    Serial.println(F("=== Iniciando Setup do sistema ==="));
  #endif

  // Primeira leitura do DHT22 para iniciar o HW
  #if (DHT22==ON)
    dht22.read2(&temperature, &humidity, NULL);
  #endif
  
  #if (LORA==ON)
    // Configura porta serial para RN2903
    #if (ARDUINO==UNO)
      LoraSerial.begin(9600);
    #elif (ARDUINO==MEGA)
      LoraSerial.begin(57600);
    #endif
    // Inicializa o módulo RN2903
    initialize_rn2903();
  #endif

  #if (DEBUG==ON)
    Serial.println(F("=== Entrando em Operação Normal ==="));
  #endif

}

// ***************************************************************************************************
// *  Função: loop (obrigatória)                                                                     *
// *  Descrição: Função de looping principal                                                         *
// *  Argumentos: Nenhum                                                                             *
// *  Retorno: Nenhum                                                                                *
// ***************************************************************************************************
void loop()
{
  // Reset de WDT
  wdt_reset();

  // Checa prescaler - Contador de 1 segundo
  if(prescaler==0){
    prescaler=PRESCALE;  // Reinicializa prescaler
    
    // Checa base de tempo - Múltiplo do prescaler
    #if (BASE_TIME > 0)
      if (base_time==0){        
        base_time = BASE_TIME;  // Reinicia base de tempo
        #if (LORA==ON)
          time_auto();          // Executa comandos da base de tempo
        #endif
      }
      base_time--;              // Decrementa contador de tempo
    #endif
  }
  
  // Trata filtro do botão
  #if (RFID==ON)
    if (programMode){
       check_wipe();
    } else {
  #endif
      if (button>0 && button==read_button()){
        if(b_filter==0) {
          button_press();       // Trata botão pressionado
          b_filter=255;
        } else {
          if (b_filter!=255)b_filter--;      
        }
      } else {
        button=read_button();   // Le botão
        b_filter=DEB_BT;
      }
      
  #if (RFID==ON)
    }
  #endif

  // Trata filtro do contador
  #if (COUNTER==1)
    if (counter_sensor==0 && counter_sensor==digitalRead(CNT_PIN)){
      if(c_filter==0) {
        counter++;          // Incrementa contador
        c_filter=255;
      } else {
        if (c_filter!=255)c_filter--;      
      }
    } else {
      counter_sensor==digitalRead(CNT_PIN);  // Lê sendor de presença
      c_filter=DEB_CT;
    }    
  #endif    
  
  // Delay multiplo do prescaler
  delay(1);
  prescaler--;
}
