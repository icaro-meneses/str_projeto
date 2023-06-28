# Projeto de Automação Residencial

Este projeto consiste em um sistema de automação residencial baseado no microcontrolador STM32F411CEU6. Ele utiliza o sistema operacional de tempo real FreeRTOS e a biblioteca HAL (Hardware Abstraction Layer) da STMicroelectronics para configurar o microcontrolador e os periféricos.

## Funcionalidades

### Tarefas Periódicas

1. **Ler Temperatura**: Esta tarefa é responsável por ler a temperatura ambiente usando um sensor simulado.

2. **Monitorar Presença**: Esta tarefa monitora a presença em determinadas áreas da residência por meio de sensores de presença e toma ações com base nessa detecção.

3. **Gerenciar Luzes**: Esta tarefa controla o acionamento das luzes em diferentes ambientes da residência, fornecendo controle automático com base em temporizadores e detecção de presença.

4. **Monitorar Segurança**: Esta tarefa monitora a segurança da residência, fazendo a detecção de instrusos nos horários programados, e toma ações apropriadas em caso de violação.

5. **Enviar Dados via Rede**: Esta tarefa é responsável por enviar dados coletados dos sensores simulados e informações do sistema por meio de uma rede de comunicaçãoi.

### Tarefas Aperiódicas

1. **Ligar Alarme**: Esta tarefa é acionada em caso de violação de segurança e é responsável por ativar o alarme sonoro e/ou visual.

2. **Ligar Luz de Presença**: Esta tarefa é acionada quando a presença é detectada em determinados ambientes e é responsável por ligar a luz de presença automaticamente.

## Configuração do Projeto

O projeto foi desenvolvido na IDE STMCubeIDE, fornecida pela STMicroelectronics, e utiliza a biblioteca HAL (Hardware Abstraction Layer) da STM para configurar o microcontrolador e os periféricos associados.

Para executar o projeto em modo de depuração, utilize o OpenCD em conjunto com o GDB para visualizar as mensagens das tarefas e depurar o código.

