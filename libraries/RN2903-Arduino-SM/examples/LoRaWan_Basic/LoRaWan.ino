// ***************************************************************************************************
// *  Definições e Funções para Sistema Lora RN2903                                                  *
// *                                                                                                 *
// *  Desenvolvido por David Souza - SmartMosaic - smartmosaic.com.br                                *
// *  Versão 1.0 - Agosto/2019                                                                       *
// *                                                                                                 *
// ***************************************************************************************************

#if (LORA==ON)
 
// ***************************************************************************************************
// *  Definições auxiliares                                                                          *
// ***************************************************************************************************
#define ABP             1
#define OTAA            2
#define PRINT_PARAMS    ON        // Imprime parametros salvos no RN2903
#define DEBUG_RN2903    ON        // Imprime mensagens de Debug do RN2903

#define LORA_PW         5         // Potência inicial do rádio
#define LORA_DR         2         // Data Rate
#define LORA_ADR        ON        // Adaptative Data Rate
#define LORA_AR         OFF       // Repetição automática de TX
#define LORA_REP        2         // Número de repetições de TX

#define TX_CNF          ON        // Confirmação de TX
#define TX_ERRORS       3         // Número máximo de erros antes de resetar o RN2903
#define MAX_JOIN        3         // Número máximo de tentativa de JOIN sem sucesso
#define TX_LORA         ON        // Desativa o uso do rádio para facilitar o debug da lógica principal

// ***************************************************************************************************
// *  Definições do Node                                                                             *
// ***************************************************************************************************
#define NODE            1         // Número do Node (caso esteja imlementado mais de 1)

// Configuração do Payload automático (base de tempo)
#define TX_TEMP         OFF       // Transmite Temperatura no Payload do LoRa
#define AL_TEMP         "A4"      // Alias para variável de Temperatura
#define TX_HUMI         OFF       // Transmite Umidade no Payload do LoRa
#define AL_HUMI         "A5"      // Alias para variável de Umidade
#define TX_COUNTER      OFF       // Transmite Contador no Payload do LoRa
#define AL_COUNTER      "A6"      // Alias para variável de Contador

#define TX_RSSI         ON        // Transmite RSSI no Payload do LoRa
#define AL_RSSI         "B0"      // Alias para variável de RSSI
#define TX_SNR          ON        // Transmite SNR  no Payload do LoRa
#define AL_SNR          "B1"      // Alias para variável de SNR
#define TX_VDD          OFF       // Transmite VDD  no Payload do LoRa
#define AL_VDD          "B2"      // Alias para variável de VDD

// Configuração do Payload manual (botão presionado)
#define TX_BT           ON        // Transmite Botão no Payload do LoRa
#define AL_BT           "A1"      // Alias para variável do Botão
#define TX_NUM          OFF        // Transmite Número Randômico no Payload do LoRa
#define AL_NUM          "A2"      // Alias para variável do Número
#define TX_SW           OFF        // Transmite Switch no Payload do LoRa
#define AL_SW           "A3"      // Alias para variável do Switch
#define TX_NUM_BT       ON        // Transmite número de vezes que 1 botão foi pressionado
#define AL_NUM_BT       "A7"      // Alias para variável de vezes do botão

// ***************************************************************************************************
// *  Checagem de conflitos                                                                          *
// ***************************************************************************************************
#if (DHT22==OFF)
  #undef  TX_TEMP
  #define TX_TEMP       OFF
  #undef  TX_HUMI
  #define TX_HUMI       OFF  
#endif
#if (COUNTER==OFF)
  #undef  TX_COUNTER
  #define TX_COUNTER    OFF
#endif


// ***************************************************************************************************
// *  Definições da Ativação                                                                         *
// ***************************************************************************************************
// Caso queira utilizar DEVEUI de forma automática, como sendo igual ao HWEUI, não definir 
// o valor de DEVEUI
#if (NODE==1)
  #define ACTIVATION    OTAA                                      // ABP ou OTAA
  #if (ACTIVATION==OTAA)
    #define DEVEUI      "SEU_DEVEUI"
    #define APPKEY      "SEU_APPKEY"
    #define APPEUI      "SEU_APPEUI" 
  #else
    #define DEVADDR     "SEU_DEVADDR" 
    #define NWKSKEY     "SEU_NEWSKEY"
    #define APPSKEY     "SEU_APPSKEY"
  #endif
#endif

#if (NODE==2)
  #define ACTIVATION    OTAA                                      // ABP ou OTAA
  #if (ACTIVATION==OTAA)
    #define DEVEUI      "SEU_DEVEUI"
    #define APPKEY      "SEU_APPKEY"
    #define APPEUI      "SEU_APPEUI" 
  #else
    #define DEVADDR     "SEU_DEVADDR" 
    #define NWKSKEY     "SEU_NEWSKEY"
    #define APPSKEY     "SEU_APPSKEY"
  #endif
#endif

// ***************************************************************************************************
// *  Função: print_params                                                                           *
// *  Descrição: Função para imprimir na porta DEBUG os parâmetros do módulo RN2903                  *
// *  Argumentos: Nenhum                                                                             *
// *  Retorno: Nenhum                                                                                *
// ***************************************************************************************************
void print_params(void){
#if (DEBUG==ON && PRINT_PARAMS==ON)
  String st;
  // Imprime quais canais estão ligados e com que frequência
  for (int i=0; i<64; i++)
  {
    st = myLora.sendRawCommand("mac get ch status "+String(i));
    if(st=="on")
    {
      Serial.print(F("Canal "));
      Serial.print(i);
      Serial.print(F(": ON / "));
      Serial.print(myLora.sendRawCommand("mac get ch freq "+String(i)));
      Serial.println(F(" Hz"));
    }
  }

  // Imprime o Data Rate (DR) selecionado
  Serial.print("DR: ");
  Serial.println(myLora.sendRawCommand(F("mac get dr")));
  
  // Imprime o status do Adaptative Data Rate (ADR)
  Serial.print("ADR: ");
  Serial.println(myLora.sendRawCommand(F("mac get adr")));

  // Imprime o status do Auto Repeat (AR)
  Serial.print("AR: ");
  Serial.println(myLora.sendRawCommand(F("mac get ar")));
  
  // Imprime o número de repetições do TX
  Serial.print("RETX: ");
  Serial.println(myLora.sendRawCommand(F("mac get retx")));

  // Imprime a potência inicial do rádio
  Serial.print("PWRIDX: ");
  Serial.println(myLora.sendRawCommand(F("mac get pwridx")));

  // Imprime o DR e a Frequência da segunda janela de recepção RX2
  Serial.print("RX2: ");
  Serial.println(myLora.sendRawCommand(F("mac get rx2")));  

  // Imprime o tempo (delay) da primeira janela de recepção RX1
  Serial.print("Delay RX1: ");
  Serial.println(myLora.sendRawCommand(F("mac get rxdelay1")));  

  // Imprime o tempo (delay) da segunda janela de recepção RX2
  Serial.print("Delay RX2: ");
  Serial.println(myLora.sendRawCommand(F("mac get rxdelay2"))); 
  
  // Imprime o Contador de Uplink
  Serial.print("UpCtr: ");
  Serial.println(myLora.sendRawCommand(F("mac get upctr"))); 

  // Imprime o Contador de Downlink
  Serial.print("DnCtr: ");
  Serial.println(myLora.sendRawCommand(F("mac get dnctr"))); 
#endif
}

// ***************************************************************************************************
// *  Função: manual_tx                                                                              *
// *  Descrição: Função para tratamento de um pacote de dados manualmente                            *
// *  Argumentos: Nenhum                                                                             *
// *  Retorno: Nenhum                                                                                *
// ***************************************************************************************************
void manual_tx(void)
{
    // Imprime DEBUG
    #if (DEBUG==ON)
      Serial.print(F("Botão: "));
      Serial.print(button); 
      Serial.print(F(", Cont. Bt: "));
      Serial.print(num_bt); 
      Serial.print(F(", Num: "));
      Serial.print(num); 
      Serial.print(F(", Sw: "));
      Serial.print(sw); 
      #if (COUNTER==ON)
          Serial.print(F(", Contador: "));
          Serial.print(counter);  
      #endif
      Serial.println("");
    #endif
    
    // Prepara PAYLOAD para transmissão
    String payload = "";
    #if (TX_BT==ON)
      payload += AL_BT;
      payload += format_zero((String) button,2);
    #endif 
    #if (TX_NUM_BT==ON)
      payload += AL_NUM_BT;
      payload += format_zero((String) num_bt,5);
    #endif 
    #if (TX_NUM==ON)
      payload += AL_NUM;
      payload += format_zero((String) num,2); 
    #endif
    #if (TX_SW==ON)
      payload += AL_SW;
      payload += format_zero((String) sw,2); 
    #endif 

    // Sensor de presença - Contador
    #if (TX_COUNTER==ON)
      payload += AL_COUNTER;
      payload += format_zero((String) counter,5); 
    #endif

    // Transmite PAYLOAD
    tx_payload(payload);
}

// ***************************************************************************************************
// *  Função: auto_tx                                                                                *
// *  Descrição: Função para transmissão automática (periódica) do pacote pelo LoRa                  *
// *  Argumentos: Nenhum                                                                             *
// *  Retorno: Nenhum                                                                                *
// ***************************************************************************************************
void auto_tx(void)
{
    // Imprime DEBUG
    #if (DEBUG==ON)
      #if (DHT22==ON)
          // Sensor de Temperatura e Umidade DHT22
          Serial.print(F("Temp: ")); Serial.print((float)temperature); Serial.print(F("°C, "));
          Serial.print(F("Umid: ")); Serial.print((float)humidity); Serial.print(F("%, "));
      #endif
      #if (COUNTER==ON)
        // Sensor de presença - Contador
        Serial.print(F("Contador: ")); Serial.print(counter);
      #endif
      #if (DHT22==ON || COUNTER==ON)
        Serial.println(F(""));
      #endif
    #endif
   
    // Prepara PAYLOAD para transmissão
    String payload = "";

    #if (TX_TEMP==ON)
      payload += AL_TEMP;
      payload += temperature;
    #endif
    #if (TX_HUMI==ON) 
      payload += AL_HUMI;
      payload += humidity;
    #endif 
    #if (TX_COUNTER==ON)
      payload += AL_COUNTER;
      payload += format_zero((String) counter,5); 
    #endif
    #if (TX_RSSI==ON)
      payload += AL_RSSI;
      if(rssi<0)
      {
        payload += "-";
      } else {
        payload += "+";
      }
      payload += format_zero((String) abs(rssi),3); 
    #endif
    #if (TX_SNR==ON)
      payload += AL_SNR;
      if(snr<0)
      {
        payload += "-";
      } else {
        payload += "+";
      }
      payload += format_zero((String) abs(snr),3); 
    #endif
    #if (TX_VDD==ON)
      payload += AL_VDD;
      payload += format_zero((String) vdd,4); 
    #endif
        
    // Transmite PAYLOAD
    tx_payload(payload);
}

// ***************************************************************************************************
// *  Função: tx_payload                                                                             *
// *  Descrição: Função para transmissão do pacote (Payload) determinado                             *
// *  Argumentos: Pacote (STRING) para transmissão                                                   *
// *  Retorno: Nenhum                                                                                *
// ***************************************************************************************************
void tx_payload(String payload)
{
    // Variável do tipo de retorno da transmissão
    TX_RETURN_TYPE tx_type;

    #if (DEBUG==ON)
      Serial.print(F("Payload: "));
      Serial.println(payload);
    #endif

    #if (TX_LORA==OFF)
      Serial.println(F("Simulando transmissão..."));
      return;
    #endif
    
    // Executa transmissão do pacote (payload)
    led_on();
    //tx_type = myLora.tx(payload, TX_CNF);
    if (button>=3){
      tx_type = myLora.tx(payload, ON);
    }else{
      tx_type = myLora.tx(payload, OFF);
    }
    led_off();

    // Checa o resultado da transmissão
    // Houve ERRO
    if(tx_type!=TX_SUCCESS && tx_type!=TX_WITH_RX)
    {
      #if (DEBUG==ON)
        Serial.print(F("Erro de TX: "));
        Serial.println(tx_type);
      #endif
      
      // Incrementa contador de erro
      tx_errors++;
      // Se atingiu máximo de erro, reinicializa o LoRa
      if (tx_errors==TX_ERRORS)
      {
        tx_errors=0;
        #if (DEBUG==ON)
          Serial.println(F("Número máximo de erros. O módulo RN2903 será reiniciado."));
        #endif
          
        // Reinicia o módulo
        myLora.join();
        return;
      }       
    }

    // Houve SUCESSO
    else
    {
      #if (DEBUG==ON)
        Serial.print(F("Sucesso de TX: "));
        Serial.println(tx_type);

        // houve resposta de retorno
        if (tx_type==TX_WITH_RX)
        {
          Serial.print(F("Resposta RX: "));
          Serial.println(myLora.getRxMessenge());
        }
      #endif  
      // Zera contador de erros
      tx_errors=0;                      
    }

    // Atualiza as variáveis do RN2903, conforme a última transmissão
    #if (TX_RSSI==ON)
      rssi = myLora.getRSSI();
    #endif
    #if (TX_SNR==ON)
      snr = myLora.getSNR();
    #endif
    #if (TX_VDD==ON)
      vdd = myLora.getVDD();
    #endif
}

// ***************************************************************************************************
// *  Função: initialize_rn2903                                                                      *
// *  Descrição: Função para iniciar o rn2903                                                        *
// *  Argumentos: Nenhum                                                                             *
// *  Retorno: Nenhum                                                                                *
// ***************************************************************************************************
void initialize_rn2903(void)
{
  #if (TX_LORA==OFF)
    Serial.println(F("Simulando inicialização do RN2903..."));
    return;
  #endif
    
  // Reseta o módulo
  myLora.factoryReset();

  // Limpa dados recebidos na porta serial
  LoraSerial.flush();
  Serial.flush();

  // Configura parâmetros do Modulo RN2903:
  myLora.setParams( 0,              // Índice da sub-banda
                    LORA_DR,        // DR (AU => 0=SF12, 1=SF11, 2=SF10, 3=SF9, 4=SF8, 5=SF7)
                    255,            // 0 (SINGLE CHANNEL - 902.3MHz) / 255 = Sub-band
                    LORA_ADR,       // ADR - Adaptative Data Rate
                    LORA_REP,       // Número de tentativas de retransmissões automáticas
                    DEBUG_RN2903,   // Deve ou não debugar na porta Serial
                    LORA_AR,        // Liga a repetição automática
                    LORA_PW         // Potência inicial do Rádio 
                    ); 
                    
  // Inicializa parâmetros para executar JOIN conforme tipo de ativação
  #if (ACTIVATION==OTAA)
    myLora.setJoin(APPEUI, APPKEY, DEVEUI, true);
  #else
    myLora.setJoin(NWKSKEY, APPSKEY, DEVADDR, false);
  #endif

  // Reseta, configura e inicializa o módulo RN2903
  myLora.init();

  #if (DEBUG==ON)
    Serial.print(F("Node: "));
    Serial.println(NODE);
    Serial.print(F("Hardware DevEUI: "));
    Serial.println(myLora.hweui());
    Serial.print(F("Versão do Firmware: "));
    Serial.println(myLora.sysver());
    #if (ACTIVATION==OTAA)
      Serial.print(F("devEui: ")); Serial.println(myLora.sendRawCommand(F("mac get deveui")));
      Serial.print(F("appEui: ")); Serial.println(myLora.sendRawCommand(F("mac get appeui")));
      Serial.print(F("appKey: ")); Serial.println(myLora.appkey());
    #else
      Serial.print(F("devAddr: ")); Serial.println(myLora.sendRawCommand(F("mac get devaddr")));
      Serial.print(F("nwkSkey: ")); Serial.println(myLora.appeui());
      Serial.print(F("appSKey: ")); Serial.println(myLora.appkey());
    #endif
  #endif
  
 
  #if (DEBUG==ON)
    print_params(); 
    Serial.print(F("Tentativa de JOIN na rede via "));
    Serial.println(myLora.netType());
  #endif
    
  bool join_result = false;
  byte n=2;                       // Número máximo de tentativas
  
  // Tenta JOIN com a rede
  do {
    // Executa tentativa de JOIN
    led_on();
    join_result = myLora.join();
    led_off();

    // Checa falha do JOIN
    if (!join_result)
    {
      #if (DEBUG==ON)
         Serial.println(F("JOIN sem sucesso. Será feita nova tentativa em breve"));
      #endif
      // Decrementa contador de tentativas
      n--;
      // Aguarda tempo antes de nova tentativa
      delay(10000);
    }
    
    // Checa fim de tentativas
    if(n==0)
    {
      #if (DEBUG==ON)
        Serial.println(F("JOIN falhou em todas as tentativas. O sistema será completamente ressetado."));
      #endif
      wdt_enable(WDTO_2S);
      while(1);
    }
   
  } while(!join_result);

  #if (DEBUG==ON)
    Serial.println(F("Sucesso de JOIN com a rede."));
  #endif
}

#endif
