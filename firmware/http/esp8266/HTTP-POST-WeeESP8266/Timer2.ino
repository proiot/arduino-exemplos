// ***************************************************************************************************
// *  Definições e Funções para Sistema de tempo baseado no Timer2                                   *
// *                                                                                                 *
// *  Desenvolvido por David Souza - SmartMosaic - smartmosaic.com.br                                *
// *  Versão 1.0 - Agosto/2019                                                                       *
// *                                                                                                 *
// ***************************************************************************************************

// ***************************************************************************************************
// *  Definições auxiliares                                                                          *
// ***************************************************************************************************

// ***************************************************************************************************
// *  Função: ISR(TIMER_OVF_vect)                                                                    *
// *  Descrição: Vetor de tinterrupção de estouro do Timer2                                          *
// *  Argumentos: Nenhum                                                                             *
// *  Retorno: Nenhum                                                                                *
// ***************************************************************************************************
ISR(TIMER2_OVF_vect){
  TCNT2 = 131;            // Reinicializa do contador. Estoura em 256 (256-131 = 125)

  // Decrementa prescaler de timer2
  // Se acabou, passou 1 segundo
  // Os flags devem ser limpos manualmente na rotina principal

  if (!(t2_ps--)) {       // Passou 1 segundo
    t2_ps=125;            // Reinicia 2º prescaler
    t2_fs=true;           // Marca flag de 1 segundo
    t2_seg++;             // Incrementa contador de segundos
    if (t2_seg==60){      // Passou 1 minuto
      t2_seg==0;          // Reinicia contador de segundos
      t2_fm=true;         // Marca flag de 1 minuto
      t2_min++;           // Incrementa contador de minutos
      if (t2_min==60){    // Passou 1 minuto
        t2_min==0;        // Reinicia contador de minutos
        t2_fh=true;       // Marca flag de 1 hora
      }    
    }
  }
}

// ***************************************************************************************************
// *  Função: setupTimer2                                                                            *
// *  Descrição: Vetor de tinterrupção de estouro do Timer2                                          *
// *  Argumentos: Nenhum                                                                             *
// *  Retorno: Nenhum                                                                                *
// ***************************************************************************************************
void setupTimer2(void){
  // Configura Timer2
  // Estouro = Timer2_count x prescaler x ciclo_máquina
  // ciclo_máquina = 1/Fosc = 1/16E6 = 62,5ns = 62,5E-9s
  // Estouro = (125 = 256 - 131) x 1024 x 62,5E-9s = 8ms
  // 2º prescaler t2_ps = 125 x 8ms = 1 segundo
  
  TCCR2B = 0x07;          // Prescaler 1:1024 (ciclo_máquina x 1024)
  TCNT2  = 131;           // Inicializa contador (256 - 131 = 125)
  TIMSK2 = 0x01;          // Interrupção por estouro de timer
  TCCR2A = 0x00;          // Timer operando em modo normal 
}
