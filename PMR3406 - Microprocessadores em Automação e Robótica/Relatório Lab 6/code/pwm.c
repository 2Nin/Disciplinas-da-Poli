/* 
 * File:     pwm.c 
 * Alunos:   Antônio Augusto Carnevalli e Thiago Lam Brawerman
 * Date:     24/05/2025
 * Comments: Gera PWM para acionamento dos motores com duty cycle de 60%.
 *           Baseado no código pwm.c utilizado para testes na
 *           atividade de laboratório 2, adaptado seguindo os
 *           requisitos da atividade de laboratório 6.
 */

#ifndef PWM_C
#define PWM_C

#include <stdio.h>
#include "always.h"
#include "pwm.h"
#include <xc.h>

#define DIR1 RA5  // direção do PWM 1 
#define DIR2 RA6  // direção do PWM 2

#define PWM_PR2 0xFF //Definindo o perídodo do PWM de acordo com o datashet
                     // para F = 19.53kHz. (Tabela 11-3)
///
/// Inicializa a função de PWM com uma determinada frequencia
///

void pwm_init(void){
    

    // Saída dos sinais de direção
    ANS4 = 0;   // RA5 é digital
    TRISA5 = 0; // RA5 é saída
    DIR1 = 0;   // direção do motor 1 (RA5)
    
    TRISA6 = 0; // RA6 é saída
    DIR2 = 0;   // direção do motor 2 (RA6)


    // 1. Desabilitando os pinos de PWM (CCPx), habilitando-os como entrada TRIS
    TRISC1 = 1; // CCP1 como entrada
    TRISC2 = 1; // CCP2 como entrada

    // 2. Definindo o período da PWM no registrador PR2
    PR2 = PWM_PR2; 

    // 3. Configurando o módulo CCP para modo PWM usando CCPxCON
    CCP1CON = 0b00001100; // Modo PWM para CCP1
    CCP2CON = 0b00001100; // Modo PWM para CCP2

    // 4. Definindo Duty Cycle do PWM em CCPRxL e nos bits DCxB<1:0>, 0% inicial
    CCPR1L = 0;
    CCP1CONbits.DC1B = 0;

    CCPR2L = 0;
    CCP2CONbits.DC2B1 = 0;
    CCP2CONbits.DC2B0 = 0;

    // 5. Configurando e disparando Timer 2
    PIR1bits.TMR2IF = 0;      // Limpa flag
    T2CONbits.T2CKPS = 0b01;  // Prescaler do timer 2 como 4
    T2CONbits.TMR2ON = 1;     // Liga Timer 2

    // 6. Habilitando a saída PWM depois que um novo ciclo iniciar
    while (!PIR1bits.TMR2IF); // Esperando o transbordo
    TRISCbits.TRISC2 = 0;     // CCP1 como saída (motor esquerdo)
    TRISCbits.TRISC1 = 0;     // CCP2 como saída (motor direito)
  
}

///
/// Define o duty cycle da saida PWM
/// @param channel - canal do PWM (1: motor 1; 2: motor 2)
/// @param duty_cycle - porcentagem do duty cycle x 10 (valor de 0 a 1000)
///

void pwm_set(int channel, int duty_cycle){
    
    //Limitando os valores para 0-100%
    if (duty_cycle < 0) duty_cycle = 0;
    if (duty_cycle > 1000) duty_cycle = 1000;
  
    switch (channel) {
        case 1: // Motor esquerdo - CCP1
            CCPR1L = (unsigned char) duty_cycle;                // Define os 8 MSBs do duty cycle de CCP1                     
            //CCP1CONbits.DC1B = pwm_value & 0b11;    // Define os 2 LSBs do duty cycle de CCP1
            break;
        case 2: // Motor direito - CCP2
            CCPR2L = (unsigned char) duty_cycle;                // Define os 8 MSBs do duty cycle de CCP2  
            //CCP2CONbits.DC2B = pwm_value & 0b11;    // Define os 2 LSBs do duty cycle de CCP2
            break;
        default:
            // Canal inválido, não faz nada
            break;
        }
}

///
/// Altera a direção de movimento
/// @param dir - código de direção
///              0 - para frente (ou qualquer outro valor)
///              1 - para trás
///              2 - gira para a esquerda
///              3 - gira para a direira
///
void pwm_direction(int dir) {
  switch (dir) {
    case 1: // para trás
      DIR1 = 1;
      DIR2 = 1;
      break;
    case 2: // para a esquerda
      DIR1 = 1;
      DIR2 = 0;
      break;
    case 3: // para a direita
      DIR1 = 0;
      DIR2 = 1;
      break;
    default: // para frente
      DIR1 = 0;
      DIR2 = 0;
  } 
}
#endif
