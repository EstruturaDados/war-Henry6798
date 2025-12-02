/*
  WAR Estruturado: Missões Estratégicas
  - Cada jogador recebe uma missão sorteada automaticamente (vetor de strings).
  - Missões armazenadas dinamicamente via malloc.
  - Verificação silenciosa da missão ao final de cada turno.
  - Ataques com rand() (1..6), validação de cor, transferência de tropas e cor.
  - Modularização: atribuirMissao, verificarMissao, exibirMissao, atacar, exibirMapa, liberarMemoria, main.

  Bibliotecas: stdio.h, stdlib.h, string.h, time.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ----------------------------- Constantes ----------------------------- */

#define MAX_NOME_TERR 30
#define MAX_COR        10
#define MAX_MISSAO     64

/* Número de jogadores e territórios (ajustável) */
#define NUM_JOGADORES  2
#define NUM_TERRITORIOS 8

/* ----------------------------- Estruturas ----------------------------- */

typedef struct {
    char nome[MAX_NOME_TERR];
    char cor[MAX_COR];   /* cor do dono atual: "azul", "vermelho", "neutro", etc. */
    int tropas;
} Territorio;

/* ----------------------------- Utilidades ----------------------------- */

/* Lê linha segura com fgets e remove '\n' se existir */
void lerLinha(char *dest, size_t tam) {
    if (fgets(dest, (int)tam, stdin) == NULL) {
        dest[0] = '\0';
        return;
    }
    size_t len = strlen(dest);
    if (len > 0 && dest[len - 1] == '\n') dest[len - 1] = '\0';
}

/* Lê inteiro via fgets + strtol com validação de intervalo */
int lerInteiroIntervalo(int minVal, int maxVal) {
    char buf[64];
    long v;
    char *end;
    while (1) {
        if (fgets(buf, sizeof(buf), stdin) == NULL) continue;
        v = strtol(buf, &end, 10);
        if (end == buf) {
            printf("Entrada inválida. Tente novamente: ");
            continue;
        }
        if (v < minVal || v > maxVal) {
            printf("Valor fora do intervalo (%d..%d). Tente novamente: ", minVal, maxVal);
            continue;
        }
        return (int)v;
    }
}

/* ----------------------------- Missões ----------------------------- */

/*
  Atribui missão sorteada de um vetor de strings para o destino (dinamicamente alocado).
  - destino: ponteiro para buffer alocado via malloc (char*)
  - missoes: vetor de strings com descrições
  - totalMissoes: quantidade de missões no vetor
*/
void atribuirMissao(char *destino, char *missoes[], int totalMissoes) {
    int idx = rand() % totalMissoes;
    strcpy(destino, missoes[idx]);
}

/*
  Exibe a missão (passagem por valor: const char*), mostrado apenas no início.
*/
void exibirMissao(const char *missao, const char *nomeJogador) {
    printf("\nMissão atribuída ao jogador %s:\n", nomeJogador);
    printf("-> %s\n", missao);
}

/*
  Verifica se a missão do jogador foi cumprida.
  - missao: descrição textual
  - mapa: vetor de territórios
  - tamanho: quantidade de territórios
  - corJogador: cor do jogador (ex.: "azul", "vermelho")

  Lógica simples inicial:
  - "Conquistar 3 territórios seguidos": verifica 3 territórios consecutivos do mapa que tenham a cor do jogador.
  - "Eliminar todas as tropas da cor vermelha": nenhum território com cor "vermelho" deve ter tropas > 0.
  - "Dominar 4 territórios": contar territórios do jogador >= 4.
  - "Reduzir tropas inimigas a zero em 2 territórios": encontrar ao menos 2 territórios inimigos com tropas == 0.
  - "Controlar a metade do mapa": territórios do jogador >= tamanho/2 (arredondando para cima).
*/
int verificarMissao(char *missao, Territorio *mapa, int tamanho, const char *corJogador) {
    /* Missão: Conquistar 3 territórios seguidos */
    if (strstr(missao, "Conquistar 3 territorios seguidos") != NULL ||
        strstr(missao, "Conquistar 3 territórios seguidos") != NULL) {
        int seguidos = 0;
        for (int i = 0; i < tamanho; i++) {
            if (strcmp(mapa[i].cor, corJogador) == 0) {
                seguidos++;
                if (seguidos >= 3) return 1;
            } else {
                seguidos = 0;
            }
        }
        return 0;
    }

    /* Missão: Eliminar todas as tropas da cor vermelha */
    if (strstr(missao, "Eliminar todas as tropas da cor vermelha") != NULL) {
        for (int i = 0; i < tamanho; i++) {
            if (strcmp(mapa[i].cor, "vermelho") == 0 && mapa[i].tropas > 0) {
                return 0;
            }
        }
        return 1;
    }

    /* Missão: Dominar 4 territórios */
    if (strstr(missao, "Dominar 4 territorios") != NULL ||
        strstr(missao, "Dominar 4 territórios") != NULL) {
        int conta = 0;
        for (int i = 0; i < tamanho; i++) {
            if (strcmp(mapa[i].cor, corJogador) == 0) conta++;
        }
        return (conta >= 4);
    }

    /* Missão: Reduzir tropas inimigas a zero em 2 territórios */
    if (strstr(missao, "Reduzir tropas inimigas a zero em 2 territorios") != NULL ||
        strstr(missao, "Reduzir tropas inimigas a zero em 2 territórios") != NULL) {
        int z = 0;
        for (int i = 0; i < tamanho; i++) {
            if (strcmp(mapa[i].cor, corJogador) != 0 && mapa[i].tropas == 0) {
                z++;
                if (z >= 2) return 1;
            }
        }
        return 0;
    }

    /* Missão: Controlar a metade do mapa */
    if (strstr(missao, "Controlar a metade do mapa") != NULL) {
        int conta = 0;
        for (int i = 0; i < tamanho; i++) {
            if (strcmp(mapa[i].cor, corJogador) == 0) conta++;
        }
        int metade = (tamanho + 1) / 2;
        return (conta >= metade);
    }

    /* Missão desconhecida: não cumprida por padrão */
    return 0;
}

/* ----------------------------- Mecânica de combate ----------------------------- */

/*
  Função de ataque:
  - atacante e defensor são ponteiros para territórios.
  - Apenas ataques contra territórios de cor diferente são válidos.
  - Rolagem de dados rand() para ambos (1..6).
  - Se atacante vencer: defensor muda de cor para a do atacante e recebe metade das tropas do atacante (arredonda para baixo).
  - Caso contrário: atacante perde 1 tropa (mínimo 0).
*/
void atacar(Territorio *atacante, Territorio *defensor) {
    if (strcmp(atacante->cor, defensor->cor) == 0) {
        printf("Ataque inválido: ambos pertencem à mesma cor (%s).\n", atacante->cor);
        return;
    }
    if (atacante->tropas <= 0) {
        printf("Ataque inválido: território atacante sem tropas.\n");
        return;
    }

    int dadoA = (rand() % 6) + 1;
    int dadoD = (rand() % 6) + 1;
    printf("Dados: atacante=%d, defensor=%d -> ", dadoA, dadoD);

    if (dadoA >= dadoD) {
        /* Vitória do atacante: conquista o defensor */
        int transfer = atacante->tropas / 2; /* metade (floor) */
        if (transfer < 1) transfer = 1;      /* garante presença */
        strcpy(defensor->cor, atacante->cor);
        defensor->tropas = transfer;
        /* Opcionalmente reduz tropas do atacante para simular deslocamento */
        atacante->tropas -= transfer;
        if (atacante->tropas < 0) atacante->tropas = 0;
        printf("Vitória! %s conquistou %s e transferiu %d tropas.\n",
               atacante->nome, defensor->nome, transfer);
    } else {
        /* Derrota do atacante: perde 1 tropa */
        atacante->tropas -= 1;
        if (atacante->tropas < 0) atacante->tropas = 0;
        printf("Derrota! %s perdeu 1 tropa.\n", atacante->nome);
    }
}

/* ----------------------------- Exibição ----------------------------- */

void exibirMapa(Territorio *mapa, int n) {
    printf("\n--- Mapa ---\n");
    for (int i = 0; i < n; i++) {
        printf("%2d) %-20s | Cor: %-9s | Tropas: %d\n",
               i, mapa[i].nome, mapa[i].cor, mapa[i].tropas);
    }
}

/* ----------------------------- Liberação de memória ----------------------------- */

void liberarMemoria(Territorio *mapa, char **missoesJogadores, int numJogadores) {
    if (mapa) free(mapa);
    if (missoesJogadores) {
        for (int i = 0; i < numJogadores; i++) {
            free(missoesJogadores[i]);
        }
        free(missoesJogadores);
    }
}

/* ----------------------------- Inicialização de dados ----------------------------- */

/* Preenche um mapa inicial simples e balanceado */
void inicializarMapa(Territorio *mapa, int n) {
    /* Nomes de exemplo */
    const char *nomes[NUM_TERRITORIOS] = {
        "Planalto Norte", "Vale Rubro", "Costa Azul", "Deserto Sul",
        "Montes Brancos", "Bacia Central", "Fronteira Leste", "Fortim Oeste"
    };
    /* Distribuição inicial de cores: alternando azul/vermelho/neutro */
    const char *cores[NUM_TERRITORIOS] = {
        "azul", "vermelho", "neutro", "azul",
        "vermelho", "neutro", "azul", "vermelho"
    };
    int tropas[NUM_TERRITORIOS] = {4, 3, 2, 5, 3, 1, 4, 2};

    for (int i = 0; i < n; i++) {
        strncpy(mapa[i].nome, nomes[i], MAX_NOME_TERR - 1);
        mapa[i].nome[MAX_NOME_TERR - 1] = '\0';
        strncpy(mapa[i].cor, cores[i], MAX_COR - 1);
        mapa[i].cor[MAX_COR - 1] = '\0';
        mapa[i].tropas = tropas[i];
    }
}

/* ----------------------------- Main (loop do jogo) ----------------------------- */

int main(void) {
    srand((unsigned)time(NULL));

    /* Vetor de missões pré-definidas (descrições) */
    char *missoesPredef[] = {
        "Conquistar 3 territorios seguidos",
        "Eliminar todas as tropas da cor vermelha",
        "Dominar 4 territorios",
        "Reduzir tropas inimigas a zero em 2 territorios",
        "Controlar a metade do mapa"
    };
    int totalMissoes = (int)(sizeof(missoesPredef) / sizeof(missoesPredef[0]));

    /* Aloca mapa dinamicamente */
    Territorio *mapa = (Territorio*)calloc(NUM_TERRITORIOS, sizeof(Territorio));
    if (!mapa) {
        fprintf(stderr, "Erro ao alocar mapa.\n");
        return 1;
    }
    inicializarMapa(mapa, NUM_TERRITORIOS);

    /* Aloca missões para cada jogador (vetor de ponteiros) */
    char **missoesJogadores = (char**)calloc(NUM_JOGADORES, sizeof(char*));
    if (!missoesJogadores) {
        fprintf(stderr, "Erro ao alocar vetor de missoes de jogadores.\n");
        free(mapa);
        return 1;
    }
    for (int i = 0; i < NUM_JOGADORES; i++) {
        missoesJogadores[i] = (char*)malloc(MAX_MISSAO);
        if (!missoesJogadores[i]) {
            fprintf(stderr, "Erro ao alocar missao do jogador %d.\n", i);
            liberarMemoria(mapa, missoesJogadores, NUM_JOGADORES);
            return 1;
        }
    }

    /* Define nomes e cores dos jogadores (ajuste conforme desejar) */
    const char *nomesJogadores[NUM_JOGADORES] = {"Jogador Azul", "Jogador Vermelho"};
    const char *coresJogadores[NUM_JOGADORES] = {"azul", "vermelho"};

    /* Sorteia e atribui missões (mostra apenas uma vez) */
    for (int i = 0; i < NUM_JOGADORES; i++) {
        atribuirMissao(missoesJogadores[i], missoesPredef, totalMissoes);
        exibirMissao(missoesJogadores[i], nomesJogadores[i]);
    }

    /* Loop principal: turnos com ataque e verificação silenciosa de missão */
    int vencedor = -1;
    int turno = 0;

    while (vencedor < 0) {
        turno++;
        printf("\n=== Turno %d ===\n", turno);
        exibirMapa(mapa, NUM_TERRITORIOS);

        /* Escolha do jogador atual alternado */
        int jogadorAtual = (turno - 1) % NUM_JOGADORES;
        const char *corAtual = coresJogadores[jogadorAtual];
        printf("\nVez de %s (%s).\n", nomesJogadores[jogadorAtual], corAtual);

        /* Escolher território atacante (deve ser da cor do jogador) */
        printf("Escolha indice do territorio ATACANTE (0..%d): ", NUM_TERRITORIOS - 1);
        int idxA = lerInteiroIntervalo(0, NUM_TERRITORIOS - 1);
        if (strcmp(mapa[idxA].cor, corAtual) != 0) {
            printf("Você só pode atacar a partir de um território da sua cor (%s).\n", corAtual);
            /* Mesmo com falha, ao fim do turno ainda verificamos missão silenciosamente */
        } else {
            /* Escolher território defensor (não pode ser da mesma cor) */
            printf("Escolha indice do territorio DEFENSOR (0..%d): ", NUM_TERRITORIOS - 1);
            int idxD = lerInteiroIntervalo(0, NUM_TERRITORIOS - 1);
            if (strcmp(mapa[idxD].cor, corAtual) == 0) {
                printf("Você só pode atacar territórios inimigos.\n");
            } else {
                atacar(&mapa[idxA], &mapa[idxD]);
            }
        }

        /* Verificação silenciosa da missão de cada jogador ao fim do turno */
        for (int j = 0; j < NUM_JOGADORES; j++) {
            if (verificarMissao(missoesJogadores[j], mapa, NUM_TERRITORIOS, coresJogadores[j])) {
                vencedor = j;
                break;
            }
        }

        if (vencedor >= 0) {
            printf("\n=== Fim de jogo ===\n");
            printf("Vitória de %s!\n", nomesJogadores[vencedor]);
            printf("Missão cumprida: %s\n", missoesJogadores[vencedor]);
        } else {
            /* Opcional: permitir continuar ou sair */
            printf("\nContinuar? (1=sim, 0=nao): ");
            int continuar = lerInteiroIntervalo(0, 1);
            if (!continuar) {
                printf("Encerrando a partida sem vencedor.\n");
                break;
            }
        }
    }

    liberarMemoria(mapa, missoesJogadores, NUM_JOGADORES);
    return 0;
}
roblemas com leituras consecutivas de scanf e getchar.
