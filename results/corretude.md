# Testes de corretude

## Objetivo

Verificar a contagem de objetos com conectividade 8 nas cinco matrizes obrigatórias do enunciado, comparando a versão sequencial com a versão paralela usando 2 e 4 threads.

## Ambiente e execução

Os testes foram executados no Ubuntu pelo WSL, em um computador com Windows. Os programas foram compilados com o Makefile do projeto, usando `-std=c89 -Wall -Wextra -pedantic` e, na versão paralela, `-pthread`. Não foram exibidos erros ou avisos na compilação registrada.

Na pasta que contém o Makefile:

```bash
make -B

for i in 1 2 3 4 5; do
    echo "=== Exemplo $i ==="
    ./sequencial < "tests/exemplo$i.txt"
    ./paralelo 2 < "tests/exemplo$i.txt"
    ./paralelo 4 < "tests/exemplo$i.txt"
done
```

## Resultados observados

| Exemplo | Dimensões | Esperado | Sequencial | Paralelo: 2 threads | Paralelo: 4 threads |
|---|---|---:|---:|---:|---:|
| 1 | 5 × 5 | 3 | 3 | 3 | 3 |
| 2 | 6 × 8 | 4 | 4 | 4 | 4 |
| 3 | 8 × 8 | 5 | 5 | 5 | 5 |
| 4 | 9 × 12 | 6 | 6 | 6 | 6 |
| 5 | 12 × 12 | 7 | 7 | 7 | 7 |

As 15 execuções registradas produziram as contagens esperadas. A versão paralela concordou com a sequencial nas duas configurações testadas.

Após a alteração da medição de tempo para `clock_gettime(CLOCK_MONOTONIC)`, o exemplo 5 também foi executado novamente nas três configurações, mantendo o resultado de 7 objetos. Além disso, as cópias corrigidas dos fontes foram verificadas no Ubuntu nos cinco exemplos e nas três configurações, com os mesmos resultados esperados.

## Escopo da validação

Estes resultados comprovam as contagens nas matrizes e configurações testadas. Não demonstram correção para todas as entradas possíveis nem substituem testes adicionais de limites e tratamento de erros.

Os tempos destas matrizes pequenas não são usados aqui para calcular speedup. A avaliação de desempenho requer uma matriz maior e medições repetidas, que ainda estão pendentes.
