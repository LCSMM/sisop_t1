# Contagem de objetos em matriz binária

Trabalho de Sistemas Operacionais — 2026/II, PUCRS.

O projeto implementa duas versões em ANSI C (C89/C90) para contar objetos em uma matriz binária: uma sequencial e uma paralela com threads POSIX (Pthreads). Um objeto é um conjunto de células de valor 1 conectadas horizontalmente, verticalmente ou diagonalmente (conectividade 8). Células de valor 0 representam o fundo.

## Links do trabalho

- [Vídeo de apresentação](https://youtu.be/SAzNxhFOLBE)
- [Repositório no GitHub](https://github.com/LCSMM/sisop_t1)

## Autores

- Gabriella Luisa Schmidt
- Lucas Merlini Marchese

## Estrutura

```text
Makefile
README.md
src/
    contaObjetosSequencial.c
    contaObjetosParalelo.c
tests/
    exemplo1.txt ... exemplo5.txt
    grande.txt
results/
    corretude.md
    corretude.txt
    desempenho.md
    desempenho.txt
```

Os arquivos `.bak` em `src/` são cópias locais anteriores à correção do cronômetro; não são necessários à compilação. Os executáveis `sequencial` e `paralelo` são gerados pelo compilador e devem ser recompilados no ambiente de execução.

## Requisitos e compilação

É necessário um ambiente Linux ou macOS com compilador C, suporte a Pthreads e `make`. No Windows, o projeto foi compilado e executado no Ubuntu pelo WSL. A medição requer suporte a `clock_gettime` e `CLOCK_MONOTONIC`.

No Ubuntu, as ferramentas podem ser instaladas com:

```bash
sudo apt update
sudo apt install build-essential
```

Na pasta que contém o Makefile:

```bash
make -B
```

O argumento `-B` força a recompilação. O Makefile atual não declara os fontes como dependências dos executáveis, portanto esse comando deve ser usado depois de alterar os arquivos C.

A compilação usa `-std=c89 -Wall -Wextra -pedantic`, acrescentando `-pthread` na versão paralela. Para remover apenas os executáveis gerados:

```bash
make clean
```

## Formato da entrada

A primeira linha informa a quantidade de linhas e colunas. Em seguida vêm os valores da matriz, separados por espaços ou quebras de linha. Os valores devem ser 0 ou 1.

Exemplo de matriz 2 × 3 com um objeto conectado pela diagonal:

```text
2 3
1 0 0
0 1 0
```

Os programas leem a entrada padrão. O operador `<` fornece o conteúdo de um arquivo como entrada; a matriz não precisa estar escrita dentro do código C.

## Execução

```bash
./sequencial < tests/exemplo1.txt
./paralelo 2 < tests/exemplo1.txt
./paralelo 4 < tests/exemplo1.txt
```

O argumento da versão paralela informa a quantidade solicitada de threads, que deve ser pelo menos 2. Para matrizes com duas ou mais linhas, a quantidade efetiva é limitada pelo número de linhas. Matrizes de uma linha são divididas em faixas de colunas, com a quantidade limitada pelo número de colunas. Uma matriz 1 × 1 é rejeitada pela versão paralela, pois não permite distribuir células entre duas trabalhadoras; pode ser processada pela versão sequencial.

A saída informa a quantidade de objetos e o tempo medido em segundos. A versão paralela também informa a quantidade efetiva de threads.

## Implementação sequencial

A matriz é armazenada em um vetor linear. Uma varredura procura células de valor 1 ainda não visitadas. Cada nova célula encontrada inicia um flood fill iterativo com pilha e incrementa a contagem de objetos.

O flood fill examina os oito vizinhos de cada célula. A marcação ocorre antes de inserir a célula na pilha, evitando inserções repetidas. Um vetor separado registra as células visitadas. Não é utilizada recursão.

## Implementação paralela

### Divisão e rotulação local

A matriz é dividida em faixas contíguas de linhas. Quando há somente uma linha, a divisão usa faixas contíguas de colunas. A divisão usa o quociente entre linhas e threads; as linhas restantes são distribuídas entre as primeiras threads. Na exceção de uma linha, o mesmo cálculo distribui as colunas. Isso equilibra o número de linhas, embora não garanta o mesmo custo de processamento em matrizes com distribuição irregular de objetos.

Cada thread percorre sua faixa e aplica flood fill sem sair dela. O vetor de rótulos também registra quais células já foram visitadas. A thread `t` inicia seus identificadores em `t * linhas * colunas + 1`, reservando intervalos exclusivos de rótulos.

### Coordenação e memória compartilhada

A thread principal cria as trabalhadoras com `pthread_create` e aguarda seu término com `pthread_join`. A matriz é compartilhada somente para leitura. Cada trabalhadora escreve apenas nas posições do vetor de rótulos pertencentes à sua faixa e nos seus próprios dados.

No caminho normal, não são necessários mutexes para essas escritas, pois as regiões são exclusivas. A consolidação começa somente depois que todos os `pthread_join` foram concluídos com sucesso.

Se a criação de uma thread falhar, o programa aguarda as threads já criadas antes de liberar os dados e retornar erro. Se `pthread_join` falhar, encerra o processo com erro sem liberar manualmente dados que ainda possam estar em uso; o sistema operacional recupera os recursos ao encerrar o processo.

### Consolidação nas fronteiras

Um objeto pode ser dividido em componentes locais ao atravessar faixas. Por isso, somar contagens locais não é suficiente.

Após a rotulação, a thread principal usa Union-Find para representar equivalências entre rótulos. Para cada fronteira entre faixas, compara a última linha da faixa superior com a primeira da seguinte, verificando conexões verticais, diagonais à esquerda e diagonais à direita. As conexões horizontais ficam dentro de cada faixa de linhas e já são tratadas pelo flood fill local. Na exceção de uma única linha, são verificadas as células adjacentes nas fronteiras entre faixas de colunas.

Uniões sucessivas também consolidam objetos que atravessam várias faixas. Uma varredura final conta as raízes distintas dos rótulos presentes na matriz. A decomposição não usa blocos em duas dimensões; as conexões diagonais entre regiões são tratadas nas fronteiras das faixas.

### Trechos paralelos e sequenciais

A rotulação das faixas ocorre concorrentemente. A leitura da entrada, a preparação das threads, a inicialização do Union-Find, a consolidação e a contagem final permanecem sequenciais.

## Testes de corretude

As cinco matrizes obrigatórias foram testadas na versão sequencial e no paralelo com 2 e 4 threads. Em todas as configurações, as contagens corresponderam aos resultados esperados: 3, 4, 5, 6 e 7 objetos.

- [Procedimento e tabela dos testes](results/corretude.md).
- [Registro de execução dos testes](results/corretude.txt).

A matriz adicional `tests/grande.txt` tem 2000 × 2000 células preenchidas com 1, formando um objeto que atravessa todas as faixas. Para executá-la:

```bash
./sequencial < tests/grande.txt
./paralelo 2 < tests/grande.txt
./paralelo 4 < tests/grande.txt
```

## Desempenho

O cronômetro usa `clock_gettime(CLOCK_MONOTONIC)` para medir tempo decorrido. A leitura da entrada fica fora do intervalo; no paralelo, criação e espera das threads, consolidação e contagem final são incluídas. Algumas alocações anteriores ao início da medição também ficam fora do intervalo.

Na matriz de 2000 × 2000, foram realizadas dez medições por configuração. Todas retornaram 1 objeto. O valor representativo é a mediana, e a aceleração é calculada por `S = Tsequencial / Tparalelo`.

| Configuração | Mediana (s) | Speedup |
|---|---:|---:|
| Sequencial | 0,306500 | 1,000× |
| 2 threads | 0,288983 | 1,061× |
| 4 threads | 0,237126 | 1,293× |

Os resultados valem para a matriz e o ambiente testados. A análise detalhada explica a sobrecarga e as limitações do experimento.

- [Metodologia, tempos e análise](results/desempenho.md).
- [Registro das dez repetições](results/desempenho.txt).

## Limitações atuais

- Os valores lidos ainda não são rejeitados quando diferem de 0 e 1; devem ser fornecidas matrizes binárias válidas.
- A conversão do argumento de threads usa `atoi`, sem validação completa.
- Não há proteção completa contra overflow nos produtos usados para dimensões, índices e rótulos.
- O Union-Find reserva espaço proporcional ao número de threads multiplicado pelo número de células, podendo consumir muita memória.
- A pilha é alocada novamente para cada componente.
- A validação apresentada cobre os testes registrados; não comprova todas as entradas possíveis.

## Referências e ferramentas

- Enunciado: *Contagem paralela de objetos em uma matriz binária*, Sistemas Operacionais — PUCRS, 2026/II, Prof. Filipo Mór.
- APIs utilizadas: biblioteca padrão C e interfaces POSIX Pthreads e `clock_gettime`.
- Ferramentas utilizadas: compilador `cc`, `make`, Ubuntu/WSL e Python 3 para gerar a matriz grande.
- Houve auxílio de ChatGPT/Codex na revisão do projeto, correção da medição de tempo, organização dos testes, cálculo de resultados e elaboração da documentação. A revisão e o domínio do trabalho são responsabilidade dos integrantes.

## Pendências antes da entrega

- Revisar as limitações técnicas descritas acima.
- Entregar o endereço do repositório pelo Moodle, conforme orientação da disciplina.
