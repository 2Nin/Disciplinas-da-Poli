/*
 * Arquivo:         main.c
 * Autores:         Antônio Augusto Carnevalli e Thiago Lam Brawerman
 * Data:            06/06/2025
 * Descrição:       Tarefa do robô autônomo, segue linha
 * Historico de revisões: ..
 */

#define SIMULATOR       // comentar quando for no robÃ´

#include <xc.h>         // definições dos microcontroladores
#include <stdio.h>      // I/O básico (printf, sprintf, ...)
#include <stdint.h>     // tipos de variáveis padrÃ£o (uint8_t, int16_t, ...)
#include <stdlib.h>     //funções adicionais (itoa, ltoa, ultoa, ...)


// Defines

// Todas as definições usadas no programa
#define DIST_THRESHOLD 500 // distância mínima de 40mm (baseado nos dados do Lab 4)


// PIC16F886 ajustes do bit de configuração
// CONFIG1
#pragma config FOSC = EC    // bits de configuração do ocilador (EC: I/O function on RA6/OSC2...
#pragma config WDTE = OFF   // bit de habilitação do Watchdog Timer (WDT disabled and can be enab...
#pragma config PWRTE = ON   // bit de habilitação do Power-up Timer (PWRT enabled)
#pragma config MCLRE = ON   // bit de seleção da função do pino RE3/MCLR (RE3/MCLR pin function...
#pragma config CP = OFF     // bit de proteção do código (Program memory code protection is ...
#pragma config CPD = OFF    // bit de proteção do código dos Dados (Data memory code protection i...
#pragma config BOREN = ON   // bit de seleção do Brown Out Reset (BOR enabled)
#pragma config IESO = OFF   // bit de Internal External Switchover (Internal/External Swi...
#pragma config FCMEN = OFF  // bit de habilitação do Fail-Safe Clock Monitor (Fail-Safe Clock Mo...
#pragma config LVP = OFF    // bit de habilitação de Low Voltage Programming (RB3 pin has digital...
// CONFIG2
#pragma config BOR4V = BOR40V   // bit de seleção do Brown-out Reset (Brown-out Reset set ...
#pragma config WRT = OFF        // bits de habilitação do Flash Program Memory Self Write (Write ...

// Includes do projeto
#include "always.h"

#ifndef SIMULATOR       // dispostivos que nÃ£o tem no simulador
#include "led_rgb.h"    // LED RGB do robô´
#endif

#include "adc.h"        // conversor A/D
#include "lcd8x2.h"     // LCD, no robô não necessário o SPI.
#include "sensor.h"     // sensores de linha, de proximidade e buzzer
#include "key.h"        // botão da placa
#include "pwm.h"        // PWM Lab 6

// Saídas
#define BUZZER RB7   // bit para buzzer
#define LED RB5      // bit de saída para o LED


// Variáveis Globais


// Funções

// inicializa Timer 0
void t0_init(void) {
    // O Timer 0 é utilizado para interrupção periódica a cada 5 ms aproximadamente
    OPTION_REGbits.T0CS = 0;    // usa clock interno FOSC/4
    OPTION_REGbits.PSA = 0;     // pre-scaler é para Timer 0 e não para WDT
    OPTION_REGbits.PS = 6;      // ajusta Prescaler para 1:128
    TMR0 = 0xff - 195;          // valor inicial do Timer 0 p/ 5 ms
    TMR0IE = 1;                 // habilita interrupção do Timer 0
}

// inicializa LED 
void led_init(void) {
  TRISB5 = 0;  // RB5 é saída para LED
  ANS13 = 0;   // RB5/AN13 é digital  
  LED = 0;
}

// inicializa Buzzer
void buzzer_init(void) {
    TRISB7 = 0;            // RB7 é saída para BUZZER 
    BUZZER = 0; 
    #ifndef SIMULATOR
    sensor_power(ON);     // Buzzer precisa para funcionar
    #endif
}

// Toca um beep
// Para usar o BUZZER o SENSOR POWER deve estar ligado.
void beep() {
    BUZZER = ON;          // liga buzzer
    __delay_ms(200);      // por 0,2s
    BUZZER = OFF;         // desliga buzzer
}

void welcome_message() {
    lcd_goto(0);                          // vai para início da 1a. linha
    lcd_puts("Lab 7");                    // coloca no LCD a atividade atual      
    lcd_goto(64);                         // vai para o início da 2a. linha
    lcd_puts("Grupo 02");                 // coloca o grupo
    __delay_ms(1);                        // dá um tempo para o display mostrar a mensagem
}

// Função de tratamento de interrupções
void __interrupt() isr(void) {
    
    // Timer 0
    // Interrupção do Timer 0 a cada ~5 ms
    if (TMR0IE && TMR0IF) { // se for interrupção do Timer 0
                
        // Debounce da chave. Deve ser incluído na interrupçãp periódica de Timer.
        key_debounce(2);    // 2 é o número de ciclos para dar 5 a 10 ms
        
        TMR0 = 0xff - 195;  // recarrega contagem no Timer 0 p/ 5,0176ms
        TMR0IF = 0;         // limpa interrupção
        
    }// fim - tratamento do Timer 0
    
    if (RBIE && RBIF) { // se for mudança estado do Port B
        
        char portB = PORTB; // faz a leitura do Port B, isso reseta a condição de interrupção junto com RBIF = 0
        key_read(portB);    // faz a leitura da chave
        RBIF = 0;           // reseta o flag de interrupção para poder receber outra interrupção
    } // fim - tratamento I-O-C PORT B
    
} // fim - tratamento de todas as interruções
 
 
// Programa Principal
void main(void) {
     
    // Variáveis locais
    char keyIn = FALSE;         // tecla pressionada, TRUE = sim
    char isOn = OFF;            // modo autônomo, inicialmente desligado
    int nearSensorData;         // valor do sensor de proximidade
    union chartype lineSensor;  // valor do sensor de linha
    int dutyCycle;              // valor do Duty cycle do PWM
    
    //
    // Inicializações
    //
    
    // Inicializações o robô
#ifndef SIMULATOR       // dispositivos que não tem no simulador
    led_rgb_init();     // inicializa LED RGB - deve ser comentado para uso no simulador
#endif
    lcd_init();         // inicializa lcd 
    sensor_init();      // inicializa sensores
    
    // Inicializações da placa local
    led_init();         // inicializa LED para debug
    buzzer_init();      // inicializa o buzzer
    key_init();         // inicializa chave 
    pwm_init();         // inicializa PWM 
    
    // Controle de interrupções
    ei();   // habilita interrupções, equivale a GIE = 1;
    
    #ifndef SIMULATOR
    sensor_power(ON);       // liga energia dos sensores, demora 40 ms para ligar o sensor de proximidade
    #endif
    beep();

    __delay_ms(2000);       // espera para poder ler mensagem de reset
    lcd_clear();            // limpa LCD, não deve ser usado dentro de loops pois demora muito
    lcd_show_cursor(OFF);   // desliga cursor do LCD
    
    welcome_message();      // recoloca mensagem inicial 
    beep();                 // toca beep para sinalizar que está pronto

     __delay_ms(2000);      //espera para a leitura da welcome_message()
    lcd_clear();            // limpa LCD

    pwm_direction(0);       // Ambos motores indo para frente
    __delay_ms(50);         // tem que esperar antes de religar motores pois o pico de corrente reseta o PIC interno
    
    while(1) { // Loop Principal
        
        keyIn = key_pressed();               // armazena status da chave
        
#ifdef SIMULATOR // valores simulados do sensor de linha
        lineSensor.byte = 0b000; // valor simulado para 0-0-0
#else // dispositivos reais
        lineSensor.byte = sensorLine_read(); // valor do sensor de linha
#endif

        // liga ou desliga o robô
        if (keyIn){
            isOn = ~isOn;       // muda o estado
            beep();             // para debug
            LED =  (__bit)isOn; // para debugging
        }
        
        // caso esteja desligado
        if(!isOn){
            // desliga os motores
            pwm_set(1,0);           // para o motor esquerdo
            pwm_set(2,0);           // para o motor direito
#ifndef SIMULATOR
            led_rgb_set_color(0);   // LED RGB desligado
#endif
        } // Fim do estado deligado do robô
        
        // caso esteja ligado
        if (isOn){
            nearSensorData = (int)sensorNear_read();    // salva o valor do sensor de proximidade
              
            if (nearSensorData >= DIST_THRESHOLD ){     // caso a distância seja menor ou igual a mínima
                
                // para os motores
                dutyCycle = 0;          // muda o duty cycle para 0
                pwm_set(1,dutyCycle);   // ajusta o PWM do motor esquerdo
                pwm_set(2,dutyCycle);   // ajusta o PWM do motor direito
#ifndef SIMULATOR
                led_rgb_set_color(4); // LED RGB cor vermelha
#endif
            }else{ // caso a distância mínima NÃO seja atingida
                
                // o valor do duty cycle (0 - 600) baseado na distância do obstáculo
                dutyCycle = 600 - nearSensorData;
                
                if (dutyCycle <= 400){ // se o duty cycle menor que um certo valor para o PWM 
                    // previne que o PWM seja ligado quando a distância for inferior à 4cm
                    
                    dutyCycle = 0;          // muda o duty cycle para 0
                    pwm_set(1,dutyCycle);   // ajusta o PWM do motor esquerdo
                    pwm_set(2,dutyCycle);   // ajusta o PWM do motor direito
#ifndef SIMULATOR
                    led_rgb_set_color(4);   // LED RGB cor vermelha
#endif
                }else{
                    // estados do sensor de linha (0 - 7)
                    switch (lineSensor.byte){
                        case 7: // sensor de linha : 1-1-1
                        case 2: // sensor de linha : 0-1-0
                        case 5: // sensor de linha : 1-0-1
                            
                            // Andar em linha reta
                            pwm_set(1,dutyCycle - 200);     // motor esquerdo em velocidade média
                            pwm_set(2,dutyCycle - 200);     // motor direito em velocidade média
        #ifndef SIMULATOR
                            led_rgb_set_color(2);           // LED RGB cor verde
        #endif
                            break;

                        case 4: // sensor de linha : 0-0-1               
                        case 6: // sensor de linha : 0-1-1
                            
                            // Girar sentido horário
                            pwm_set(1,dutyCycle - 200);     // motor esquerdo em velocidade média
                            pwm_set(2,dutyCycle - 400);     // motor direito em velocidade baixa

        #ifndef SIMULATOR
                            led_rgb_set_color(1);           // LED RGB cor azul
        #endif
                            break;

                        case 1: // sensor de linha : 1-0-0
                        case 3: // sensor de linha : 1-1-0
                            
                            // Girar sentido anti horário
                            pwm_set(1,dutyCycle - 400);     // motor esquerdo em velocidade alta 
                            pwm_set(2,dutyCycle - 200);     // motor direito em velocidade média
        #ifndef SIMULATOR
                            led_rgb_set_color(5);           // LED RGB cor magenta
        #endif
                            break;

                        case 0: // sensor de linha : 0-0-0
                            
                            // Rodar em círculo no sentido horário
                            pwm_set(1,dutyCycle -100);      // motor esquerdo em velocidada alta
                            pwm_set(2,dutyCycle - 300);     // motor direito em velocidade média
        #ifndef SIMULATOR
                            led_rgb_set_color(1);           // LED RGB cor azul
        #endif
                            break;

                        default: // Outras entradas
                            
                            // para o robô
                            pwm_set(1,0);           // motor esquerdo parado
                            pwm_set(2,0);           // motor direito parado
                            break;
                    }
                }
            }
            __delay_ms(20);   // tem que esperar antes de religar motores pois o pico de corrente reseta o PIC interno
        } //Fim do estado ligado do robô
    } // Fim do Loop Principal
} // Fim do Programa Principal