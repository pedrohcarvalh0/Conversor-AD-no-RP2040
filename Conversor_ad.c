#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

// Definição dos pinos
#define GREEN_LED  11
#define BLUE_LED   12
#define RED_LED    13
#define BUTTON_A   5
#define VRX        26
#define VRY        27
#define BUTTON_JOYSTICK 22

// I2C
#define I2C_SDA 14
#define I2C_SCL 15
#define I2C_ID           i2c1
#define SSD1306_ADDR     0x3C   // Endereço do dispositivo I2C
#define I2C_FREQ         100000     // 100kHz

// Variáveis para PWM
const uint16_t PERIOD =   4095;       // Valor máximo do wrap
const float DIVIDER_PWM = 255.0;      // Divisor do clock para o PWM
bool pwm_enabled = true;

// Variáveis globais
static volatile uint32_t last_time = 0; // Para debouncing (em microssegundos)
ssd1306_t ssd;                          // Estrutura do display SSD1306

// Flags para atualizar o display fora da IRQ
volatile bool update_green_led_display = false;

// Protótipos das funções
void setup();
void gpio_irq_handler(uint gpio, uint32_t events);
void update_leds(int x, int y);
void update_display(int x, int y, int last_x_pos , int last_y_pos );
void handle_BUTTON_A();
void handle_BUTTON_JOYSTICK();

void setup() {
    // Inicializa o LED verde (simples, não usa PWM)
    gpio_init(GREEN_LED);
    gpio_set_dir(GREEN_LED, GPIO_OUT);
    gpio_put(GREEN_LED, 0);

    // Inicializa os LEDs Azul e Vermelho como PWM
    gpio_set_function(BLUE_LED, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(BLUE_LED);
    pwm_set_clkdiv(slice, DIVIDER_PWM);
    pwm_set_wrap(slice, PERIOD);
    pwm_set_gpio_level(BLUE_LED, 0);
    pwm_set_enabled(slice, true);

    gpio_set_function(RED_LED, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(RED_LED);
    pwm_set_clkdiv(slice, DIVIDER_PWM);
    pwm_set_wrap(slice, PERIOD);
    pwm_set_gpio_level(RED_LED, 0);
    pwm_set_enabled(slice, true);

    // Inicializa os botões
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_JOYSTICK);
    gpio_set_dir(BUTTON_JOYSTICK, GPIO_IN);
    gpio_pull_up(BUTTON_JOYSTICK);

    // Inicializa o ADC para o joystick
    adc_init();
    adc_gpio_init(VRX);
    adc_gpio_init(VRY);

    // Inicializa o I2C e o display SSD1306
    i2c_init(I2C_ID, I2C_FREQ);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, SSD1306_ADDR, I2C_ID);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    // Limpa o display
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

void update_leds(int x, int y) {
    if (pwm_enabled) {
        // Ajusta a intensidade: no centro (2048) o LED fica apagado, aumentando conforme a distância
        pwm_set_gpio_level(RED_LED, abs(x - 2048));
        pwm_set_gpio_level(BLUE_LED, abs(y - 2048));
    }
}

void update_display(int x, int y, int last_x_pos , int last_y_pos ) {
    // Normaliza os valores lidos para as posições no display
    int norm_x = x / 39 + 8;           // 4096/(116-8) ≈ 39
    int norm_y = (4096 - y) / 91 + 7;    // 4096/(54-8) ≈ 91

    int norm_last_x_pos  = last_x_pos  / 39 + 8;
    int norm_last_y_pos  = (4096 - last_y_pos ) / 91 + 7;

    // Apaga o quadrado na posição anterior
    ssd1306_rect(&ssd, norm_last_y_pos , norm_last_x_pos , 8, 8, 0, 1);
    // Desenha o quadrado na nova posição
    ssd1306_rect(&ssd, norm_y, norm_x, 8, 8, 1, 1);
    ssd1306_send_data(&ssd);
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Debouncing: somente processa se passaram 300ms do último evento
    if (current_time - last_time > 300000) {
        if (gpio == BUTTON_A) {
            handle_BUTTON_A();
        } else if (gpio == BUTTON_JOYSTICK) {
            handle_BUTTON_JOYSTICK();
        }
        last_time = current_time; // Atualiza o tempo do último evento
    }
}

void handle_BUTTON_A() {
    // Alterna o estado dos LEDs (PWM ligado/desligado)
    pwm_enabled = !pwm_enabled;
    uint slice = pwm_gpio_to_slice_num(BLUE_LED);
    pwm_set_enabled(slice, pwm_enabled);
    slice = pwm_gpio_to_slice_num(RED_LED);
    pwm_set_enabled(slice, pwm_enabled);
}

void handle_BUTTON_JOYSTICK() {
    // Alterna o LED verde
    gpio_put(GREEN_LED, !gpio_get(GREEN_LED));
    update_green_led_display = true;
}

int main() {
    stdio_init_all();
    setup();

    // Configura interrupções para os botões
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_JOYSTICK, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Desenha elementos estáticos do display
    ssd1306_rect(&ssd, 3, 3, 122, 60, 1, 0);  // Borda Inicial
    ssd1306_send_data(&ssd);

    uint16_t vrx_value, vry_value;                // Valores lidos do joystick (0 ~ 4095)
    uint16_t last_x_pos  = 0, last_y_pos  = 0;    // Para rastrear a posição anterior do quadrado

    while (true) {
        // Leitura dos eixos do joystick
        adc_select_input(1);
        vrx_value = adc_read();

        adc_select_input(0);
        vry_value = adc_read();

        // Atualiza intensidade dos LEDs (PWM)
        update_leds(vrx_value, vry_value);

        // Atualiza a posição do quadrado no display
        update_display(vrx_value, vry_value, last_x_pos , last_y_pos );
        last_x_pos  = vrx_value;
        last_y_pos  = vry_value;

        // Se a flag estiver ativa, atualiza os elementos relacionados ao LED verde
        if (update_green_led_display) {
            // Atualiza a borda do display conforme o estado do LED verde
            ssd1306_rect(&ssd, 3, 3, 122, 60, 1, 0);  // Borda sólida
            ssd1306_rect(&ssd, 6, 6, 116, 54, gpio_get(GREEN_LED), 0);
            ssd1306_send_data(&ssd);
            update_green_led_display = false;
        }

        sleep_ms(10); // Delay para evitar leituras excessivamente rápidas
    }
}