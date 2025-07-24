#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

// Endereço da JTAG_UART
#define IO_JTAG_UART (*(volatile unsigned int *)0xFF201000)

// Tamanho do array de entrada
#define SIZE 50


//ARRAY DE PIXELS
#define VGA_BASE 0xc8000000
uint16_t *ppix = (uint16_t *)(VGA_BASE);

//largura da tela
#define col_max 320
//largura falsa da tela
#define mem_col_max 512


//altura da tela
#define lin_max 240





typedef struct {
    const char *nome;
    uint16_t valor;
} Cor;

// Cores predefinidas (RGB 565)
const Cor paleta[] = {
    {"PRETO",    0x0000},
    {"BRANCO",   0xFFFF},
    {"CINZA",    0x8410},
    {"VERMELHO", 0xF800},
    {"VERDE",    0x07E0},
    {"AZUL",     0x001F},
    {"MAGENTA",  0xF81F},
    {"AMARELO",  0xFFE0},
    {"CIANO",    0x07FF}
};
const int num_cores = sizeof(paleta) / sizeof(Cor);

const Cor* buscar_cor(const char *entrada) {
    for (int i = 0; i < num_cores; i++) {
        if (strcasecmp(entrada, paleta[i].nome) == 0) {
            return &paleta[i];  // retorna ponteiro para struct
        }
    }
    return NULL;
}


// Cor atual
uint16_t current_color = 0x0000;
char current_color_name[12] = "PRETO";


// Função para limpar os caracteres da JTAG_UART antigos
void limpar_teclas() {
    unsigned int data;
    // Enquanto tiver caractere disponível (bit 15 == 1)
    while (IO_JTAG_UART & 0x8000) {
        data = IO_JTAG_UART;  // Lê e descarta
    }
}

//Recebe um array e seu tamanho para escrever o que o usuário digita na Jtag Uart. Trata Backspace para apagar, enter para enviar e ESC apaga tudo e escreve apenas ESC, para cancelar funções. Ecoa tudo que é digitado.
void input(char *buffer, int tam){
    int index = 0;
    
    
    while (1) {
        unsigned int data = IO_JTAG_UART;
        if (data & 0x8000) { // Dado válido?
            char  tecla = data & 0xFF; // Extrai o caractere

            // Trata backspace (ASCII 127)
            if (tecla == 127 && index > 0) { 
                index--;
                printf("\b \b"); // Apaga o caractere no terminal
                fflush(stdout);
            }
            // Trata Enter (finaliza leitura)
            else if (tecla == 10) {
                buffer[index] = '\0'; // Adiciona terminação
                printf("\n"); // Nova linha
                break;
            }
            // Trata ESC (cancelar opção atual)
            else if (tecla == 27) {  
                memset(buffer, 0, tam);
                strcpy(buffer, "EsC");
                printf("\033[2K\r");
                fflush(stdout);
                break;
            }
            // Caractere normal
            else if ((index < tam- 1) && isprint(tecla)) {
                buffer[index++] = tecla;
                printf("%c", tecla); // Ecoa o caractere
                fflush(stdout);
            }
            else if (index >= tam - 1) {
                memset(buffer, 0, tam);
                index = 0;
                printf("\033[2K\r");
                fflush(stdout);
                printf("Voce digitou mais do que pode, coloque um comando valido:\n");
            }
        }
    }
}


//Interface principal de seleção de comandos. Recebe um vetor de char e seu tamanho para acomodar os inputs do usuário
void menu(char *comando, int tam){
    printf("Ola amigo! Escolha o que voce quer fazer hoje:\n");
    do{
        printf("#Escolher a Cor _________ (COR)\n#Desenhar Linha _________ (LIN)\n#Desenhar Circulo _______ (CIR)\n#Desenhar Caixa _________ (BOX)\n#Desenhar Retangulo _____ (RET)\n#Preencher Fundo ________ (FUN)\nEscreva o seu comando: ");
        fflush(stdout);
        memset(comando, 0, tam);
        input(comando,tam);
        if (strncasecmp(comando, "COR", 3) == 0){
            menu_cor(comando,tam);
        }
        else if(strncasecmp(comando, "LIN", 3)==0){
            menu_linha(comando,tam);
        }
        else if(strncasecmp(comando, "CIR", 3)==0){
            menu_circulo(comando,tam);
        }
        else if(strncasecmp(comando, "BOX", 3)==0){
            menu_caixa(comando,tam);
        }
        else if(strncasecmp(comando, "RET", 3)==0){
            menu_retangulo(comando,tam);
        }
        else if(strncasecmp(comando, "FUN", 3)==0){
            menu_fundo(comando,tam);
        }
        else if(strcmp(comando, "EsC")!=0){
            printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
            printf("Comando invalido, escreva uma das opcoes e aperte enter\n");
        }
    } while ((strcmp(comando, "EsC")!=0));

}


// Converte RGB888 para RGB565
uint16_t rgb_to_565(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t r5 = (r * 31) / 255;
    uint16_t g6 = (g * 63) / 255;
    uint16_t b5 = (b * 31) / 255;
    return (r5 << 11) | (g6 << 5) | b5;
}


//Chama a interação com o usuário para definir a cor
void menu_cor(char *cor, int tam){
    printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
    int feito = 0;
    do{
        printf("Escolha o nome de uma das cores pre definidas a baixo:\n#VERMELHO #AZUL    #VERDE #PRETO #BRANCO \n#MAGENTA  #AMARELO #CIANO #CINZA \n\nOu sua propria cor em RGB255\nOBS:3 numeros de 0 a 255 separados por espaco\nOBS2: Sendo valores para vermelho, verde e azul, respectivamente\nEx de laranja: 255 127 0\nEscreva o sua cor:");
        fflush(stdout);
        memset(cor, 0, tam);
        input(cor,tam);

                                                        
        
        if(strcmp(cor, "EsC")!=0){       
            const Cor *cor_encontrada = buscar_cor(cor);
            if (cor_encontrada != NULL) {
                current_color = cor_encontrada->valor  ;
                printf("\033[2J\033[H");  // limpa tela
                strcpy(current_color_name, cor_encontrada->nome);
                printf("A cor atual e: %s\n", current_color_name);
                feito = 1;
            }

            else {
                // Tenta ler 3 valores inteiros
                int r, g, b;
                int lidos = sscanf(cor, "%d %d %d", &r, &g, &b);

                if (lidos == 3 && r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
                    current_color = rgb_to_565((uint8_t)r, (uint8_t)g, (uint8_t)b);
                    printf("\033[2J\033[H");  // limpa tela
                    sprintf(current_color_name, "#%02X%02X%02X", r, g, b);
                    printf("A cor atual e: %s\n", current_color_name);
                    feito = 1;
                } else {
                    printf("Entrada invalida. Use cor pre definida ou RGB.\n");
                }
            }
        } else{
            printf("\033[2J\033[H");  // limpa tela
        }
    } while ((strcmp(cor, "EsC")!=0) && !feito);
    memset(cor, 0, tam);

}

//Chama a interação com o usuário para definir o fundo
void menu_fundo(char *comando, int tam){
    printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
    memset(comando, 0, tam);
    printf("A cor atual e: %s\n", current_color_name);
    printf("Tem certeza que quer realizar essa funcao?\nEla ira mudar toda a tela para a cor atual, apagando tudo\nque estiver desenhado\nSim: Enter\nNao: ESC\n>");
    fflush(stdout);
    input(comando,tam);
    if (strcmp(comando, "EsC")!=0){
        fundo(current_color);    
    }
    printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
    memset(comando, 0, tam);
}

//Chama a interação com o usuário para definir as coordenadas de linhas
void menu_linha(char *comando, int tam){
    printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
    do{
        printf("A cor atual e: %s\n", current_color_name);
        printf("Para desenhar uma linha, de as coordenadas x;y do ponto\ninicial e final da linha separados por espaco,para interromper, aperte ESC.\nEx para (0;0)_(319;239): 0 0 319 239\nAs coordenadas devem estar nos limites a seguir:\n0<= x <320  0<= y <240\n\nSua coordenada>");
        fflush(stdout);
        memset(comando, 0, tam);
        input(comando,tam);
        // Tenta ler 4 valores inteiros
        int x0, y0, x1, y1;
        int lidos = sscanf(comando, "%d %d %d %d", &x0, &y0, &x1, &y1);
        if (lidos ==4 && y0 >= 0 && y0 < lin_max && x0 >= 0 && x0 < col_max && y1 >= 0 && y1 < lin_max && x1 >= 0 && x1 < col_max){
            linha(x0,y0,x1,y1,current_color);
            printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
        }
        else if((strcmp(comando, "EsC")!=0)){
            printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
            printf("Coordenadas fora de alcance ou formatacao invalida\n");
        }
    }while((strcmp(comando, "EsC")!=0));

    printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
    memset(comando, 0, tam);
}


//Chama a interação com o usuário para definir as coordenadas de caixas
void menu_caixa(char *comando, int tam){
    printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
    do{
        printf("A cor atual e: %s\n", current_color_name);
        printf("Para desenhar uma caixa, de as coordenadas x;y de dois vertices\nopostos separados por espaco,para interromper, aperte ESC.\nEx para (0;0)>(319;239): 0 0 319 239\nAs coordenadas devem estar nos limites a seguir:\n0<= x <320  0<= y <240\n\nSua coordenada>");
        fflush(stdout);
        memset(comando, 0, tam);
        input(comando,tam);
        // Tenta ler 4 valores inteiros
        int x0, y0, x1, y1;
        int lidos = sscanf(comando, "%d %d %d %d", &x0, &y0, &x1, &y1);
        if (lidos ==4 && y0 >= 0 && y0 < lin_max && x0 >= 0 && x0 < col_max && y1 >= 0 && y1 < lin_max && x1 >= 0 && x1 < col_max){
            caixa(x0,y0,x1,y1,current_color);
            printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
        }
        else if((strcmp(comando, "EsC")!=0)){
            printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
            printf("Coordenadas fora de alcance ou formatacao invalida\n");
        }
    }while((strcmp(comando, "EsC")!=0));

    printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
    memset(comando, 0, tam);
}



//Chama a interação com o usuário para definir as coordenadas de circulos
void menu_circulo(char *comando, int tam){
    printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
    do{
        printf("A cor atual e: %s\n", current_color_name);
        printf("Para desenhar um circulo, de as coordenadas x;y do centro e\no tamanho do raio separados por espaco,para interromper, aperte ESC.\nEx para um circulo em (160;120) e raio 20: 160 120 20\nAs coordenadas devem estar nos limites a seguir:\n0<= x <320  0<= y <240\n\nSua coordenada>");
        fflush(stdout);
        memset(comando, 0, tam);
        input(comando,tam);
        // Tenta ler 3 valores inteiros
        int xc, yc, r;
        int lidos = sscanf(comando, "%d %d %d", &xc, &yc, &r);
        if (lidos ==3 && yc >= 0 && yc < lin_max && xc >= 0 && xc < col_max && r<400){
            circulo(xc,yc,r,current_color);
            printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
        }
        else if((strcmp(comando, "EsC")!=0)){
            printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
            printf("Coordenadas fora de alcance ou formatacao invalida\n");
        }
    }while((strcmp(comando, "EsC")!=0));

    printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
    memset(comando, 0, tam);
}

//Chama a interação com o usuário para definir as coordenadas de retangulos
void menu_retangulo(char *comando, int tam){
    printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
    do{
        printf("A cor atual e: %s\n", current_color_name);
        printf("Para desenhar um retangulo, de as coordenadas x;y de dois vertices\nopostos separados por espaco,para interromper, aperte ESC.\nEx para (0;0)>(319;239): 0 0 319 239\nAs coordenadas devem estar nos limites a seguir:\n0<= x <320  0<= y <240\n\nSua coordenada>");
        fflush(stdout);
        memset(comando, 0, tam);
        input(comando,tam);
        // Tenta ler 4 valores inteiros
        int x0, y0, x1, y1;
        int lidos = sscanf(comando, "%d %d %d %d", &x0, &y0, &x1, &y1);
        if (lidos ==4 && y0 >= 0 && y0 < lin_max && x0 >= 0 && x0 < col_max && y1 >= 0 && y1 < lin_max && x1 >= 0 && x1 < col_max){
            retangulo(x0,y0,x1,y1,current_color);
            printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
        }
        else if((strcmp(comando, "EsC")!=0)){
            printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
            printf("Coordenadas fora de alcance ou formatacao invalida\n");
        }
    }while((strcmp(comando, "EsC")!=0));

    printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
    memset(comando, 0, tam);
}




//Colore um pixel na tela com a cor passada para ele. Recebe
void setpix(int lin, int col, uint16_t cor){
    if (lin >= 0 && lin < lin_max && col >= 0 && col < col_max){
        ppix[lin*mem_col_max + col] = cor;
    }
}


//Desenha uma linha de (x0;y0) para (x1;y1) com a cor, não verifica se as coordenadas estão dentro dos limites
void linha(int x0, int y0, int x1, int y1, uint16_t cor) {
    int dx = abs(x1 - x0), sx = (x0 < x1) ? 1 : -1;
    int dy = -abs(y1 - y0), sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy, e2;

    setpix(y0, x0, cor); //dessa forma o while para só, sem precisar de um break
    while (!(x0 == x1 && y0 == y1)) {
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
        setpix(y0, x0, cor);
    }
}

//Desenha um circulo com raio e enctrado em (xc;yc) com a cor, não verifica se as coordenadas estão dentro dos limites
void circulo(int xc, int yc, int raio, uint16_t cor) {
    int x = 0, y = raio;
    int d = 3 - 2 * raio;

    while (x <= y) {
        // Desenha 8 pontos por simetria (octantes)
        setpix(yc+x, xc+y, cor); setpix(yc-x, xc+y, cor);
        setpix(yc+x, xc-y, cor); setpix(yc-x, xc-y, cor);
        setpix(yc+y, xc+x, cor); setpix(yc-y, xc+x, cor);
        setpix(yc+y, xc-x, cor); setpix(yc-y, xc-x, cor);
        
        if (d < 0) d += 4 * x + 6;
        else { d += 4 * (x - y) + 10; y--; }
        x++;
    }
}


//Desenha uma caixa com verteces ospostos em (x0;y0) para (x1;y1) com a cor, não verifica se as coordenadas estão dentro dos limites
void caixa(int x0, int y0, int x1, int y1, uint16_t cor) {
    linha(x0, y0, x1, y0, cor); // Topo
    linha(x0, y1, x1, y1, cor); // Base
    linha(x0, y0, x0, y1, cor); // Esquerda
    linha(x1, y0, x1, y1, cor); // Direita
}

//Desenha um retangulo com verteces ospostos em (x0;y0) para (x1;y1) com a cor, não verifica se as coordenadas estão dentro dos limites
void retangulo(int x0, int y0, int x1, int y1, uint16_t cor) {
    int x_min = (x0 < x1) ? x0 : x1, x_max = (x0 > x1) ? x0 : x1;
    int y_min = (y0 < y1) ? y0 : y1, y_max = (y0 > y1) ? y0 : y1;
    
    for (int y = y_min; y <= y_max; y++) {
        for (int x = x_min; x <= x_max; x++) {
            setpix(y,x,current_color);
        }
    }
}




//Muda a cor de toda tela com a cor
void fundo(uint16_t cor) {
    for (int i = 0; i < lin_max * mem_col_max; i++) {
        ppix[i] = cor;
    }
}


/*Flood fill???*/


int main(void) {
    limpar_teclas();
    printf("\033[2J\033[H"); // Limpa tela e move cursor para o topo
    fundo(paleta[0].valor);
    char comando[SIZE];
    menu(comando, SIZE);
    return 0;
}