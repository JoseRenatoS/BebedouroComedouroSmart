#include <WiFi.h>                                                             // biblioteca usada para conectar o ESP32 a uma rede Wi-Fi
#include <WiFiClientSecure.h>                                                 // biblioteca que estabelece uma conexão segura com o Wi-Fi
#include <UniversalTelegramBot.h>                                             // biblioteca para interagir com a API do Telegram
#include <ESP32Servo.h>                                                       // biblioteca para o ESP32 controlar servomotores
#include <NTPClient.h>                                                        // biblioteca usada para conectar a data e horário da internet   (https://github.com/taranais/NTPClient)
#include <ArduinoJson.h>

// Inicializando a conexao Wi-Fi
char ssid[] = "nome_do_wifi";                                                  
char password[] = "senha_do_wifi";                                              


// Inicializa o BOT Telegram - copie aqui a chave Token quando configurou o seu BOT - entre aspas
#define BOTtoken "inserir_chave_Token_aqui"

#define ID_CHAT "inserir_ID_do_telegram_do_usuario_aqui"


// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient ntp(ntpUDP);

WiFiClientSecure client;                                                      // Cria um cliente seguro para conexões Wi-Fi
UniversalTelegramBot bot(BOTtoken, client);                                   // Inicializa o bot do Telegram com a chave de acesso definida e o cliente WiFi seguro                      
Servo servoMotor;

int Bot_mtbs = 1000;                                                          // define o tempo entre a leitura das mensagens do bot
long Bot_lasttime;                                                            // Armazena o tempo da última mensagem recebida
bool Start = false;                                                           // Variável booleana usada para controlar o início

const int intervalo = 1000;                                                   // Defina a frequência de amostragem (em milissegundos) [1 segundo]
const int aguaPin = 16;                                                       // nomeia o pino 16 como aguaPin (responsável por acionar o relé que liga a bomba d'água)
const int comidaPin = 26;                                                     // nomeia o pino 26 como comidaPin (responsável por acionar o servomotor que libera ração)
const int aguaSensor = 36;                                                    // nomeia o pino 36 como aguaSensor (recebe os dados do sensor de nível de água)
float comidaDesejada = 0;                                                     // variável responsável por armazenar o peso de ração desejado (g)
float vazaoComida = 50;                                                       // variável que armazena a quantidade de ração que sai pela tubulação (g)
double aguaMedida = 0;                                                        // variável responsável por armazenar a quantidade de água medida (ml)
float aguaDesejada = 0;                                                       // variável responsável por armazenar a quantidade de água desejada (ml)
float calibration_factor = 42130;                                             // fator de calibração para teste inicial
int flowPin = 2;                                                              // Pino de entrada no ESP32
volatile int count;                                                           // Contador de pulsos
int countComida = 0;                                                          // contador para água e comida
int i;                                                                        // contador usado para verificar os agendamentos
int ano, mes, dia, hora, minuto;                                              // variáveis usadas para armazenar o ano, mês, dia, hora e minuto atuais

int botaoPIN = 32;                                                            // nomeia o pino 32 como botaoPIN (botão usado para fornecer água e ração, quando não houver internet)
int estadoBotao = 0;                                                          // Variável responsável por armazenar o estado do botão (ligado/desligado)

// Variáveis globais para controle do tempo e estado do servo
bool liberandoComida = false;
unsigned long tempoInicio = 0;
unsigned long tempoInicioAgua = 0;
bool liberandoAgua = false;
const unsigned long TIMEOUT = 10000;
unsigned long duracao = 0;


struct agendamento {
  int startDay, startMonth, startYear;                                        // Data (dia, mês, ano) de início
  int endDay, endMonth, endYear;                                              // Data (dia, mês, ano) de término
  int hour, minute;                                                           // Hora e minuto
  String type;                                                                // Tipo: "comida", "agua" ou "ambos"
  float comidaAgendada;                                                       // Quantidade de ração (em gramas)
  float aguaAgendada;                                                         // Quantidade de água (em mililitros)
};


agendamento agenda[10];                                                       // Suporta até 10 agendamentos
int agendamentoContador = 0;                                                  // Contador de agendamentos


void apagarAgendamento(int posicao) {
    int posicaoExcluida = posicao - 1;

    if (posicaoExcluida >= 0 && posicaoExcluida < agendamentoContador) {
        // Limpa os dados da posição especificada
        agenda[posicaoExcluida] = {0, 0, 0, 0, 0, 0, 0, 0, "", 0.0, 0.0};
        
        // Move os agendamentos posteriores uma posição para frente
        for (int i = posicaoExcluida; i < agendamentoContador - 1; i++) {
            agenda[i] = agenda[i + 1];
        }

        // Marca a última posição como vazia
        agenda[agendamentoContador - 1] = {0, 0, 0, 0, 0, 0, 0, 0, "", 0.0, 0.0};

        // Reduz o contador de agendamentos
        agendamentoContador--;
    } else {
        Serial.println("Posição inválida!");
    }
}


String gerarResumoAgendamentos() {
  String resumo = "=== Lista de Agendamentos ===\n";
  for (int i = 0; i < 10; i++) {
    resumo += "-------------------------\n";
    resumo += "Agendamento " + String(i + 1) + ":\n";
    
    resumo += "   Data de Início: " + String(agenda[i].startDay) + "/" + String(agenda[i].startMonth) + "/" + String(agenda[i].startYear) + "\n";
    resumo += "   Data de Término: " + String(agenda[i].endDay) + "/" + String(agenda[i].endMonth) + "/" + String(agenda[i].endYear) + "\n";

    resumo += "   Hora: " + String(agenda[i].hour) + ":";
    if (agenda[i].minute < 10) {
      resumo += "0"; // Adiciona zero antes de minutos menores que 10
    }
    resumo += String(agenda[i].minute) + "\n";

    resumo += "   Tipo: " + agenda[i].type + "\n";

    if(String(agenda[i].type) == "comida") {
      resumo += "   Comida Agendada: " + String(agenda[i].comidaAgendada) + "g\n";
    }
    if(String(agenda[i].type) == "agua") {
      resumo += "   Água Agendada: " + String(agenda[i].aguaAgendada) + "ml\n";
    }
    if(String(agenda[i].type) == "ambos") {
      resumo += "   Comida Agendada: " + String(agenda[i].comidaAgendada) + "g\n";
      resumo += "   Água Agendada: " + String(agenda[i].aguaAgendada) + "ml\n";
    }

  }
  return resumo;
}



void liberarComida(float comidaDesejada) {
  servoMotor.write(0);                                                                  // Gira o servo para liberar ração
  tempoInicio = millis();                                                               // Marca o tempo de início

  duracao = (comidaDesejada / vazaoComida) * 1000;                                      // Calcula duração em milissegundos
  
  while(true) {                                                                         // Verifica se já passou o tempo necessário
    if ((millis() - tempoInicio >= duracao)) {
      servoMotor.write(180);                                                            // Retorna o servo para posição inicial
      break;
    }
  }
}



void liberarAgua(float quantidade) {
  tempoInicioAgua = millis();
  digitalWrite(aguaPin, HIGH);                                                         // Aciona a bomba d'água
  liberandoAgua = true;

  while(true) {
    aguaMedida = analogRead(aguaSensor);                                               // Realiza a leitura do sensor
    aguaMedida = map(aguaMedida, 0, 2048, 0, 475);                                     // converte o valor para uma nova escala
    Serial.println(aguaMedida);

    if (aguaMedida >= quantidade && quantidade > 0) {
      digitalWrite(aguaPin, LOW);                                                      // Desliga a bomba d'água
      break;
    }

    // Verifica o timeout
    if (millis() - tempoInicioAgua >= TIMEOUT) {
      Serial.println("Erro: Timeout ao liberar água.");
      digitalWrite(aguaPin, LOW);                                                     // Desliga a bomba d'água
      break;
    }
  }
}




void handleNewMessages(int numNewMessages) {                                  // Declaração de uma função chamada handleNewMessages que recebe o número de novas mensagens como argumento
  Serial.print("Mensagem recebida = ");                                       // Imprime no console a mensagem "Mensagem recebida = "
  Serial.println(String(numNewMessages));                                     // Imprime no console o número de novas mensagens recebidas


  for (int i = 0; i < numNewMessages; i++) {                                  // loop que percorre as novas mensagens recebidas
    String chat_id = String(bot.messages[i].chat_id);                         // obtém o ID da conversa da mensagem atual, em formato string

    if (chat_id != ID_CHAT) {                                                 // compara o ID
      bot.sendMessage(chat_id, "Usuário não autorizado", "");
      continue;
    }
    
    String text = bot.messages[i].text;                                       // obtém o texto da mensagem atual
    String from_name = bot.messages[i].from_name;                             // obtém o nome do remetente da mensagem atual.
    if (from_name == "") {
      from_name = "Usuário desconhecido";                                     // Verifica se o nome do remetente está vazio e, se sim, define como "Usuário desconhecido"
    }

    if(text.startsWith("/agendar")) {

      if (agendamentoContador >= 10) {
        bot.sendMessage(chat_id, "ERRO: Limite de agendamentos atingido.", "");
      } else {
        int inicioDia, inicioMes, inicioAno;
        int fimDia, fimMes, fimAno;
        int horas, minutos;
        char categoria[10];
        float comidaProgramada  = 0;
        float aguaProgramada = 0;

        sscanf(text.c_str(), "/agendar %d-%d-%d %d-%d-%d %d:%d %s %f", &inicioDia, &inicioMes, &inicioAno, &fimDia, &fimMes, &fimAno, &horas, &minutos, categoria, &comidaProgramada);

        if (strcmp(categoria, "agua") == 0) {
          aguaProgramada = comidaProgramada;                                        // Se o tipo for "agua", usa o valor como quantidade de água
          comidaProgramada = 0;
        } else if (strcmp(categoria, "ambos") == 0) {
          sscanf(text.c_str(), "/agendar %d-%d-%d %d-%d-%d %d:%d %s %f %f", &inicioDia, &inicioMes, &inicioAno, &fimDia, &fimMes, &fimAno, &horas, &minutos, categoria, &comidaProgramada, &aguaProgramada);
        }

        // Armazena o agendamento
        agenda[agendamentoContador].startDay = inicioDia;
        agenda[agendamentoContador].startMonth = inicioMes;
        agenda[agendamentoContador].startYear = inicioAno;
        agenda[agendamentoContador].endDay = fimDia;
        agenda[agendamentoContador].endMonth = fimMes;
        agenda[agendamentoContador].endYear = fimAno;
        agenda[agendamentoContador].hour = horas;
        agenda[agendamentoContador].minute = minutos;
        agenda[agendamentoContador].type = String(categoria);
        agenda[agendamentoContador].comidaAgendada = comidaProgramada;
        agenda[agendamentoContador].aguaAgendada = aguaProgramada;
        agendamentoContador++;

        String reply = "Agendamento de "+ String(categoria) + " criado para " + String(horas) + ":" + String(minutos) + ", valido de " +  String(inicioDia) + "/" + String(inicioMes) + "/" + String(inicioAno) + " até " + String(fimDia) + "/" + String(fimMes) + "/" + String(fimAno) + ".";
        bot.sendMessage(chat_id, reply, "Markdown");
      }


    } else if (text.startsWith("/comida")) {                                  // verifica se o texto inicia com "/comida"
      String numeroString = text.substring(8);                                // Extrai a quantidade da string a partir da 8 posição
      comidaDesejada = numeroString.toFloat();                                // Converte para float
      liberarComida(comidaDesejada);                                          // Função usada para o fornecimento de ração
      bot.sendMessage(chat_id, "Comida fornecida com sucesso!", "");          // envia mensagem



    } else if (text.startsWith("/agua")) {                                    // verifica se o texto inicia com "/agua"
      String numeroString = text.substring(6);                                // Extrai a quantidade da string a partir da 8 posição
      aguaDesejada = numeroString.toFloat();                                  // Converte para float
      liberarAgua(aguaDesejada);                                              // Função usada para o fornecimento de agua
      bot.sendMessage(chat_id, "Agua fornecida com sucesso!", "");            // envia mensagem



    } else if (text.startsWith("/ambos")) {                                   // verifica se o texto inicia com "/ambos"
      int primeiroEspaco = text.indexOf(' ');                                 // Localiza o primeiro espaço após "/ambos"
      int segundoEspaco = text.indexOf(' ', primeiroEspaco + 1);              // Localiza o segundo espaço
      
      // Verifica se ambos os espaços foram encontrados
      if (primeiroEspaco != -1 && segundoEspaco != -1) {
        // Extrai as quantidades de ração e água
        String quantidadeComidaStr = text.substring(primeiroEspaco + 1, segundoEspaco);         // Quantidade de comida
        String quantidadeAguaStr = text.substring(segundoEspaco + 1);                           // Quantidade de água

        // Converte as strings para float
        float qtdComida = quantidadeComidaStr.toFloat();
        float qtdAgua = quantidadeAguaStr.toFloat();

        // Verifica se os valores são válidos
        if (qtdComida > 0 && qtdAgua > 0) {
          liberarComida(qtdComida);                                           // Chama a função para liberar a comida
          liberarAgua(qtdAgua);                                               // Chama a função para liberar a água

          // Envia mensagem de sucesso ao usuário
          bot.sendMessage(chat_id, "Comida e água fornecidas com sucesso!", "");  
        } else {
          // Envia mensagem de erro se os valores não forem válidos
          bot.sendMessage(chat_id, "Por favor, informe quantidades válidas para comida e água.", "");  
        }
      } else {
        // Envia mensagem de erro se o formato do comando estiver incorreto
        bot.sendMessage(chat_id, "Formato incorreto. Use: /ambos <quantidade_comida> <quantidade_agua>", "");
      }
   
    } else if (text.startsWith("/status")) {                                  // comando usado para exibir a quantidade de água e de ração nos potes e nos reservatórios
      aguaMedida = analogRead(aguaSensor);                                    // Realiza a leitura do sensor
      aguaMedida = map(aguaMedida, 0, 2048, 0, 475);                          // converte o valor para uma nova escala
      Serial.println(aguaMedida);

      String statusMsg = "Status Atual:\n";
      //statusMsg += "-> Ração disponível no pote: " + String(comidaMedida) + "g\n";
      statusMsg += "-> Água disponível no pote: " + String(aguaMedida) + "ml\n";
      bot.sendMessage(chat_id, statusMsg, "");


    } else if (text.startsWith("/agendamentos")) {                            // comando usado para verificar os agendamentos
      String resumo = gerarResumoAgendamentos();
      bot.sendMessage(chat_id, resumo, "Markdown");


    } else if (text.startsWith("/apagar")) {                                  // comando usado para excluir agendamentos (liberar espaço)
      String numeroString = text.substring(8);                                // Extrai a quantidade da string a partir da 8 posição
      int numeroInteiro = numeroString.toInt();                               // Converte a string em número inteiro
      apagarAgendamento(numeroInteiro);                                       // Função usada para apagar o agendamento especificado
      bot.sendMessage(chat_id, "Agendamento apagado com sucesso!", "");       // envia mensagem


    } else {                                                                  // comando usado iniciar
      String welcome = "Bem-vindo ao Bebedouro e Comedouro Inteligente para Pets, " + from_name + ".\n";
      welcome += "O que você deseja fazer?\n\n";
      welcome += "--> Fornecer APENAS ração (em gramas). Digite o comando /comida <quantidade>\n\n";
      welcome += "--> Fornecer APENAS água (em militros). Digite o comando /agua <quantidade>\n\n";
      welcome += "--> Fornecer AMBOS (comida e água). Digite o comando /ambos <quantidade_comida> <quantidade_agua>\n\n";
      welcome += "--> Realizar o agendamento de APENAS água ou comida. Digite o comando /agendar <dia-mês-ano> <dia-mês-ano> <hora:minuto> <agua_ou_comida> <quantidade>\n\n";
      welcome += "--> Realizar o agendamento de AMBOS (comida e água). Digite o comando /agendar <dia-mês-ano> <dia-mês0ano> <hora:minuto> ambos <quantidade_comida> <quantidade_agua>\n\n";
      welcome += "--> Verificar a quantidade restante de ração e de água nos potes. Digite o comando /status\n\n";
      welcome += "--> Exibir todos os agendamentos. Digite o comando /agendamentos\n\n";
      welcome += "--> Para apagar um agendamento de cada vez. Digite o comando /apagar <numero_agendamento>";
      bot.sendMessage(chat_id, welcome, "");
    }
  }
}






void setup() {
  Serial.begin(115200);                                                       // inicializa a comunicação serial com uma taxa de transmissão de 115200 bps por segundo

  servoMotor.attach(comidaPin);                                               // inicializa o servo motor no pino especificado por SERVO_PIN
  servoMotor.write(180);                                                      // Servo no ângulo inicial de 0 graus


  WiFi.mode(WIFI_STA);                                                        // Configura o WIFI do NodeMCU para modo estação WIFI_STA
  WiFi.disconnect();                                                          // desconecta o WIFI, se houver alguma conexão existente
  delay(100);                                                                 // atraso de 100 milissegundos
                

  
  Serial.print("Conectando Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);                                // Add root certificate for api.telegram.org



  Serial.println("Conectando-se no WiFi...");
  while (WiFi.status() != WL_CONNECTED) {                                     // aguardando a conexão WEB
    delay(100);                                                               // atraso de 0,5 segundos
    Serial.println(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");                                           // WIFI conectado
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());



  // Inicializa um NTPClient para obter o horário
  ntp.begin();
  // GMT em segundos - Definir o tempo de deslocamento em segundos para ajustar ao seu fuso horário
  // +1 = 3600             +8 = 28800                -1 = -3600                    -3 = -10800 (BRASIL)
  ntp.setTimeOffset(-10800);



  pinMode(aguaPin, OUTPUT);                                                   // configura o pino da água como saida
  pinMode(botaoPIN, INPUT);                                                   // configura o pino do botão como entrada
  digitalWrite(aguaPin, LOW);                                                 // inicializa o pino da água como apagado
  digitalWrite(comidaPin, LOW);                                               // inicializa o pino da comida como apagado
  delay(500);

  

  // MEDIÇÃO DA QUANTIDADE DE ÁGUA NO POTE
  // o nível de água medido no pino 18 exibirá um valor de 0 a 4095, ao multiplicar por 0.1172, será obtido o valor em mililitro para um pote de água de 480ml
  aguaMedida = analogRead(aguaSensor);                                        // Realiza a leitura do sensor
  aguaMedida = map(aguaMedida, 0, 2048, 0, 475);                              // converte o valor para uma nova escala
  Serial.println(aguaMedida);

  delay(500);                                                                 // atraso de 500 milissegundos
}





void loop() {
  estadoBotao = digitalRead(botaoPIN);                                        // Lê o valor de botaoPIN e armazena em estadoBotao
  if (estadoBotao == HIGH) {                                                  // Se estadoBotao for igual a HIGH (1)
    liberarComida(300);                                                       // libera 250g de ração
    delay(100);                                                               // atraso de 100 milissegundos
    liberarAgua(300);                                                         // libera 250ml de água
    delay(100);                                                               // atraso de 100 milissegundos
    Serial.println("Botão acionado");
  }


  if (ntp.update()) {
    String dataHora = ntp.getFormattedDate();                                 // Exemplo: "2024-12-10T10:37:04Z"

    // Separar dia, mês, ano, hora e minuto
    ano = dataHora.substring(0, 4).toInt();                                   // exemplo: 2024
    mes = dataHora.substring(5, 7).toInt();                                   // exemplo: 12
    dia = dataHora.substring(8, 10).toInt();                                  // exemplo: 10
    hora = dataHora.substring(11, 13).toInt();                                // exemplo: 10
    minuto = dataHora.substring(14, 16).toInt();                              // exemplo: 37

    // Exibir os valores separados
    Serial.println("DATA E HORA FORMATADA: " + dataHora);
    Serial.println("Ano: " + String(ano));
    Serial.println("Mes: " + String(mes));
    Serial.println("Dia: " + String(dia));
    Serial.println("Hora: " + String(hora));
    Serial.println("Minuto: " + String(minuto));

  } else {
    Serial.println("!Erro ao atualizar NTP!");
  }
  Serial.println("----------------------------------\n\n");
  delay(500);






  if (millis() > Bot_lasttime + Bot_mtbs) {                                  // controlando as mensagens
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {                                                 // numero de novas mensagens
      Serial.println("Resposta recebida do Telegram");                       // exibe a mensagem no console
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    Bot_lasttime = millis();
  }



  // Verifica se há algum agendamento ativo
  for (int i = 0; i < agendamentoContador; i++) {
    if (ano >= agenda[i].startYear && ano <= agenda[i].endYear && mes >= agenda[i].startMonth && mes <= agenda[i].endMonth && dia >= agenda[i].startDay && dia <= agenda[i].endDay && hora == agenda[i].hour && minuto == agenda[i].minute) {
      if(agenda[i].type == "comida" || agenda[i].type == "ambos") {
        liberarComida(agenda[i].comidaAgendada);
      }
      if(agenda[i].type == "agua" || agenda[i].type == "ambos") {
        liberarAgua(agenda[i].aguaAgendada);
      }
    } 

    // usado para apagar qualquer agendamento que tenha datas antigas
    if (ano > agenda[i].startYear && ano > agenda[i].endYear && mes > agenda[i].startMonth && mes > agenda[i].endMonth && dia > agenda[i].startDay && dia > agenda[i].endDay) {
      apagarAgendamento(i);
    }

  }

}
