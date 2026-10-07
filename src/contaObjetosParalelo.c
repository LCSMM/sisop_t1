#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    int linha;
    int coluna;
} Celula;

typedef struct {
    int id;
    int inicio;
    int fim;
    int inicio_coluna;
    int fim_coluna;
    int linhas;
    int colunas;
    const int *matriz;
    int *rotulos;
    int proximo_rotulo;
} DadosThread;

static int indice(int linha, int coluna, int colunas)
{
    return linha * colunas + coluna;
}

static void flood_fill_local(DadosThread *dados,
                             int linha_inicial,
                             int coluna_inicial,
                             int rotulo)
{
    Celula *pilha;
    int capacidade;
    int topo;
    int dl;
    int dc;

    capacidade = (dados->fim - dados->inicio)
               * (dados->fim_coluna - dados->inicio_coluna);

    pilha = (Celula *) malloc(
        (size_t) capacidade * sizeof(Celula)
    );

    if (pilha == NULL) {
        fprintf(stderr, "Erro ao alocar memoria.\n");
        exit(EXIT_FAILURE);
    }

    topo = 0;

    pilha[topo].linha = linha_inicial;
    pilha[topo].coluna = coluna_inicial;
    topo++;

    dados->rotulos[
        indice(linha_inicial, coluna_inicial, dados->colunas)
    ] = rotulo;

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

                /*
                 * A thread somente percorre sua propria
                 * regiao de linhas e colunas.
                 */
                if (nl < dados->inicio ||
                    nl >= dados->fim ||
                    nc < dados->inicio_coluna ||
                    nc >= dados->fim_coluna)
                    continue;

                pos = indice(nl, nc, dados->colunas);

                if (dados->matriz[pos] == 1 &&
                    dados->rotulos[pos] == 0) {

                    dados->rotulos[pos] = rotulo;

                    pilha[topo].linha = nl;
                    pilha[topo].coluna = nc;
                    topo++;
                }
            }
        }
    }

    free(pilha);
}

static void *processar_faixa(void *arg)
{
    DadosThread *dados;
    int l;
    int c;

    dados = (DadosThread *) arg;

    for (l = dados->inicio; l < dados->fim; l++) {
        for (c = dados->inicio_coluna; c < dados->fim_coluna; c++) {
            int pos;

            pos = indice(l, c, dados->colunas);

            if (dados->matriz[pos] == 1 &&
                dados->rotulos[pos] == 0) {

                flood_fill_local(
                    dados,
                    l,
                    c,
                    dados->proximo_rotulo
                );

                dados->proximo_rotulo++;
            }
        }
    }

    return NULL;
}

/* ---------- Union-Find ---------- */

static int encontrar(int *pai, int x)
{
    while (pai[x] != x) {
        pai[x] = pai[pai[x]];
        x = pai[x];
    }

    return x;
}

static void unir(int *pai, int a, int b)
{
    int raiz_a;
    int raiz_b;

    raiz_a = encontrar(pai, a);
    raiz_b = encontrar(pai, b);

    if (raiz_a != raiz_b)
        pai[raiz_b] = raiz_a;
}

static int contar_resultado(int *rotulos,
                            int linhas,
                            int colunas,
                            int *pai,
                            int total_rotulos)
{
    unsigned char *usado;
    int quantidade;
    int i;

    usado = (unsigned char *) calloc(
        (size_t) total_rotulos,
        sizeof(unsigned char)
    );

    if (usado == NULL) {
        fprintf(stderr, "Erro ao alocar memoria.\n");
        exit(EXIT_FAILURE);
    }

    quantidade = 0;

    for (i = 0; i < linhas * colunas; i++) {
        if (rotulos[i] != 0) {
            int raiz;

            raiz = encontrar(pai, rotulos[i]);

            if (!usado[raiz]) {
                usado[raiz] = 1;
                quantidade++;
            }
        }
    }

    free(usado);

    return quantidade;
}

static void aguardar_threads(pthread_t *threads, int quantidade)
{
    int t;

    for (t = 0; t < quantidade; t++) {
        if (pthread_join(threads[t], NULL) != 0) {
            fprintf(stderr, "Erro ao aguardar thread %d.\n", t);
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[])
{
    int linhas;
    int colunas;
    int numero_threads;
    int *matriz;
    int *rotulos;
    pthread_t *threads;
    DadosThread *dados;
    int *pai;
    int i;
    int t;
    int linhas_base;
    int unidades;
    int resto;
    int atual;
    int total_rotulos;
    int objetos;
    struct timespec inicio;
    struct timespec fim;
    double tempo;

    if (argc != 2) {
        fprintf(stderr,
                "Uso: %s <numero_de_threads>\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    numero_threads = atoi(argv[1]);

    if (numero_threads < 2) {
        fprintf(stderr,
                "Use pelo menos 2 threads.\n");
        return EXIT_FAILURE;
    }

    if (scanf("%d %d", &linhas, &colunas) != 2) {
        fprintf(stderr, "Entrada invalida.\n");
        return EXIT_FAILURE;
    }

    if (linhas <= 0 || colunas <= 0) {
        fprintf(stderr, "Dimensoes invalidas.\n");
        return EXIT_FAILURE;
    }

    unidades = (linhas == 1) ? colunas : linhas;
    if (unidades < 2) {
        fprintf(stderr,
                "A versao paralela requer pelo menos duas celulas.\n");
        return EXIT_FAILURE;
    }
    if (numero_threads > unidades)
        numero_threads = unidades;

    matriz = (int *) malloc(
        (size_t) linhas * colunas * sizeof(int)
    );

    rotulos = (int *) calloc(
        (size_t) linhas * colunas,
        sizeof(int)
    );

    threads = (pthread_t *) malloc(
        (size_t) numero_threads * sizeof(pthread_t)
    );

    dados = (DadosThread *) malloc(
        (size_t) numero_threads * sizeof(DadosThread)
    );

    if (matriz == NULL ||
        rotulos == NULL ||
        threads == NULL ||
        dados == NULL) {

        fprintf(stderr, "Erro ao alocar memoria.\n");

        free(matriz);
        free(rotulos);
        free(threads);
        free(dados);

        return EXIT_FAILURE;
    }

    for (i = 0; i < linhas * colunas; i++) {
        if (scanf("%d", &matriz[i]) != 1) {
            fprintf(stderr, "Erro ao ler matriz.\n");

            free(matriz);
            free(rotulos);
            free(threads);
            free(dados);

            return EXIT_FAILURE;
        }
    }

    if (clock_gettime(CLOCK_MONOTONIC, &inicio) != 0) {
        perror("Erro ao iniciar medicao de tempo");
        free(matriz);
        free(rotulos);
        free(threads);
        free(dados);
        return EXIT_FAILURE;
    }

    linhas_base = unidades / numero_threads;
    resto = unidades % numero_threads;
    atual = 0;

    /*
     * Cada thread recebe uma faixa exclusiva de linhas ou,
     * para uma unica linha, de colunas.
     *
     * Os rotulos tambem possuem intervalos exclusivos.
     * O valor (t * linhas * colunas + 1) evita que duas
     * threads criem o mesmo identificador.
     */
    for (t = 0; t < numero_threads; t++) {
        int quantidade;

        quantidade = linhas_base;

        if (t < resto)
            quantidade++;

        dados[t].id = t;
        dados[t].inicio = (linhas == 1) ? 0 : atual;
        dados[t].fim = (linhas == 1) ? 1 : atual + quantidade;
        dados[t].inicio_coluna = (linhas == 1) ? atual : 0;
        dados[t].fim_coluna = (linhas == 1) ? atual + quantidade : colunas;
        dados[t].linhas = linhas;
        dados[t].colunas = colunas;
        dados[t].matriz = matriz;
        dados[t].rotulos = rotulos;
        dados[t].proximo_rotulo =
            t * linhas * colunas + 1;

        atual += quantidade;

        if (pthread_create(
                &threads[t],
                NULL,
                processar_faixa,
                &dados[t]) != 0) {

            fprintf(stderr,
                    "Erro ao criar thread %d.\n",
                    t);

            aguardar_threads(threads, t);
            free(matriz);
            free(rotulos);
            free(threads);
            free(dados);

            return EXIT_FAILURE;
        }
    }

    aguardar_threads(threads, numero_threads);

    /*
     * O maior rotulo possivel fica abaixo desse valor.
     */
    total_rotulos =
        numero_threads * linhas * colunas + 1;

    pai = (int *) malloc(
        (size_t) total_rotulos * sizeof(int)
    );

    if (pai == NULL) {
        fprintf(stderr, "Erro ao alocar Union-Find.\n");

        free(matriz);
        free(rotulos);
        free(threads);
        free(dados);

        return EXIT_FAILURE;
    }

    for (i = 0; i < total_rotulos; i++)
        pai[i] = i;

    /*
     * Consolida as fronteiras entre faixas.
     *
     * Como a conectividade e 8, para uma celula da
     * ultima linha de uma faixa precisamos verificar:
     *
     * inferior esquerda
     * inferior
     * inferior direita
     */
    for (t = 0; t < numero_threads - 1; t++) {
        int linha_superior;
        int linha_inferior;
        int c;

        if (linhas == 1) {
            int esquerda;
            int direita;

            esquerda = dados[t].fim_coluna - 1;
            direita = dados[t].fim_coluna;
            if (matriz[esquerda] == 1 && matriz[direita] == 1)
                unir(pai, rotulos[esquerda], rotulos[direita]);
            continue;
        }

        linha_superior = dados[t].fim - 1;
        linha_inferior = dados[t].fim;

        for (c = 0; c < colunas; c++) {
            int pos_superior;
            int pos_inferior;

            pos_superior =
                indice(linha_superior, c, colunas);

            if (matriz[pos_superior] == 0)
                continue;

            /* Vertical */
            pos_inferior =
                indice(linha_inferior, c, colunas);

            if (matriz[pos_inferior] == 1) {
                unir(
                    pai,
                    rotulos[pos_superior],
                    rotulos[pos_inferior]
                );
            }

            /* Diagonal esquerda */
            if (c > 0) {
                pos_inferior =
                    indice(
                        linha_inferior,
                        c - 1,
                        colunas
                    );

                if (matriz[pos_inferior] == 1) {
                    unir(
                        pai,
                        rotulos[pos_superior],
                        rotulos[pos_inferior]
                    );
                }
            }

            /* Diagonal direita */
            if (c + 1 < colunas) {
                pos_inferior =
                    indice(
                        linha_inferior,
                        c + 1,
                        colunas
                    );

                if (matriz[pos_inferior] == 1) {
                    unir(
                        pai,
                        rotulos[pos_superior],
                        rotulos[pos_inferior]
                    );
                }
            }
        }
    }

    objetos = contar_resultado(
        rotulos,
        linhas,
        colunas,
        pai,
        total_rotulos
    );

    if (clock_gettime(CLOCK_MONOTONIC, &fim) != 0) {
        perror("Erro ao finalizar medicao de tempo");
        free(pai);
        free(matriz);
        free(rotulos);
        free(threads);
        free(dados);
        return EXIT_FAILURE;
    }

    tempo = (double) (fim.tv_sec - inicio.tv_sec)
          + (double) (fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;

    printf("Objetos: %d\n", objetos);
    printf("Threads: %d\n", numero_threads);
    printf("Tempo: %.6f segundos\n", tempo);

    free(pai);
    free(dados);
    free(threads);
    free(rotulos);
    free(matriz);

    return EXIT_SUCCESS;
}