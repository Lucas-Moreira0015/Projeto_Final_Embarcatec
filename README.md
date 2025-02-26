# Sistema de Controle de Acesso e Alarme

## Descrição
Este projeto é um sistema de controle de acesso e alarme baseado na placa Raspberry Pi Pico, utilizando sensores, LEDs, botões e um display OLED para monitoramento e interação.

## Funcionalidades
1. **Controle de Acesso**: Permite abrir e fechar a porta por meio de um botão (Botão A), desde que o dono esteja próximo.
2. **Verificação de Proximidade**: O dono pode ser identificado através de um botão (Botão B).
3. **Alarme de Movimento**: Detecta movimentação através do joystick e ativa um alerta.
4. **Sistema de Alerta**:
   - LED vermelho e buzzer ativados em caso de alarme.
   - LED azul ativado ao detectar movimento.
   - LED verde indica porta aberta.
5. **Interface de Display**: O display OLED exibe o status atual da porta, proximidade do dono e ativação do alarme.

## Componentes Utilizados
- **Placa**: Raspberry Pi Pico
- **Display OLED**: SSD1306 (I2C)
- **Botões**: A e B para interação
- **Joystick**: Utilizado para detecção de movimento
- **LEDs**:
  - Verde (porta aberta)
  - Azul (movimento detectado)
  - Vermelho (alarme ativado)
- **Buzzer**: Emite som quando o alarme é acionado

## Configuração de Hardware
- **I2C**:
  - SDA: GPIO 14
  - SCL: GPIO 15
- **Botões**:
  - Botão A: GPIO 5
  - Botão B: GPIO 6
  - Botão do Joystick: GPIO 22
- **Joystick**:
  - Eixo X: GPIO 26 (ADC)
  - Eixo Y: GPIO 27 (ADC)
- **LEDs**:
  - Verde: GPIO 11
  - Azul: GPIO 12
  - Vermelho: GPIO 13
- **Buzzer**: GPIO 21

## Lógica de Funcionamento
1. **Controle de Acesso**:
   - Pressionar o Botão A abre/fecha a porta se o dono estiver próximo.
   - Se o dono se afastar com a porta aberta, ela fecha automaticamente.
2. **Detecção de Movimento**:
   - Se o joystick for movido, o sistema detecta movimentação e ativa o LED azul por 5 segundos.
3. **Sistema de Alarme**:
   - Pressionar o botão do joystick ativa o alarme (LED vermelho e buzzer).
   - Se o dono estiver próximo, pressionar o Botão B desativa o alarme.
4. **Atualização do Display**:
   - Exibe o status da porta, proximidade do dono e alarme ativado.

## Dependências
Este projeto utiliza as bibliotecas:
- **pico/stdlib.h**
- **hardware/i2c.h**
- **hardware/gpio.h**
- **hardware/adc.h**
- **hardware/pwm.h**
- **hardware/clocks.h**
- **hardware/timer.h**
- **ssd1306.h** (para controle do display OLED)

## Compilação e Execução
1. Instale o SDK do Raspberry Pi Pico.
2. Compile o código com CMake.
3. Suba o binário para a placa Pico e execute o programa.

## Autores
Desenvolvido para aplicações de segurança residencial e automação.

**Autor: Lucas Moreira da Silva**

