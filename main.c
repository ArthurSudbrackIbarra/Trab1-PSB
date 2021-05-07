#include <math.h>
#include <string.h>		// para usar strings

// Rotinas para acesso da OpenGL
#include "opengl.h"

// Protótipos
void process();
void carregaHeader(FILE* fp);
void carregaImagem(FILE* fp, int largura, int altura);
//void criaImagensTeste();

//
// Variáveis globais a serem utilizadas (NÃO ALTERAR!)
//

// Dimensões da imagem de entrada
int sizeX, sizeY;

// Header da imagem de entrada
unsigned char header[11];

// Pixels da imagem de ENTRADA (em formato RGBE)
unsigned char* image;

// Pixels da imagem de SAÍDA (em formato RGB)
unsigned char* image8;

// Fator de exposição
float exposure;

// Histogramas
float histogram[HISTSIZE];
float adjusted[HISTSIZE];

// Flag para exibir/ocultar histogramas
unsigned char showhist = 0;

// Níveis mínimo/máximo (preto/branco)
int minLevel = 0;
int maxLevel = 255;

// Função principal de processamento: ela deve chamar outras funções
// quando for necessário (ex: algoritmo de tone mapping, etc)
void process()
{
    printf("Exposure: %.3f\n", exposure);
    float expos = pow(2, exposure);

    //
    // EXEMPLO: preenche a imagem com pixels cor de laranja...
    //
    //
    // SUBSTITUA este código pelos algoritmos a serem implementados
    //

    unsigned char* ptrImage = image;
    int totalBytesImage = sizeX * sizeY * 4;

    unsigned char* ptrImage8 = image8;

    float expoenteGama = 1/2.2;

    float* ptrHistogram = histogram;
    int maiorDoHistograma = -1;

    for(int pos = 0; pos < totalBytesImage; pos += 4){

        //Obtendo os 4 valores do RGBE.
        int valor1 = *ptrImage++;
        int valor2 = *ptrImage++;
        int valor3 = *ptrImage++;
        int mantissa = *ptrImage++;

        //Calculando o fator de conversao com base na mantissa.
        float fatorConversao = pow(2, mantissa - 136);

        //Convertendo os pixels para float e aplicando o fator de exposicao.
        float red = valor1 * fatorConversao * expos;
        float green = valor2 * fatorConversao * expos;
        float blue = valor3 * fatorConversao * expos;

        //Algoritmo de Tone Mapping - RED.
        float aux = red * 0.6;
        red = (aux * (2.51 * aux + 0.03))/(aux * (2.43 * aux + 0.59) + 0.14);

        //Garantindo que o resultado estara entre 0 e 1.
        if(red > 1) red = 1;
        else if(red < 0) red = 0;

        //Correcao Gama.  
        red = pow(red, expoenteGama);

        //Conversao para 24 bits.
        int finalRed = (int) (red * 255);

        //Aplicando o mesmo processo para o GREEN.
        aux = green * 0.6;
        green = (aux * (2.51 * aux + 0.03))/(aux * (2.43 * aux + 0.59) + 0.14);

        if(green > 1) green = 1;
        else if(green < 0) green = 0;
        
        green = pow(green, expoenteGama);

        int finalGreen = (int) (green * 255);

        //Aplicando o mesmo processo para o BLUE.
        aux = blue * 0.6;
        blue = (aux * (2.51 * aux + 0.03))/(aux * (2.43 * aux + 0.59) + 0.14);

        if(blue > 1) blue = 1;
        else if(blue < 0) blue = 0;

        blue = pow(blue, expoenteGama);

        int finalBlue = (int) (blue * 255);
        
        //Armazenando os valores obtidos no vetor da imagem de saida 'image8'.
        *ptrImage8++ = (unsigned char) finalRed;
        *ptrImage8++ = (unsigned char) finalGreen;
        *ptrImage8++ = (unsigned char) finalBlue;

        /*//Calculando a luminancia dos pixels.
        float luminancia = 0.299 * finalRed + 0.587 * finalGreen + 0.114 * finalBlue;

        //Adicionando a luminancia obtida no vetor responsavel pelo histograma.
        *ptrHistogram++ = luminancia;

        //Mantendo o rastreamento do maior valor presente no histograma.
        if(luminancia > maiorDoHistograma) maiorDoHistograma = luminancia;*/
    }

    /*for(int i = 0; i < HISTSIZE; i++){
        histogram[i] = histogram[i]/maiorDoHistograma;
    }*/

    // Dica: se você precisar de um vetor de floats para armazenar
    // a imagem convertida, etc, use este trecho
    // (não esqueça o free no final)
    // float *fpixels = malloc(sizeX * sizeY * 3 * sizeof(float));
    
    // NÃO ALTERAR A PARTIR DAQUI!!!!
    //
    //free(fpixels);
    buildTex();
}

int main(int argc, char** argv)
{
    if(argc==1) {
        printf("hdrvis [image file.hdf]\n");
        exit(1);
    }

    // Inicialização da janela gráfica
    init(argc,argv);

    //
    // PASSO 1: Leitura da imagem
    // A leitura do header já foi feita abaixo
    // 
    FILE* arq = fopen(argv[1], "rb");
    carregaHeader(arq);

    //
    // IMPLEMENTE AQUI o código necessário para
    // extrair as informações de largura e altura do header

    int largura = header[3] + (header[4] << 8) + (header[5] << 16) + (header[6] << 24);
    int altura = header[7] + (header[8] << 8) + (header[9] << 16) + (header[10] << 24);

    printf("Minha largura: %d, Minha altura: %d\n", largura, altura);

    //printf("\nBytes Largura: %02X, %02X, %02X, %02X\n", header[3], header[4], header[5], header[6]);
    //printf("\nBytes Altura: %02X, %02X, %02X, %02X\n", header[7], header[8], header[9], header[10]);

    //
    // Descomente a linha abaixo APENAS quando isso estiver funcionando!
    //
    carregaImagem(arq, largura, altura);
    
    // Fecha o arquivo
    fclose(arq);

    //
    // COMENTE a linha abaixo quando a leitura estiver funcionando!
    // (caso contrário, ele irá sobrepor a imagem carregada com a imagem de teste)
    //
    //criaImagensTeste();

    exposure = 0.0f; // exposição inicial

    // Aplica processamento inicial
    process();

    // Não retorna... a partir daqui, interação via teclado e mouse apenas, na janela gráfica

    // Mouse wheel é usada para aproximar/afastar
    // Setas esquerda/direita: reduzir/aumentar o fator de exposição
    // A/S: reduzir/aumentar o nível mínimo (black point)
    // K/L: reduzir/aumentar o nível máximo (white point)
    // H: exibir/ocultar o histograma
    // ESC: finalizar o programa
   
    glutMainLoop();

    return 0;
}

// Função apenas para a criação de uma imagem em memória, com o objetivo
// de testar a funcionalidade de exibição e controle de exposição do programa
void criaImagensTeste()
{
    // TESTE: cria uma imagem de 800x600
    sizeX = 800;
    sizeY = 600;

    printf("%d x %d\n", sizeX, sizeY);

    // Aloca imagem de entrada (32 bits RGBE)
    image = (unsigned char*) malloc(sizeof(unsigned char) * sizeX * sizeY * 4);

    // Aloca memória para imagem de saída (24 bits RGB)
    image8 = (unsigned char*) malloc(sizeof(unsigned char) * sizeX * sizeY * 3);
}

// Esta função deverá ser utilizada para ler o conteúdo do header
// para a variável header (depois você precisa extrair a largura e altura da imagem desse vetor)
void carregaHeader(FILE* fp)
{
    // Lê 11 bytes do início do arquivo
    fread(header, 11, 1, fp);
    // Exibe os 3 primeiros caracteres, para verificar se a leitura ocorreu corretamente
    printf("Id: %c%c%c\n", header[0], header[1], header[2]);
}

// Esta função deverá ser utilizada para carregar o restante
// da imagem (após ler o header e extrair a largura e altura corretamente)
void carregaImagem(FILE* fp, int largura, int altura)
{
    sizeX = largura;
    sizeY = altura;

    // Aloca imagem de entrada (32 bits RGBE)
    image = (unsigned char*) malloc(sizeof(unsigned char) * sizeX * sizeY * 4);

    // Aloca memória para imagem de saída (24 bits RGB)
    image8 = (unsigned char*) malloc(sizeof(unsigned char) * sizeX * sizeY * 3);

    // Lê o restante da imagem de entrada
    fread(image, sizeX * sizeY * 4, 1, fp);
    // Exibe primeiros 3 pixels, para verificação
    for(int i=0; i<12; i+=4) {
        printf("%02X %02X %02X %02X\n", image[i], image[i+1], image[i+2], image[i+3]);
    }
}

