#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C

// Variáveis Globais
ssd1306_t ssd; // Variável global para o display
bool dono_proximo = false; // Estado de proximidade do dono
bool alarme_ativado = false; // Estado do alarme
bool estado_do_buzzer = false; // Estado do alarme

// Definição dos pinos dos LEDs e botões
const uint led_verde = 11; // LED verde para porta aberta
const uint led_azul = 12;  // LED azul para movimento detectado
const uint led_vermelho = 13; // LED vermelho para alarme
const uint botao_A = 5; // Botão de abrir/fechar a porta
const uint botao_B = 6; // Botão para verificar proximidade do dono
const uint joystick_x = 26; // Pino do eixo X do joystick (ADC)
const uint joystick_y = 27; // Pino do eixo Y do joystick (ADC)
const uint botao_joystick = 22; // Pino do botão do joystick
const uint buzzer = 21; // Pino para o buzzer

// Estados dos LEDs e sensores
bool led_verde_on = false;
bool led_azul_on = false;
bool led_vermelho_on = false;
bool porta_aberta = false; // Estado da porta (aberta ou fechada)
bool movimento_detectado = false; // Alarme de movimento

// Flags para ações
bool tocar_buzzer_flag = false;

// Função para tocar o buzzer
void tocar_buzzer(int duracao_ms) {
    int tempo = duracao_ms;  // Duração total em milissegundos
    
    while (tempo > 0) {
        estado_do_buzzer = !estado_do_buzzer;
        gpio_put(buzzer, estado_do_buzzer);  // Liga o buzzer
        sleep_ms(1);             // Mantém o buzzer ligado por 1ms
        tempo -= 1;              // Diminui o tempo restante

        gpio_put(buzzer, false); // Desliga o buzzer
        sleep_ms(3);             // Aguarda 3ms antes de repetir
        tempo -= 3;              // Diminui o tempo restante
    }
}

// Função para atualizar o display OLED
void atualizar_display() {
    ssd1306_fill(&ssd, false);
    if (porta_aberta) {
        ssd1306_draw_string(&ssd, "Porta Aberta", 10, 10);
    } else {
        ssd1306_draw_string(&ssd, "Porta Fechada", 10, 10);
    }
    if (dono_proximo) {
        ssd1306_draw_string(&ssd, "Dono Proximo", 10, 20);
    } else {
        ssd1306_draw_string(&ssd, "Dono Longe", 10, 20);
    }
    if (movimento_detectado) {
        ssd1306_draw_string(&ssd, "Movimento ", 10, 30);
    }
    if (alarme_ativado) {
        ssd1306_draw_string(&ssd, "Alarme Ativado!", 10, 40);
    }
    ssd1306_send_data(&ssd);
}

// Função de interrupção para os botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    static uint32_t last_time = 0;
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Debouncing de 300ms
    if (current_time - last_time > 300000) {
        last_time = current_time;

        // Verifica se o botão A foi pressionado (abre ou fecha a porta)
        if (gpio == botao_A && !gpio_get(botao_A)) {
            if (dono_proximo) { // Só abre/fecha se o dono estiver próximo
                porta_aberta = !porta_aberta;
                led_verde_on = porta_aberta;
                gpio_put(led_verde, led_verde_on);
                atualizar_display();
            }
        }

        // Verifica se o botão B foi pressionado (verifica proximidade do dono)
        if (gpio == botao_B && !gpio_get(botao_B)) {
            dono_proximo = !dono_proximo; // Alterna entre perto e longe
            if (!dono_proximo && porta_aberta) {
                // Se o dono estiver longe e a porta estiver aberta, fecha a porta rapidamente
                porta_aberta = false;
                led_verde_on = false;
                gpio_put(led_verde, led_verde_on);
                atualizar_display();
            }
            // Se o alarme estiver ativado e o dono estiver próximo, desativa o alarme
            if (alarme_ativado && dono_proximo) {
                alarme_ativado = false;
                led_vermelho_on = false;
                gpio_put(led_vermelho, led_vermelho_on);
                gpio_put(buzzer, false); // Desliga o buzzer
                atualizar_display();
            }
            atualizar_display();
        }

        // Verifica se o botão do joystick foi pressionado
        if (gpio == botao_joystick && !gpio_get(botao_joystick)) {
            
            alarme_ativado = true; // Ativa o alarme
            led_vermelho_on = true; // Liga o LED vermelho
            gpio_put(led_vermelho, led_vermelho_on);
            tocar_buzzer_flag = true; // Sinaliza para tocar o buzzer
            atualizar_display();
        }
    }
}

// Função para ler o valor do eixo X ou Y do joystick
uint16_t ler_joystick(uint pino_adc) {
    adc_select_input(pino_adc - 26); // Converte o pino para o canal ADC (0 para pino 26, 1 para pino 27)
    return adc_read();
}

int main() {
    stdio_init_all(); // Inicializa o sistema, para funções como printf
    sleep_ms(2000);  // Aguarda 2 segundos antes de continuar

    // Inicializa os LEDs e botões, e os direciona como saída e entrada digitais
    gpio_init(led_verde);
    gpio_set_dir(led_verde, GPIO_OUT);
    gpio_init(led_azul);
    gpio_set_dir(led_azul, GPIO_OUT);
    gpio_init(led_vermelho);
    gpio_set_dir(led_vermelho, GPIO_OUT);
    gpio_init(botao_A);
    gpio_set_dir(botao_A, GPIO_IN);
    gpio_pull_up(botao_A); // Aciona o resistor de pull-up para o botão A
    gpio_init(botao_B);
    gpio_set_dir(botao_B, GPIO_IN);
    gpio_pull_up(botao_B); // Aciona o resistor de pull-up para o botão B
    gpio_init(botao_joystick);
    gpio_set_dir(botao_joystick, GPIO_IN);
    gpio_pull_up(botao_joystick); // Aciona o resistor de pull-up para o botão do joystick
    gpio_init(buzzer);
    gpio_set_dir(buzzer, GPIO_OUT); // Configura o buzzer como saída

    // Configuração do ADC para o joystick
    adc_init();
    adc_gpio_init(joystick_x);
    adc_gpio_init(joystick_y);

    // Configuração do I2C para o display OLED
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa o display OLED
    ssd1306_init(&ssd, 128, 64, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Configura interrupções para os botões
    gpio_set_irq_enabled_with_callback(botao_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botao_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botao_joystick, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true) {
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            switch (c) {
                case 'A': 
                    if (dono_proximo) {
                        porta_aberta = true;
                        gpio_put(led_verde, true);
                        atualizar_display();
                    }
                    break;
                case 'F': 
                    porta_aberta = false;
                    gpio_put(led_verde, false);
                    atualizar_display();
                    break;
                case 'L': 
                    alarme_ativado = true;
                    gpio_put(led_vermelho, true);
                    tocar_buzzer_flag = true;
                    atualizar_display();
                    break;
                case 'D': 
                    alarme_ativado = false;
                    gpio_put(led_vermelho, false);
                    gpio_put(buzzer, false);
                    atualizar_display();
                    break;
                default:
                    break;
            }
        }

        // Verifica se o joystick foi movido (eixo X ou Y)
        uint16_t valor_x = ler_joystick(joystick_x);
        uint16_t valor_y = ler_joystick(joystick_y);

        // Se o joystick for movido (valor fora da faixa central)
        if (valor_x < 1900 || valor_x > 2050 || valor_y < 1950 || valor_y > 2250) {
            movimento_detectado = true;
            led_azul_on = true; // Liga o LED azul
            gpio_put(led_azul, led_azul_on);
            porta_aberta = false; // Fecha a porta rapidamente
            led_verde_on = false;
            gpio_put(led_verde, led_verde_on);
            atualizar_display();
            sleep_ms(5000); // Mantém o LED azul ligado por 5 segundos
            led_azul_on = false; // Desliga o LED azul
            gpio_put(led_azul, led_azul_on);
            movimento_detectado = false; // Reseta o estado de movimento
            atualizar_display();
        }

        // Verifica se o buzzer precisa ser tocado
        if (tocar_buzzer_flag) {
            tocar_buzzer(5000);
            tocar_buzzer_flag = false;
        }

        sleep_ms(100); // Reduz a carga do processador
    }
}