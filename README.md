# Documentação do Código - Projeto VGA com JTAG\_UART na DE1-SoC

Este projeto implementa uma interface de desenho para a placa FPGA DE1-SoC, permitindo interação via JTAG\_UART com comandos para desenhar elementos gráficos (linhas, círculos, retângulos, caixas, etc.) na tela VGA.

Abaixo está uma descrição detalhada das principais funções do código, bem como da struct `Cor` e da função principal `main`.

---

## Struct `Cor`

```c
typedef struct {
    const char *nome;
    uint16_t valor;
} Cor;
```

Essa struct representa uma cor em formato RGB565. Ela armazena:

* `nome`: nome descritivo da cor (ex: "VERMELHO")
* `valor`: valor numérico de 16 bits que representa a cor no formato RGB565.

A struct é usada como base para a paleta de cores predefinidas do programa.

---

## Funções de Utilidade

### `limpar_teclas()`

```c
void limpar_teclas();
```

Limpa todos os caracteres pendentes no buffer da JTAG\_UART. Evita que entradas anteriores afetem o próximo comando do usuário.

---

### `input(char *buffer, int tam)`

```c
void input(char *buffer, int tam);
```

Lê texto digitado pelo usuário via JTAG\_UART. Trata os seguintes casos:

* `Enter` finaliza a entrada
* `Backspace` apaga caractere
* `ESC` cancela a entrada (buffer recebe "EsC")
* Imprime feedback no terminal com `printf`

---

## Menus e Controle de Fluxo

### `menu(char *comando, int tam)`

Apresenta o menu principal e direciona para os submenus com base na entrada do usuário:

* COR: seleciona cor
* LIN, CIR, BOX, RET: desenhos
* FUN: preenche fundo
* EsC: sai do menu

---

## Manipulação de Cores

### `const Cor* buscar_cor(const char *entrada)`

Busca uma cor na `paleta[]` com base no nome fornecido (case-insensitive). Retorna ponteiro para a struct `Cor` correspondente.

---

### `uint16_t rgb_to_565(uint8_t r, uint8_t g, uint8_t b)`

Converte três componentes RGB (0-255) para um valor de 16 bits no formato RGB565.

---

### `menu_cor(char *cor, int tam)`

Interface com o usuário para definir a cor atual. Pode ser:

* Escolhida da paleta
* Definida com valores RGB (ex: `255 127 0`)
* Cancelada com ESC

---

## Submenus de Desenho

Cada um desses menus interage com o usuário, recebe as coordenadas e chama a função de desenho apropriada:

### `menu_linha`, `menu_circulo`, `menu_caixa`, `menu_retangulo`

* Validam limites da tela
* Podem ser interrompidos com ESC
* Realizam limpeza da tela após cada interação

### `menu_fundo`

Solicita confirmação antes de pintar a tela inteira com a cor atual.

---

## Funções de Desenho

### `setpix(int lin, int col, uint16_t cor)`

Pinta um pixel na posição `(col, lin)` com a cor especificada.

---

### `linha(int x0, int y0, int x1, int y1, uint16_t cor)`

Desenha uma linha entre dois pontos usando o algoritmo de Bresenham. Chama `setpix` para cada ponto do caminho.

---

### `caixa(int x0, int y0, int x1, int y1, uint16_t cor)`

Desenha um retângulo sem preenchimento, com linhas nas bordas superior, inferior, esquerda e direita.

---

### `retangulo(int x0, int y0, int x1, int y1, uint16_t cor)`

Desenha um retângulo preenchido entre dois pontos opostos. Usa dois `for` aninhados para preencher linha por linha.

---

### `circulo(int xc, int yc, int raio, uint16_t cor)`

Desenha um círculo centrado em `(xc, yc)` usando o algoritmo do ponto médio. Usa simetria dos octantes para desenhar todos os pixels.

---

### `fundo(uint16_t cor)`

Pinta todos os pixels da tela com a cor fornecida. Usado para limpar ou redefinir o fundo.

---

## Função Principal

### `int main(void)`

1. Limpa o buffer de entrada
2. Limpa a tela com a cor inicial
3. Inicia o menu principal

---

## Possíveis Extensões Futuras

* Implementar flood fill (preenchimento por região)
* Desenho com mouse (caso use periférico)
* Salvamento/restauração de imagens
* Exportação de frames ou prints

---

Esse README pode ser usado como base para a documentação no GitHub do projeto. Se quiser, posso complementar com exemplos, prints de uso ou estrutura de diretórios.
