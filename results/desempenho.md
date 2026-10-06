# Análise de desempenho

## Dados e metodologia

Foi utilizada a matriz `tests/grande.txt`, de 2000 × 2000 células (4 milhões de células), todas preenchidas com 1. Pela conectividade 8, existe um único objeto. A escolha permite conferir o resultado e exercitar um componente que atravessa todas as faixas de linhas. Ela representa uma matriz densa; os resultados não devem ser generalizados para todas as distribuições de objetos.

As execuções ocorreram no Ubuntu pelo WSL no Windows, sobre o mesmo arquivo. Foram realizadas 10 repetições por configuração, sempre na ordem sequencial, paralelo com 2 threads e paralelo com 4 threads. Antes da série registrada, cada configuração já havia sido executada uma vez. Todas as 30 execuções retornaram 1 objeto.

Os programas foram compilados pelo Makefile com `-std=c89 -Wall -Wextra -pedantic` e `-pthread` na versão paralela, sem opção explícita de otimização. O cronômetro usa `clock_gettime(CLOCK_MONOTONIC)`. O intervalo exclui a leitura da entrada; no paralelo inclui a divisão das faixas, criação e espera das threads, inicialização do Union-Find, consolidação e contagem final. Algumas alocações anteriores ao início do cronômetro ficam fora da medição.

Foi adotada a mediana para reduzir a influência de valores extremos. Para 10 medições, ela é a média do quinto e do sexto valores após ordenação. Os valores foram calculados a partir dos tempos exibidos com seis casas decimais. O registro original está em [desempenho.txt](desempenho.txt).

## Tempos registrados (segundos)

| Repetição | Sequencial | Paralelo: 2 threads | Paralelo: 4 threads |
|---|---:|---:|---:|
| 1 | 0.409107 | 0.268943 | 0.234347 |
| 2 | 0.305913 | 0.271737 | 0.226024 |
| 3 | 0.339355 | 0.296570 | 0.255736 |
| 4 | 0.292423 | 0.289404 | 0.245971 |
| 5 | 0.316795 | 0.285988 | 0.236698 |
| 6 | 0.298301 | 0.295475 | 0.228280 |
| 7 | 0.289301 | 0.278705 | 0.237554 |
| 8 | 0.289988 | 0.289765 | 0.245191 |
| 9 | 0.307087 | 0.321327 | 0.242586 |
| 10 | 0.324060 | 0.288562 | 0.231932 |

## Medianas e aceleração

A aceleração foi calculada como `S = mediana sequencial / mediana paralela`.

| Configuração | Mediana (s) | Speedup | Redução do tempo em relação ao sequencial |
|---|---:|---:|---:|
| Sequencial | 0.3065000 | 1,000 | — |
| Paralelo: 2 threads | 0.2889830 | 1.061 | 5.72% |
| Paralelo: 4 threads | 0.2371260 | 1.293 | 22.63% |

## Interpretação

Com base nas medianas, a versão paralela foi mais rápida nas duas configurações. Com 2 threads, o ganho foi pequeno; com 4 threads, o speedup foi aproximadamente 1.29. Isso corresponde a uma redução de 22.6% no tempo mediano com 4 threads, sem atingir aceleração proporcional ao número de threads.

Pela estrutura do código, somente a rotulação dentro das faixas ocorre em paralelo. A inicialização do Union-Find, a consolidação nas fronteiras e a varredura final permanecem sequenciais. A reserva e inicialização de estruturas de rótulos também crescem com a quantidade de threads. Esses custos ajudam a explicar por que o ganho não é linear; não foram medidos separadamente neste experimento.

Na repetição 9, a execução com 2 threads foi mais lenta que a sequencial (0,321327 s contra 0,307087 s). Uma medição isolada não representa toda a série. Variações de escalonamento e carga do sistema são possíveis explicações, mas sua causa não foi investigada.

Para matrizes pequenas, os custos de criação e coordenação das threads podem superar a economia de processamento. Os registros antigos das matrizes obrigatórias não são usados para comparar speedup, pois parte deles antecede a correção do cronômetro.

## Limitações

Este experimento cobre uma matriz densa, duas configurações de threads e uma única máquina. A ordem das configurações foi fixa. Os resultados refletem este ambiente, esta implementação e as opções de compilação utilizadas.

## Ambiente informado

As informações abaixo foram obtidas com `lscpu`, `free -h`, `cat /etc/os-release` e `cc --version`, no mesmo Ubuntu utilizado para os testes.

| Item | Informação |
|---|---|
| Processador identificado | Intel Core i7-1265U, 12ª geração |
| Arquitetura | x86_64 |
| Processadores lógicos visíveis no ambiente | 12 |
| Memória total visível no Ubuntu | 7,6 GiB |
| Memória disponível no momento da consulta | 7,1 GiB |
| Swap visível | 2,0 GiB |
| Sistema convidado | Ubuntu 26.04 LTS (Resolute Raccoon) |
| Ambiente | WSL no Windows, hipervisor Microsoft |
| Compilador `cc` | GCC 15.2.0, pacote Ubuntu 15.2.0-16ubuntu1 |

A memória e a topologia reportadas são as expostas ao ambiente virtualizado, não uma confirmação da memória física total ou da topologia física do computador. A disponibilidade de memória foi consultada depois dos testes. A versão exata do Windows e do WSL não foi registrada.
