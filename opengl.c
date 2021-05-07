#include "opengl.h"

// Altere os defines abaixo caso queira trocar as cores
// dos histogramas e/ou dos marcadores

// Cores do histograma original (0..1, Ahist é o nível de opacidade)
#define Rhist   0.8
#define Ghist   0.8  // Cinza claro
#define Bhist   0.8
#define Ahist   0.6  // 40% transparência

// Cores do histograma ajustado (0..1, Aajust é o nível de opacidade)
#define Rajust  0.0
#define Gajust  0.7   // Verde 70%
#define Bajust  0.0
#define Aajust  0.5   // 50% transparência

// Cor dos marcadores de nível mínimo e máximo
#define Rmark   1.0
#define Gmark   0.0   // Vermelho 100%
#define Bmark   0.0

// ************************************************************
//
// VOCÊ NÃO DEVE ALTERAR ESTE MÓDULO DO PROGRAMA!!!
//
// A NÃO SER QUE VOCÊ SAIBA ***EXATAMENTE*** O QUE ESTÁ FAZENDO
//
// ************************************************************

// Dimensões da imagem de entrada
extern int sizeX, sizeY;

// Imagem de entrada
extern float *image;

// Imagem de saída
extern unsigned char *image8;

// Texid
GLuint tex;

// Escala de exibição da imagem
float scale;

// Externs são variáveis declaradas em outro módulo (no caso, main.c)

extern float exposure;
extern unsigned char showhist;

extern int minLevel;
extern int maxLevel;

extern float histogram[HISTSIZE];
extern float adjusted[HISTSIZE];

extern void process();

#define LARGURA_JAN 1024
#define ALTURA_JAN 768

void init(int argc, char **argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(10, 10);

    // Define o tamanho da janela gráfica do programa
    glutInitWindowSize(LARGURA_JAN, ALTURA_JAN);
    glutCreateWindow("Visualizador HDR");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(arrow_keys);
    glutMouseFunc(mouse);

    scale = 1.0f; // escala inicial de exibição
}

void reshape(int w, int h)
{
    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);
    gluOrtho2D(0, w, 0, h);

    // Set the clipping volume
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Reconstrói a textura da imagem de saída quando algum parâmetro
// for alterado
void buildTex()
{
    if (tex == -1) // só precisa gerar o texid uma única vez
        glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D, tex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // Vamos usar minificação/magnificação com interpolação linear,
    // para a qualidade ficar melhor caso o usuário aproxime/afaste
    // a imagem
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sizeX, sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image8);
}

void display()
{
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // Fundo de tela preto
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);

    int glut_width = glutGet(GLUT_WINDOW_WIDTH);
    int glut_height = glutGet(GLUT_WINDOW_HEIGHT);

    float offsetx, offsety;

    // Calcula offsets para centralizar imagem
    offsetx = glut_width / 2 - (sizeX-256) * scale / 2;
    offsety = glut_height / 2 - sizeY * scale / 2;

    glColor3ub(255, 255, 255); // cinza claro

    glPushMatrix();
    // Posiciona e aplica escala na imagem
    glTranslatef(offsetx, offsety, 0);
    glScalef(scale, scale, 1.0);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBegin(GL_QUADS);

    glTexCoord2f(0, 1);
    glVertex2f(0, 0);

    glTexCoord2f(1, 1);
    glVertex2f(sizeX, 0);

    glTexCoord2f(1, 0);
    glVertex2f(sizeX, sizeY);

    glTexCoord2f(0, 0);
    glVertex2f(0, sizeY);

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    if (showhist)
    {
        glColor3f(0.3, 0.3, 0.3);

        glPushMatrix();
        glScalef(1.5,1.5,1);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(256, 0);
        glVertex2f(256, 128);
        glVertex2f(0, 128);
        glEnd();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(Rhist, Ghist, Bhist, Ahist);
        drawHist(histogram);
        glColor4f(Rajust, Gajust, Bajust, Aajust);
        drawHist(adjusted);
        glDisable(GL_BLEND);

        glColor3f(Rmark, Gmark, Bmark);
        glPointSize(5.0);
        glBegin(GL_POINTS);
        glVertex2f(minLevel, 2);
        glVertex2f(maxLevel, 2);
        glEnd();
        glPopMatrix();
    }

    // Exibe a tela
    glutSwapBuffers();
}

void drawHist(float hist[])
{
    glBegin(GL_LINES);
    for (int i = 0; i < HISTSIZE; i++)
    {
        glVertex2f(i, 0);
        glVertex2f(i, hist[i] * 128);
    }
    glEnd();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:
        free(image8);
        free(image);
        exit(1);
        break;
    case 'h':
        showhist = !showhist;
        break;
    case 'a':
        if (--minLevel < 0)
            minLevel = 0;
        break;
    case 's':
        if (++minLevel >= maxLevel)
            minLevel = maxLevel - 1;
        break;
    case 'k':
        if (--maxLevel <= minLevel)
            maxLevel = minLevel + 1;
        break;
    case 'l':
        if (++maxLevel > HISTSIZE-1)
            maxLevel = HISTSIZE-1;
        break;
    }
    process();
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
    //printf("%d %d %d\n",button,x,y);
    if (button == 3)
        scale -= 0.02;
    else if (button == 4)
        scale += 0.02;
    glutPostRedisplay();
}

#define OFFSET 0.05
void arrow_keys(int a_keys, int x, int y)
{
    switch (a_keys)
    {
    case GLUT_KEY_RIGHT:
        exposure += OFFSET;
        process();
        glutPostRedisplay();
        break;
    case GLUT_KEY_LEFT:
        exposure -= OFFSET;
        process();
        glutPostRedisplay();
        break;
    default:
        break;
    }
}
