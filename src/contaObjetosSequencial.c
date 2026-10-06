#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    int linha;
    int coluna;
} Celula;

static int indice(int linha, int coluna, int colunas)
{
    return linha * colunas + coluna;
}

static void flood_fill(const int *matriz, unsigned char *visitado,
                       int linhas, int colunas,
                       int linha_inicial, int coluna_inicial)
{
    Celula *pilha;
    int topo;
    int capacidade;
    int dl;
    int dc;

    capacidade = linhas * colunas;
    pilha = (Celula *) malloc((size_t) capacidade * sizeof(Celula));

    if (pilha == NULL) {
        fprintf(stderr, "Erro ao alocar memoria.\n");
        exit(EXIT_FAILURE);
    }

    topo = 0;

    pilha[topo].linha = linha_inicial;
    pilha[topo].coluna = coluna_inicial;
    topo++;

    visitado[indice(linha_inicial, coluna_inicial, colunas)] = 1;

    while (topo > 0) {
        Celula atual;

        topo--;
        atual = pilha[topo];

        for (dl = -1; dl <= 1; dl++) {
            for (dc = -1; dc <= 1; dc++) {
                int nl;
                int nc;
                int pos;

                if (dl == 0 && dc == 0)
                    continue;

                nl = atual.linha + dl;
                nc = atual.coluna + dc;

                if (nl < 0 || nl >= linhas ||
                    nc < 0 || nc >= colunas)
                    continue;

                pos = indice(nl, nc, colunas);

                if (matriz[pos] == 1 && !visitado[pos]) {
                    visitado[pos] = 1;

                    pilha[topo].linha = nl;
                    pilha[topo].coluna = nc;
                    topo++;
                }
            }
        }
    }

    free(pilha);
}

static int contar_objetos(const int *matriz, int linhas, int colunas)
{
    unsigned char *visitado;
    int quantidade;
    int l;
    int c;

    visitado = (unsigned char *) calloc(
        (size_t) linhas * colunas,
        sizeof(unsigned char)
    );

    if (visitado == NULL) {
        fprintf(stderr, "Erro ao alocar memoria.\n");
        exit(EXIT_FAILURE);
    }

    quantidade = 0;

    for (l = 0; l < linhas; l++) {
        for (c = 0; c < colunas; c++) {
            int pos;

            pos = indice(l, c, colunas);

            if (matriz[pos] == 1 && !visitado[pos]) {
                quantidade++;

                flood_fill(
                    matriz,
                    visitado,
                    linhas,
                    colunas,
                    l,
                    c
                );
            }
        }
    }

    free(visitado);

    return quantidade;
}

int main(void)
{
    int linhas;
    int colunas;
    int *matriz;
    int i;
    int objetos;
    struct timespec inicio;
    struct timespec fim;
    double tempo;

    if (scanf("%d %d", &linhas, &colunas) != 2) {
        fprintf(stderr, "Entrada invalida.\n");
        return EXIT_FAILURE;
    }

    if (linhas <= 0 || colunas <= 0) {
        fprintf(stderr, "Dimensoes invalidas.\n");
        return EXIT_FAILURE;
    }

    matriz = (int *) malloc(
        (size_t) linhas * colunas * sizeof(int)
    );

    if (matriz == NULL) {
        fprintf(stderr, "Erro ao alocar matriz.\n");
        return EXIT_FAILURE;
    }

    for (i = 0; i < linhas * colunas; i++) {
        if (scanf("%d", &matriz[i]) != 1) {
            fprintf(stderr, "Erro ao ler matriz.\n");
            free(matriz);
            return EXIT_FAILURE;
        }
    }

    /* Mede tempo decorrido, excluindo a leitura da entrada. */
    if (clock_gettime(CLOCK_MONOTONIC, &inicio) != 0) {
        perror("Erro ao iniciar medicao de tempo");
        free(matriz);
        return EXIT_FAILURE;
    }

    objetos = contar_objetos(matriz, linhas, colunas);

    if (clock_gettime(CLOCK_MONOTONIC, &fim) != 0) {
        perror("Erro ao finalizar medicao de tempo");
        free(matriz);
        return EXIT_FAILURE;
    }

    tempo = (double) (fim.tv_sec - inicio.tv_sec)
          + (double) (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;

    printf("Objetos: %d\n", objetos);
    printf("Tempo: %.6f segundos\n", tempo);

    free(matriz);

    return EXIT_SUCCESS;
}