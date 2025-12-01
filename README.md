# Foge, Sô!

## Descrição

**Foge, Sô!** é um jogo de ação e sobrevivência no estilo *roguelite / bullet hell*, fortemente inspirado em clássicos do gênero como *Vampire Survivors*. O jogador assume o controle de um personagem que deve resistir a hordas incessantes de inimigos em uma arena aberta. O objetivo central é sobreviver através de ondas temporais, coletando experiência deixada pelos oponentes derrotados para se fortalecer e enfrentar desafios de dificuldade progressiva.

![enemy-1](https://github.com/user-attachments/assets/24fdb2f1-2a2e-4334-8493-a5b205ef78b6)

![enemy-3](https://github.com/user-attachments/assets/5b0a040a-4979-49ca-b0d0-18ccf645b4db)

A mecânica principal foca na movimentação estratégica ("kiting") e no gerenciamento de atributos. O personagem ataca automaticamente os inimigos mais próximos, permitindo que o jogador concentre sua atenção em desviar de perigos, navegar pelo mapa e ganhar experiência e novas habilidades. 
![improvements](https://github.com/user-attachments/assets/547b933b-45d4-4c65-9441-89c11bc366e9)


A estrutura do jogo é dividida em ondas baseadas no tempo. Para vencer, o jogador deve sobreviver até a **Wave 5** e derrotar o Chefão Final (o "Sprayer"), que possui padrões de ataque complexos como tiros em espiral e rastro de bombas. A condição de derrota é simples: se a barra de vida do jogador chegar a zero devido ao contato com inimigos ou projéteis, o jogo termina (*Game Over*).
![boss-1](https://github.com/user-attachments/assets/ae76331a-a16d-4250-95ea-5c7e4985a5af)
![boss-2](https://github.com/user-attachments/assets/8ed3a4a7-f23d-4565-9b8a-7f8680091a09)


## Funcionalidades a serem testadas

Pedimos aos jogadores que observem com atenção os seguintes pontos durante o *playtesting*:

* **Sistema de Ondas e Spawn:** Verificar se os inimigos mudam corretamente conforme o tempo passa (Comuns -> Corredores -> Gordos Explosivos -> Atiradores).
* **Comportamento dos Chefes (IA):**
    * **Wave 3 (Tank):** Testar se ele executa a "Investida" (Dash) e o "Bombardeio" corretamente e se a vida dele baixa ao ser atingido.
    * **Wave 5 (Sprayer):** Testar os ataques de "Sniper", "Espiral" e "Rastro de Bombas".
* **Estabilidade:** Verificar se o jogo continua rodando normalmente após a morte de um chefe (sem travamentos/crashes).
* **Colisão e Dano:** Confirmar se o jogador recebe dano ao encostar nos inimigos e se os projéteis do jogador estão registrando acertos.
* **Sistema de Level Up:** Verificar se a coleta de XP preenche a barra e se o menu de Upgrade (pressionando **'U'**) aplica corretamente os bônus de atributo.
* **Performance:** Observar se há queda brusca de FPS quando há muitos inimigos na tela.

## Créditos

* **Pedro Henrique Meireles de Almeida** – *Programação da Engine, Física, Colisão, Protótipo Inicial, Sistema de Gameplay (XP, Upgrades) e Balanceamento.*
* **Arthur Linhares Madureira** – *Ex: Interface (UI) e Áudio*
* **Victor Yuji Yano** – *Implementação da IA dos Inimigos e Chefes.*
* **Marcelo Lommez Rodrigues de Jesus** – *Ex: HUD e FeedbackVisual*


---
*Desenvolvido em C++ com SDL2.*
