/*
  WAR Estruturado: Organização de unidades militares
  - Cadastro de tropas
  - Ordenação por nome, tipo ou poder
  - Busca binária por nome
  - Medição de desempenho (comparações e tempo)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_UNIDADES 20
#define MAX_NOME 30
#define MAX_TIPO 20

typedef struct {
    char nome[MAX_NOME];
    char tipo[MAX_TIPO];
    int poder;
} Unidade;

/* ---------------- Utilidades ---------------- */
void tirarNovaLinha(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') s[len - 1] = '\0';
}

void lerLinha(char *dest, size_t tam) {
    fgets(dest, (int)tam, stdin);
    tirarNovaLinha(dest);
}

int lerInteiroLinha(int minVal, int maxVal) {
    char buf[64];
    long valor;
    char *endptr;
    while (1) {
        fgets(buf, sizeof(buf), stdin);
        valor = strtol(buf, &endptr, 10);
        if (endptr == buf) { printf("Número inválido. Tente: "); continue; }
        if (valor < minVal || valor > maxVal) { 
            printf("Valor fora do intervalo (%d-%d). Tente: ", minVal, maxVal); 
            continue; 
        }
        return (int)valor;
    }
}

void trocar(Unidade *a, Unidade *b) {
    Unidade tmp = *a;
    *a = *b;
    *b = tmp;
}

/* ---------------- Exibição ---------------- */
void mostrarUnidades(Unidade v[], int n) {
    printf("\n--- Formação Militar (%d unidades) ---\n", n);
    for (int i = 0; i < n; i++) {
        printf("%2d) Nome: %-25s | Tipo: %-15s | Poder: %d\n",
               i+1, v[i].nome, v[i].tipo, v[i].poder);
    }
    printf("--------------------------------------\n");
}

/* ---------------- Ordenações ---------------- */
void bubbleSortNome(Unidade v[], int n, long *comparacoes) {
    *comparacoes = 0;
    for (int i = 0; i < n-1; i++) {
        for (int j = 0; j < n-1-i; j++) {
            (*comparacoes)++;
            if (strcmp(v[j].nome, v[j+1].nome) > 0) {
                trocar(&v[j], &v[j+1]);
            }
        }
    }
}

void insertionSortTipo(Unidade v[], int n, long *comparacoes) {
    *comparacoes = 0;
    for (int i = 1; i < n; i++) {
        Unidade chave = v[i];
        int j = i-1;
        while (j >= 0) {
            (*comparacoes)++;
            if (strcmp(v[j].tipo, chave.tipo) > 0) {
                v[j+1] = v[j];
                j--;
            } else break;
        }
        v[j+1] = chave;
    }
}

void selectionSortPoder(Unidade v[], int n, long *comparacoes) {
    *comparacoes = 0;
    for (int i = 0; i < n-1; i++) {
        int minIdx = i;
        for (int j = i+1; j < n; j++) {
            (*comparacoes)++;
            if (v[j].poder < v[minIdx].poder) minIdx = j;
        }
        if (minIdx != i) trocar(&v[i], &v[minIdx]);
    }
}

/* ---------------- Busca Binária ---------------- */
int buscaBinariaPorNome(Unidade v[], int n, const char chave[], long *comparacoes) {
    int esq = 0, dir = n-1;
    *comparacoes = 0;
    while (esq <= dir) {
        int meio = (esq + dir)/2;
        int cmp = strcmp(v[meio].nome, chave);
        (*comparacoes)++;
        if (cmp == 0) return meio;
        else if (cmp < 0) esq = meio+1;
        else dir = meio-1;
    }
    return -1;
}

/* ---------------- Medição ---------------- */
double medirTempo(void (*alg)(Unidade[], int, long*), Unidade v[], int n, long *comparacoes) {
    clock_t ini = clock();
    alg(v, n, comparacoes);
    clock_t fim = clock();
    return (double)(fim-ini)/CLOCKS_PER_SEC;
}

/* ---------------- Cadastro ---------------- */
void cadastrarUnidade(Unidade *u) {
    printf("Nome da unidade: ");
    lerLinha(u->nome, sizeof(u->nome));
    printf("Tipo da unidade (infantaria/artilharia/aereo): ");
    lerLinha(u->tipo, sizeof(u->tipo));
    printf("Poder de combate (1-10): ");
    u->poder = lerInteiroLinha(1,10);
}

/* ---------------- Menu ---------------- */
int main() {
    Unidade exercito[MAX_UNIDADES];
    int n = 0;
    int ordenadoPorNome = 0;
    char chaveBusca[MAX_NOME];
    int indiceChave = -1;

    int opcao;
    do {
        printf("\n=== WAR Estruturado ===\n");
        printf("1) Cadastrar unidade\n");
        printf("2) Listar unidades\n");
        printf("3) Ordenar por NOME (Bubble Sort)\n");
        printf("4) Ordenar por TIPO (Insertion Sort)\n");
        printf("5) Ordenar por PODER (Selection Sort)\n");
        printf("6) Buscar unidade por NOME (após ordenação por nome)\n");
        printf("7) Formação final\n");
        printf("0) Sair\n");
        printf("Escolha: ");
        opcao = lerInteiroLinha(0,7);

        switch(opcao) {
            case 1:
                if (n >= MAX_UNIDADES) { printf("Limite atingido.\n"); break; }
                cadastrarUnidade(&exercito[n]);
                n++; ordenadoPorNome=0; indiceChave=-1;
                break;
            case 2:
                mostrarUnidades(exercito,n);
                break;
            case 3: {
                long comps; double t = medirTempo(bubbleSortNome, exercito,n,&comps);
                printf("Bubble Sort (Nome): %ld comparações, %.6f s\n", comps,t);
                mostrarUnidades(exercito,n);
                ordenadoPorNome=1; indiceChave=-1;
                break;
            }
            case 4: {
                long comps; double t = medirTempo(insertionSortTipo, exercito,n,&comps);
                printf("Insertion Sort (Tipo): %ld comparações, %.6f s\n", comps,t);
                mostrarUnidades(exercito,n);
                ordenadoPorNome=0; indiceChave=-1;
                break;
            }
            case 5: {
                long comps; double t = medirTempo(selectionSortPoder, exercito,n,&comps);
                printf("Selection Sort (Poder): %ld comparações, %.6f s\n", comps,t);
                mostrarUnidades(exercito,n);
                ordenadoPorNome=0; indiceChave=-1;
                break;
            }
            case 6: {
                if (!ordenadoPorNome) { printf("Ordene por nome primeiro.\n"); break; }
                printf("Nome da unidade a buscar: ");
                lerLinha(chaveBusca,sizeof(chaveBusca));
                long comps; int idx = buscaBinariaPorNome(exercito,n,chaveBusca,&comps);
                if (idx>=0) {
                    printf("Unidade encontrada (%ld comparações): %s | Tipo: %s | Poder: %d\n",
                           comps, exercito[idx].nome, exercito[idx].tipo, exercito[idx].poder);
                    indiceChave=idx;
                } else {
                    printf("Unidade não encontrada (%ld comparações).\n", comps);
                }
                break;
            }
            case 
                trada do teclado (stdin), evitando problemas com leituras consecutivas de scanf e getchar.
