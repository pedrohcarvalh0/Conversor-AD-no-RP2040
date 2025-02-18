# Controle de LEDs e Display OLED com Joystick - RP2040 + BitDogLab

Este projeto utiliza o microcontrolador **RP2040** com a placa **BitDogLab**, permitindo controlar a intensidade de **LEDs RGB via joystick analógico usando um conversor analógico-digital**, além de movimentar um **quadrado no display OLED SSD1306**. O sistema inclui **interrupções (IRQ) para botões**, **PWM para LEDs RGB** e **I2C para comunicação com o display**.

---

## 📌 **Funcionalidades**
### 🎛️ **1. Controle de LEDs RGB via Joystick**
- O **LED Azul** ajusta sua intensidade com base no **eixo Y** do joystick.
- O **LED Vermelho** ajusta sua intensidade com base no **eixo X** do joystick.
- No centro do joystick (valor **2048**), os LEDs ficam **apagados**. Nos extremos (**0 ou 4095**), atingem **intensidade máxima**.

### 🖥️ **2. Movimento do Quadrado no Display OLED (SSD1306)**
- Um **quadrado de 8x8 pixels** se move proporcionalmente aos valores do joystick.
- Inicialmente centralizado e atualizado conforme o usuário move o joystick.

### 🔘 **3. Botões e Interrupções**
- **Botão do Joystick (BUTTON_JOYSTICK)**  
  - Alterna o **LED Verde** ligado/desligado.  
  - Modifica a **borda do display** para indicar ativação.  

- **Botão A (BUTTON_A)**  
  - Ativa/desativa os **LEDs RGB controlados por PWM**.  

---

## 🛠️ **Tecnologias e Bibliotecas Utilizadas**
- **Microcontrolador RP2040 (BitDogLab)**
- **C/C++ para desenvolvimento do firmware**
- **Protocolo I2C** para comunicação com **display SSD1306**
- **PWM** para controle de intensidade dos **LEDs RGB**
- **ADC (Conversão Analógica-Digital)** para leitura do **joystick**
- **Interrupções (IRQ) com debouncing via software** para os botões

---

## 🔧 **Estrutura do Código**
### `setup()`
Configura **GPIOs, ADC, PWM, I2C** e inicializa o **display OLED**.

### `update_leds(x, y)`
Atualiza a **intensidade dos LEDs** de acordo com o **joystick**.

### `update_display(x, y, last_x, last_y)`
Move o **quadrado no display** conforme os valores do joystick.

### `gpio_irq_handler()`
Trata as **interrupções dos botões** e aplica **debouncing**.

### `handle_BUTTON_A()`
Alterna o estado dos **LEDs PWM**.

### `handle_BUTTON_JOYSTICK()`
Liga/desliga o **LED Verde** e atualiza a **borda do display**.

### `main()`
Executa o **loop principal**:
1. Lê os valores do **joystick**.
2. Atualiza **LEDs RGB e display**.
3. Verifica mudanças no botão do **joystick**.

---

## 🚀 Compilação e Upload

```plaintext
1. Clone o repositório do código:
   git clone <URL_DO_REPOSITORIO>

2. Compile o programa usando CMake:
   mkdir build
   cd build
   cmake ..
   make

   Também é possível compilar diretamente na  extensão do Raspberry Pi Pico Project no VS Code clicando em "Compile Project"

3. Envie o arquivo `.uf2` gerado para o Raspberry Pi Pico:
   - Mantenha o botão `BOOTSEL` pressionado enquanto conecta o Pico ao computador.
   - Através da extensão do Raspberry Pi Pico Project no VS Code, é possível enviar o código para a placa clicando em "Run Project (USB)".
   - Caso não consiga enviar diretamente, copie o arquivo `.uf2` gerado para a unidade montada.

```

## 🎥 Explicação e Demonstração

```plaintext
https://youtu.be/jjBdjFtQ50k
```

---
