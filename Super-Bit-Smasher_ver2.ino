// C++ code
//
// --- Super-Bit-Smasher Game ---
//
// Concebido Por: Carlos Oliveira


// Declaração das variáveis globais a serem utilizadas ao longo do código:
unsigned long lastDebounceTime[3] = {0,0,0}; // Array com o tempo da última mudança de estado de cada botão ({Botão_Pino_2, Botão_Pino_3, Botão_Pino_4}).
byte lastButtonState[3] = {HIGH,HIGH,HIGH}; // Array com o último estado de cada botão ({Botão_Pino_2, Botão_Pino_3, Botão_Pino_4}).
byte buttonState[3] = {HIGH,HIGH,HIGH}; // Array com o estado atual de cada botão ({Botão_Pino_2, Botão_Pino_3, Botão_Pino_4}).
const unsigned long timeLimit = 60000; // Tempo limite para cada rodada do jogo (60 segundos).
const unsigned int TEMPO_RESET = 2000; // Tempo para as longas pressões no botão OR (2 segundos).
unsigned long ultimo_ciclo = 0; // Variável para guardar o tempo da última mudança de estado do botão OR para o debounce do reset.
bool jogo_ativo = true; // variável para controlar o estado do jogo.
byte target; // Variável para guardar o valor target.
byte ponto_de_partida; // Variável para guardar o valor do ponto de partida.
byte anti_ciclo[2] = {0,0}; // Array para fazer com que os prints que estejam dentro de um ciclo sejam impressos apenas uma vez.
byte last_input_bin; // Variável para guardar o último input do utilizador em binário.
bool AND_HABILITADO; // Variável para verificar se a operação AND está habilitada, se não estiver é porque a operação XOR está habilitada.
unsigned long tempo_inicio_jogo; // Variável para guardar o tempo de início do jogo.


void setup() {
    Serial.begin(9600); // Inicializa a comunicação Serial com um rate de 9600 bps.
    randomSeed(analogRead(0)); // Inicializa o gerador de números aleatórios com um valor aleatório.
    for (byte PIN_LED = 8; PIN_LED <= 11; PIN_LED++) pinMode(PIN_LED, OUTPUT); // Inicializa os pinos dos LEDs como OUTPUT.
    for (byte PIN_BOTAO = 2; PIN_BOTAO <= 4; PIN_BOTAO++) pinMode(PIN_BOTAO, INPUT_PULLUP); // Inicializa os pinos dos botões como INPUT com resistência de pull-up.
}

// Função para fazer debounce dos botões:
bool debounceButton(const byte PINO, const unsigned long debounceDelay = 50) {
    byte reading = digitalRead(PINO);
    const byte index = PINO - 2; // Indexa o array de estados dos botões corretamente.

    if (reading != lastButtonState[index]) lastDebounceTime[index] = millis();

    if ((millis() - lastDebounceTime[index]) >= debounceDelay) {
        if (reading != buttonState[index]) {
            buttonState[index] = reading;
            if (buttonState[index] == LOW) {
                lastButtonState[index] = reading; // Atualiza o último estado do botão.
                return true;
            }
        }
    }

    lastButtonState[index] = reading; // Atualiza o último estado do botão.
    return false;
}

// Função para fazer o reset do jogo:
bool reset_jogo() {
    // Bloco de código tipo debounce para assegurar o reset do jogo quando o botão do OR é pressionado por mais de 2 segundos:
    unsigned long novo_ciclo = millis();
    byte leitura_reset = digitalRead(3);
    if (leitura_reset == LOW) {
        if (novo_ciclo - ultimo_ciclo > TEMPO_RESET) {
            Serial.println("");
            Serial.println("--- Jogo Resetado! ---");
            delay(1000); // Delay intencional para dar um intervalo entre rodadas do jogo após o reset.
            return true;
        }
    } else ultimo_ciclo = novo_ciclo;
    return false;
}

// Função para fazer a contagem do tempo limite do jogo e ativar os LEDs de tempo restante:
bool timer() {
    // Condição para verificar se o tempo limite do jogo foi atingido, se foi o jogo é terminado:
    if (millis() - tempo_inicio_jogo > timeLimit) {
        Serial.println("");
        Serial.println("--- Tempo esgotado! Tente novamente. ---");
        delay(500); // Delay intencional para dar um intervalo entre rodadas do jogo após o tempo esgotado.
        jogo_ativo = false;
        return true;
    }

    // Condição para ativar os LEDs de tempo restante:
    for (byte quarto = 1; quarto <= 4; quarto++){
        if (millis() - tempo_inicio_jogo == quarto*(timeLimit/4)) {
        digitalWrite(7 + quarto, HIGH);
        }
    }

    return false;
}

// Função principal do jogo:
void main_jogo() {
    tempo_inicio_jogo = millis(); // Inicializa o tempo de início do jogo.
    while(true) {
        // Chama a função reset_jogo() para verificar se o botão OR foi pressionado por mais de 2 segundos:
        if (reset_jogo()) return;

        // Chama a função timer() para verificar se o tempo limite do jogo foi atingido:
        if (timer()) return;

        // Condição para verificar a vitória:
        if (ponto_de_partida == target) {
            Serial.println("Parabéns! Acertou no alvo!");
            jogo_ativo = false;
            return;
        }

        // Condição para fazer um print a pedir um novo input ao utilizador de uma maneira mais formatada e de fácil compreensão e a indicar a operação selecionada:
        anti_ciclo[1]++;
        if (anti_ciclo[1] == 1) {
            Serial.println(""); // Print vazio para dar um espaço entre mensagens de texto.
            Serial.print("Introduza um valor: ");
        }
        if (anti_ciclo[1] == 5) anti_ciclo[1] = 3;

        // Condição para verificar se há um input disponível (se ouverem mais do que 0 bytes disponíveis, executa):
        if (Serial.available() > 0) {
            // Reset do valor do array anti_ciclo associado ao print a pedir um novo input, para que cada vez que após cada operação, o programa peça um novo input:
            anti_ciclo[1] = 0;

            String INPUT_STR = Serial.readStringUntil('\n'); // Lê o input do utilizador até que seja pressionada a tecla Enter.
            int input_int = INPUT_STR.toInt(); // Converte o input do utilizador para um valor inteiro.

            // Verifica se o valor de input é válido, se não for, o programa pede um novo input:
            if ( 0 > input_int || input_int > 255) {
                Serial.println("ERRO: Valor de input invalido!");
                continue;
            }

            // Conversor de número inteiro para binário (em formato de Byte):
            String input_strbin = String(input_int, BIN); // Converte o valor inteiro para a forma de binário MAS em formato de string.
            byte input_bin=input_strbin.toInt(); // Converte o valor binário de string para um valor binário.

            // Condição para fazer print do valor de input e do valor de input em binário:
            Serial.print(input_int);
            Serial.print("; BIN: ");
            Serial.println(input_strbin);

            // Condição para fazer um único print a pedir a seleção de um operador, no início de cada rodada:
            anti_ciclo[0]++;
            if (anti_ciclo[0] == 1) Serial.println("--- Prima um Operador ---");
            if (anti_ciclo[0] == 5) anti_ciclo[0] = 3; // Para evitar overflow e reduzir o espaço de memória ocupado pelo array anti_ciclo.

            if (reset_jogo()) return;

            if (timer()) return;

            // Loop criado para esperar por um input do utilizador:
            while (buttonState[0]==lastButtonState[0] or buttonState[1]==lastButtonState[1] or buttonState[2]==lastButtonState[2]) {
                // Operações Bitwise que incrementam ao valor do ponto de partida, o novo input do utilizador em binário:
                if (debounceButton(2) && !AND_HABILITADO) {
                    anti_ciclo[0]=0;
                    Serial.println("Operacao Selecionada: XOR -Amarelo");
                    Serial.println("Operacoes permitidas: OR - Branco");
                    ponto_de_partida ^= input_bin;  // Operação: XOR
                    Serial.print("|     Valor calculado: ");
                    Serial.println(ponto_de_partida, BIN);
                    Serial.print("|     Valor Target: ");
                    Serial.println(target, BIN);
                    Serial.println("----------------------------------------");
                    for (byte index = 0; index <= 3; index++) lastButtonState[index] = HIGH;
                    break;
                } else if (debounceButton(4) && AND_HABILITADO) {
                    anti_ciclo[0]=0;
                    Serial.println("Operacao Selecionada: AND - Vermelho");
                    Serial.println("Operacoes permitidas: OR - Branco");
                    ponto_de_partida &= input_bin; // Operação: AND
                    Serial.print("|     Valor calculado: ");
                    Serial.println(ponto_de_partida, BIN);
                    Serial.print("|     Valor Target: ");
                    Serial.println(target, BIN);
                    Serial.println("----------------------------------------");
                    for (byte index = 0; index <= 3; index++) lastButtonState[index] = HIGH;
                    break;
                } else if (debounceButton(3)) {
                    anti_ciclo[0]=0;
                    Serial.println("Operacao Selecionada: OR - Branco");
                    if (AND_HABILITADO) Serial.println("Operacoes permitidas: AND - Vermelho");
                    if (!AND_HABILITADO) Serial.println("Operacoes permitidas: XOR - Amarelo");
                    ponto_de_partida |= input_bin; // Operação: OR
                    Serial.print("|     Valor calculado: ");
                    Serial.println(ponto_de_partida, BIN);
                    Serial.print("|     Valor Target: ");
                    Serial.println(target, BIN);
                    Serial.println("----------------------------------------");
                    for (byte index = 0; index <= 3; index++) lastButtonState[index] = HIGH;
                    break;
                } else if (debounceButton(4) && !AND_HABILITADO) {
                    Serial.println("ERRO: Operacao Invalida! Selecione novamente.");
                    continue;
                } else if (debounceButton(2) && AND_HABILITADO) {
                    Serial.println("ERRO: Operacao Invalida! Selecione novamente.");
                    continue;
                }

                // Fazer reset e continuar a contabilizar o tempo mesmo dentro do loop de espera de input:
                if (reset_jogo()) return;

                if (timer()) return;
            
            }
        }
    }
}


void loop() {
    Serial.println("");
    Serial.println("**** NOVA RODADA ****");

    // Geradores de números aleatórios para o alvo e ponto de partida:
    target = random(0b10000000, 0b11111111);
    ponto_de_partida = random(0b10000000, 0b11111111);

    Serial.print("Valor Base: ");
    Serial.println(ponto_de_partida, BIN);
    Serial.print("Valor Target: ");
    Serial.println(target, BIN);

    // Máscaras para verificar se as operações AND e XOR estão habilitadas e mensagens de texto correspondentes:
    if (target & (1 << 0)) AND_HABILITADO = true;
    
    if (AND_HABILITADO) {
        Serial.println("Operacoes permitidas: AND - Vermelho | OR - Branco");
    } else if (!AND_HABILITADO) {
        Serial.println("Operacoes permitidas: OR - Branco | XOR - Amarelo");
    }

    // Início do jogo:
    main_jogo();

    // Condição para terminar o jogo caso o tempo tenha expirado ou caso o utilizador tenha ganho o jogo:
    if (jogo_ativo==false) return;

    // Redefenição de algumas variáveis globais utilizadas ao longo de cada rodada do jogo e do estado dos LEDs: 
    for (byte LED = 8; LED <= 11; LED++) pinMode(LED, LOW);
    last_input_bin = 0b00000000; // Variável para guardar o último input do utilizador em binário.
}