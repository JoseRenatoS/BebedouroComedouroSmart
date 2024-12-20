# Bebedouro e Comedouro Inteligente controlado via Telegram, para Pets


<p align="center">
  <img src="http://img.shields.io/static/v1?label=STATUS&message=CONCLUIDO&color=GREEN&style=for-the-badge"/>
</p>

<!--
<p align="center">
  <img src="https://github.com/user-attachments/assets/2aef1242-0a4b-43fe-b11e-9ba5767dd0ac" width="650" height="500"/>
</p>
<p align="center">Figura 1: Protótipo do Sistema de Automação Residencial</p>
<br><br>
-->

## :round_pushpin: Índice 
* [:house: Descrição do Projeto](#house-descrição-do-projeto)
* [:hammer: Funcionalidades do projeto](#hammer-funcionalidades-do-projeto)
* [:computer: Pré-Requisitos](#computer-pré-requisitos)
* [:toolbox: Materiais & Custos](#toolbox-materiais--custos)
* [:memo: Licença](#memo-licença)
* [:bookmark_tabs: Referências](#bookmark_tabs-referências)
<br><br><br><br><br>





## :house: Descrição do Projeto
Trata-se de um projeto da disciplina de Automação Residencial, do Curso de Engenharia Elétrica. Tendo como objetivo desenvolver protótipo ligado à automação residencial. A partir disso, foi proposto a ideial de desenvolver o prótotipo de um bebedouro e comedouro controlado através do Telegram.
<br><br><br><br><br>



## :hammer: Funcionalidades do projeto

- `função`: texto.
<br><br><br><br><br>



## :computer: Pré-requisitos

Para utilizar o projeto, alguns requisitos são necessário:

- Ter um computador para realizar qualquer alteração necessária no código-fonte, e para transferir os códigos para os microcontroladores correspondentes.
- Ter uma conta no Telegram, e o aplicativo instalado.
- Criar um Bot no Telegram e pegar a chave Token do Bot criado (use o [BotFather](https://t.me/BotFather) para realizar este processo).
- Identificar o seu ID de usuário no Telegram (use o [IDBot](https://t.me/myidbot) para realizar essa etapa).

> [!NOTE]
> *Caso não saiba criar o Bot no Telegram e identificar o ID do Usuário. Siga o passo a passo em [Random Nerd Tutorials](https://randomnerdtutorials.com/telegram-control-esp32-esp8266-nodemcu-outputs/).*

<br><br><br><br><br>



## :toolbox: Materiais & Custos
| Componentes | Preço Unitário (R$) | Quantidade | Preço (R$) |
|----------|:----------:|:----------:|:----------:|
| ESP32 Doit DevKitC V4 WROOM-32D  | 56,78 | 1 | 56,78 |
| Sensor de Nível de Água  | 7,47 | 1 | 7,47 |
| Mini Bomba de Água Submersa 3V-5V | 12,90 | 1 | 12,90 |
| Micro Servo Motor | 25,02 | 1 | 25,02 |
| Resistores de 10kΩ | 0,90 | 1 | 0,90 |
| Botão | 2,14| 1 | 2,14 |
| Protoboard 830 pinos | 12,90 | 1 | 12,90 |
| Fonte Bivolt 9V/1A | 10,90 | 1 | 10,90 |
| **TOTAL**              ||| **R$ 129,01** |
<br>

> [!NOTE]
> *Os valores estão sujeitos à alterações, a depender de diversos fatores.*

> [!NOTE]
> *Não foi considerado o gasto com a fiação do projeto.*

<br><br><br><br><br>



## :memo: Licença
Este projeto está licenciado nos termos da [MIT License]().
<br><br><br><br><br>



## :bookmark_tabs: Referências
* Repositório da biblioteca [Universal Telegram Bot Library](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot?tab=readme-ov-file) utilizada para fazer a conexão entre o ESP32 e o API do Telegram Bot.
* A ideia de utilizar controle do sistema via Telegram, veio da inspiração do projeto [Telegram: Control ESP32/ESP8266 Outputs (Arduino IDE)](https://randomnerdtutorials.com/telegram-control-esp32-esp8266-nodemcu-outputs/) da Random Nerd Tutorials
