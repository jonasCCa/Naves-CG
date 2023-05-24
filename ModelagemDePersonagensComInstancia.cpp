#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>

#define PI 3.14159265358979323846

using namespace std;

#ifdef WIN32
#include <windows.h>
#include <glut.h>
#else
#include <sys/time.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif

#ifdef __linux__
#include <glut.h>
#endif

#include "Ponto.h"
#include "Modelo.h"
#include "Instancia.h"
#include <vector>

#include "Temporizador.h"

//QUANTIDADE DE INIMIGOS
int qntInimigos = 20;

class Point {
public:
    float x, y;
    void setxy(float x2, float y2)
    {
        x = x2; y = y2;
    }
    const Point & operator=(const Point &rPoint)
    {
        x = rPoint.x;
        y = rPoint.y;
        return *this;
    }

};



Temporizador T;
double AccumDeltaT=0;

const int tamanho = 5;

vector<Ponto> VetorModelos[tamanho];

Ponto VetorFrente(0,1,0);
Modelo MeiaSeta;
Modelo Mastro;
//*****************
//0 Atira
//1 esquerda
//2 direita
//3 cima
//4 baixo
//*****************
bool vetorTeclas[5];


int qntAtInimigos;

float vel = 0.3;

int vida;
int vidaAnt;

Instancia* Universo;

Ponto Min, Max;

bool desenha = false;

float angulo=0.0;
int nInstancias=0;

double nFrames=0;
int frame=0;
double TempoTotal=0;

struct tiro{
    bool existe;
    Ponto frente;
    float x,y;
    float angulo;
};

tiro* tirosInimigos;

void LeObjeto(const char *nome, Poligono &p)
{
    ifstream input;
    input.open(nome, ios::in);
    if (!input)
    {
        cout << "Erro ao abrir " << nome << ". " << endl;
        exit(0);
    }
    cout << "Lendo arquivo " << nome << "...";
    string S;
    int nLinha = 0;
    unsigned int qtdVertices;

    input >> qtdVertices;
    double x,y;

    // Le a primeira linha apenas para facilitar o calculo do limites
    input >> x >> y;

    Min = Ponto(x,y);
    Max = Ponto(x,y);
    p.insereVertice(Ponto(x,y));

    for (int i=0; i< qtdVertices; i++)
    {
        // Le cada elemento da linha
        input >> x >> y;
        // atualiza os limites
        if (x<Min.x) Min.x = x;
        if (y<Min.y) Min.y = y;

        if (x>Max.x) Max.x = x;
        if (y>Max.y) Max.y = y;

        if(!input)
            break;
        nLinha++;
        //cout << "Arquivo: " << x << " " << y << endl;
        p.insereVertice(Ponto(x,y));
    }
    cout << "leitura concluida." << endl;
    //cout << "Linhas lidas: " << nLinha << endl;
    //cout << "Limites:" << endl;

}

void CarregaModelo(const char *nome, int NModelo)
{
     ifstream input;
    input.open(nome, ios::in);
    if (!input)
    {
        cout << "Erro ao abrir " << nome << ". " << endl;
        exit(0);
    }
    cout << "Lendo arquivo " << nome << "...";
    unsigned int qtdCores,linha,coluna;
    vector <Ponto> cores;


    input >> qtdCores;
    for(int i =0; i< qtdCores; i++){
        int R,G,B,indice;

        input >> indice;
        input >> R;
        input >> G;
        input >> B;
        cores.push_back(Ponto(R,G,B));
        //cout << R <<","<< G <<","<< B <<","<<endl;
    }

    input >> linha;
    input >> coluna;
    VetorModelos[NModelo].push_back(Ponto(linha,coluna,0));
    //cout << "aqui: ";Ponto(linha,coluna,0).imprime();
    cout << endl;
    for(int i =0; i<linha*coluna;i++){
        int cor;
        input >> cor;
        VetorModelos[NModelo].push_back(cores[cor-1]);
        //VetorModelos[NModelo][i+1].imprime();cout<<endl;
    }
}



//void drawLine(Point p1, Point p2) {
//    glBegin(GL_LINES);
//      glVertex2f(p1.x, p1.y);
//      glVertex2f(p2.x, p2.y);
//    glEnd();
//    glFlush();
//}



int factorial(int n)
{
    if (n<=1)
        return(1);
    else
        n=n*factorial(n-1);
    return n;
}

float binomial_coff(float n,float k)
{
    float ans;
    ans = factorial(n) / (factorial(k)*factorial(n-k));
    return ans;
}



Point drawBezierGeneralized(Point PT[], double t) {
    Point P;
    P.x = 0; P.y = 0;
    for (int i = 0; i<3; i++)
    {
        P.x = P.x + binomial_coff((float)(3 - 1), (float)i) * pow(t, (double)i) * pow((1 - t), (3 - 1 - i)) * PT[i].x;
        P.y = P.y + binomial_coff((float)(3 - 1), (float)i) * pow(t, (double)i) * pow((1 - t), (3 - 1 - i)) * PT[i].y;
    }

    return P;
}

Point** abc;

void bezier(double t, int i){
   if(t<=1) {
    //glColor3f(1.0,1.0,1.0);
    //Point p1 = abc[0];

        for(int k=0;k<3-1;k++){
            Point p2 = drawBezierGeneralized(abc[i],t);
            //cout<<p1.x<<"  ,  "<<p1.y<<endl;

            Universo[i+1].posicao.x = p2.x;
            Universo[i+1].posicao.y = p2.y;

            //cout<<p2.x<<"  ,  "<<p2.y<<endl;
            //cout<<endl;

            //drawLine(p1, p2);
            //p1 = p2;
        }
        Ponto v1 = Ponto(0,-1,0);
        Ponto v2 = Ponto(Universo[i+1].posicao.x - Universo[0].posicao.x,Universo[i+1].posicao.y - Universo[0].posicao.y);

        float anguloX = acos(((v1.x * v2.x) + (v1.y * v2.y)) / ((sqrt(pow(v1.x,2) + (pow(v1.y,2))) * sqrt(pow(v2.x,2) + (pow(v2.y,2))))));
        Universo[i+1].rotacao = anguloX;
    }
}


void CarregaInstancias()
{
    Universo[0].posicao = Ponto (-0,0);
    Universo[0].rotacao = 0;
    Universo[0].modelo = 0;
    int maior = 50;
    int menor = -50;

    for(int i =1; i <= qntInimigos; i++){
        int aleatoriox = rand()%(maior-menor+1) + menor;
        int aleatorioy = rand()%(maior-menor+1) + menor;

        Universo[i].posicao = Ponto(aleatoriox,aleatorioy);
        Universo[i].rotacao = 0;
        Universo[i].modelo = (i%4)+1;
    }

    nInstancias = 20;

}


void DesenhaCenario(){
    for(int i=0; i< nInstancias;i++){
        if(Universo[i].ativo == true){
            Universo[i].desenha();
        }
    }
}


void DesenhaRetangulo()
{
    glBegin(GL_QUADS);
        glVertex2d(-1, -1);
        glVertex2d(-1, 1);
        glVertex2d(1, 1);
        glVertex2d(1, -1);
    glEnd();
}




int maxTiros = 10;
tiro *tiros;
float velTiro = 1.5f;
int qntTiros = 0;

void atirar(){
    if(qntTiros < maxTiros){
        for(int i=0;i<maxTiros;i++){
            if(tiros[i].existe==false){
                tiro t;

                t.existe = true;

                t.x = Universo[0].posicao.x;
                t.y = Universo[0].posicao.y;
                //t.z = Universo[0].posicao.z;

                t.frente.x = VetorFrente.x;
                t.frente.y = VetorFrente.y;
                //t.frente.z = VetorFrente.z;

                t.angulo = Universo[0].rotacao;

                tiros[i] = t;

                qntTiros++;

                //cout << tiros.size() << endl;
                break;
            }
        }

    }
}

void desenhaTiros() {
    for(int i =0; i<maxTiros; i++) {
        if(tiros[i].existe) {
            glPushMatrix();
                glColor3f(1,1,0);
                glTranslatef(tiros[i].x,tiros[i].y,0);
                glRotatef(tiros[i].angulo, 0, 0, 1);

                DesenhaRetangulo();
                glTranslatef(0,2,0);
                DesenhaRetangulo();
            glPopMatrix();

            tiros[i].x -= tiros[i].frente.x*velTiro;
            tiros[i].y -= tiros[i].frente.y*velTiro;
            //tiros[i].z += tiros[i].frente.z*velTiro;

            //sair do cenario
            if(tiros[i].x > 50.0 || tiros[i].y > 50.0 || tiros[i].x < -50.0 || tiros[i].y < -50.0){
                tiros[i].existe = false;
                qntTiros--;
            }

            //colisao com inimigo
            for(int j=1; j <= nInstancias; j ++){
                if(tiros[i].x < (Universo[j].posicao.x + 5) &&
                   tiros[i].x > (Universo[j].posicao.x - 5) &&
                   tiros[i].y < (Universo[j].posicao.y + 5) &&
                   tiros[i].y > (Universo[j].posicao.y - 5) &&
                   Universo[j].ativo == true){
                    Universo[j].ativo = false;
                    tiros[i].existe = false;
                    qntTiros--;

                    qntAtInimigos--;
                }
            }


            //cout << tiros.size() << endl;
        }
    }
}

void init()
{
    //inicializa tiros
    tiros = new tiro[maxTiros];
    for(int i=0; i<maxTiros; i++){
        tiros[i].existe = false;
    }
    vida = 3;

    srand((unsigned)time(0));

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    CarregaModelo("j.txt",0);

    CarregaModelo("i1.txt",1);
    CarregaModelo("i2.txt",2);
    CarregaModelo("i3.txt",3);
    CarregaModelo("i4.txt",4);


//    CarregaModelos();
    CarregaInstancias();
    cout << "Vertices no Vetor: " << MeiaSeta.getNVertices() << endl;
    cout<<endl;
    cout<<endl;
    Min = Ponto(-50,-50);
    Max = Ponto(50,50);

}

void inimigoAtira(int i) {
    Ponto vetorDirecao = Ponto(Universo[0].posicao.x - Universo[i].posicao.x, Universo[0].posicao.y - Universo[i].posicao.y);
    double modulo = sqrt(pow(vetorDirecao.x,2) + (pow(vetorDirecao.y,2)));
    vetorDirecao = Ponto(vetorDirecao.x/modulo, vetorDirecao.y/modulo);

    tiro t;
    tirosInimigos[i].x = Universo[i].posicao.x;
    tirosInimigos[i].y = Universo[i].posicao.y;

    tirosInimigos[i].frente = Ponto(vetorDirecao.x*vel, vetorDirecao.y*vel);

    tirosInimigos[i].angulo = Universo[i].rotacao;

    tirosInimigos[i].existe = true;
}

void desenhaTirosInimigos() {
    //cout << "desenhando tiros" << endl;
    for(int i = 0; i<qntInimigos; i++) {
            //cout << "desenhando tiros" << endl;
        if(tirosInimigos[i].existe) {
                //cout << "desenhando tiros" << endl;
            //movimenta tiro
            tirosInimigos[i].x += tirosInimigos[i].frente.x;
            tirosInimigos[i].y += tirosInimigos[i].frente.y;
            //colide com parede
            if(tirosInimigos[i].x > 50.0 || tirosInimigos[i].y > 50.0 || tirosInimigos[i].x < -50.0 || tirosInimigos[i].y < -50.0){
                tirosInimigos[i].existe = false;
            }
            //colide com jogador
            if(tirosInimigos[i].x < (Universo[0].posicao.x + 6) &&
               tirosInimigos[i].x > (Universo[0].posicao.x - 6) &&
               tirosInimigos[i].y < (Universo[0].posicao.y + 5) &&
               tirosInimigos[i].y > (Universo[0].posicao.y - 5) &&
               Universo[0].ativo == true){
                vida--;
                tirosInimigos[i].existe = false;
            }
            //desenha
            glPushMatrix();
                glColor3f(1,0,0);
                glTranslatef(tirosInimigos[i].x,tirosInimigos[i].y,0);
                glRotatef(tirosInimigos[i].angulo, 0, 0, 1);

                DesenhaRetangulo();
            glPopMatrix();
        }
    }
}

double TempoCurva;
void animate()
{
    if(vidaAnt>vida) {
        cout << "Vida: " << vida << endl;
        vidaAnt = vida;
    }
    if(vida <= 0) {
        cout << "Derrota! Fim de Jogo!" << endl;
        glutLeaveMainLoop();
    }
    if(qntAtInimigos <= 0) {
        cout << "Vitoria! Fim de Jogo!" << endl;
        glutLeaveMainLoop();
    }

    double dt;
    dt = T.getDeltaT();
    AccumDeltaT += dt;
    TempoTotal += dt;
    TempoCurva += dt;

    //nFrames++;

    double auxT = TempoCurva/5;
    for(int i = 0; i < qntInimigos; i++) {
        bezier(auxT, i);
    }

    if (AccumDeltaT > 1.0/75) // fixa a atualiza‹o da tela em 30
    {
        AccumDeltaT = 0;
        angulo+=1;
        glutPostRedisplay();
    }
    if(TempoCurva > 5) {
        //gera novo ponto
        for(int i = 0; i<qntInimigos;i++) {
            abc[i][0] = {abc[i][2].x,abc[i][2].y};

            int maior = 50;
            int menor = -50;

            int aleatoriox2 = rand()%(maior-menor+1) + menor;
            int aleatorioy2 = rand()%(maior-menor+1) + menor;

            abc[i][1] = {aleatoriox2,aleatorioy2};

            int aleatoriox3 = rand()%(maior-menor+1) + menor;
            int aleatorioy3 = rand()%(maior-menor+1) + menor;

            abc[i][2] = {aleatoriox3,aleatorioy3};
        }
        TempoCurva = 0;
    }
    if(TempoTotal > 2) {
        for(int i = 0; i<qntInimigos; i++){
            if(Universo[i].ativo && !tirosInimigos[i].existe) {
                int atira = rand()%10;
                if(atira >=5) {
                    //cout << "inimigo " << i << " atirou" << endl;
                    inimigoAtira(i);
                }
            }
        }
        TempoTotal = 0;
    }

    //if (TempoTotal > 5.0)
    //{
        //cout << "Tempo Acumulado: "  << TempoTotal << " segundos. " ;
        //cout << "Nros de Frames sem desenho: " << nFrames << endl;
        //cout << "FPS(sem desenho): " << nFrames/TempoTotal << endl;
        //TempoTotal = 0;
        //nFrames = 0;
    //}
}




void reshape( int w, int h )
{
    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Define a area a ser ocupada pela area OpenGL dentro da Janela
    glViewport(0, 0, w, h);
    // Define os limites logicos da area OpenGL dentro da Janela
    glOrtho(Min.x,Max.x,
            Min.y,Max.y,
            -10,+10);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

#define LARG


void DesenhaPersonagem(int e)  // modelo - veio de arquivo
{
    glPushMatrix();
        int linhas,colunas;

        colunas = (int)VetorModelos[e][0].y;
        linhas = (int)VetorModelos[e][0].x;
        //for(int i =0; i<linhas*colunas;i++){
        //    cout << (float)VetorModelos[e][i+1].x/255.0f<< ","<<(float)VetorModelos[e][i+1].y/255.0f<< ","<<(float)VetorModelos[e][i+1].z/255.0f << endl;
        //}
        glTranslatef(-colunas -1,linhas-1,0);

        int aux=0;

        for(int i =0; i<linhas*colunas;i++){

            if(aux >= colunas){
                glTranslatef(-2*colunas,-2,0);
                aux = 0;
            }
               //if(VetorModelos[e][i+1].x != 0) {
               // if(VetorModelos[e][i+1].y != 0) {
                 //  if(VetorModelos[e][i+1].z != 0) {
                    glColor3f((float)VetorModelos[e][i+1].x/255.0f,(float)VetorModelos[e][i+1].y/255.0f,(float)VetorModelos[e][i+1].z/255.0f);
                 //   }

               // }
           // }
            glTranslatef(2,0,0);
            DesenhaRetangulo();
            aux++;

        }
    glPopMatrix();
}

double t;
void display( void )
{
    /*if(t>1) {
        t = 0;
    }
    double dt;
    dt = T.getDeltaT();
    t += dt;
    //frame++;
    //if(frame<=1) {
        //for(double t = 0.0;t <= 1.0; t += 0.0005){

        //}
    //}
    */

    if(vetorTeclas[0]){
        atirar();
    }
    if(vetorTeclas[1]){
        Universo[0].rotacao +=5;
        double auxX = VetorFrente.x;
        double auxY = VetorFrente.y;
        VetorFrente.x = auxX*cos(5*(PI/180.0f)) - auxY*sin(5*(PI/180.0f));
        VetorFrente.y = auxX*sin(5*(PI/180.0f)) + auxY*cos(5*(PI/180.0f));
    }
    if(vetorTeclas[2]){
        Universo[0].rotacao -=5;
        double auxX = VetorFrente.x;
        double auxY = VetorFrente.y;
        VetorFrente.x = auxX*cos(-5*(PI/180.0f)) - auxY*sin(-5*(PI/180.0f));
        VetorFrente.y = auxX*sin(-5*(PI/180.0f)) + auxY*cos(-5*(PI/180.0f));
    }

    /*if(vetorTeclas[4]){
        double auxX = Universo[0].posicao.x + VetorFrente.x;
        double auxY = Universo[0].posicao.y + VetorFrente.y;
        if(!(auxX > 50 || auxX < -50 || auxY > 50 || auxY < -50)){
            Universo[0].posicao.x = auxX;
            Universo[0].posicao.y = auxY;
        }
    }*/

    if(vetorTeclas[3]){
        double auxX = Universo[0].posicao.x - VetorFrente.x;
        double auxY = Universo[0].posicao.y - VetorFrente.y;
        if(!(auxX > 50 || auxX < -50 || auxY > 50 || auxY < -50)){
            Universo[0].posicao.x = auxX;
            Universo[0].posicao.y = auxY;
        }
    }

    //VetorFrente.imprime();
    //cout << __FUNCTION__ << endl;
	// Limpa a tela coma cor de fundo
	glClear(GL_COLOR_BUFFER_BIT);

    // Define os limites lógicos da área OpenGL dentro da Janela
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


	glLineWidth(1);
	glColor3f(1,1,1); // R, G, B  [0..1]
//    DesenhaEixos();

    glLineWidth(2);

    //DesenhaCatavento();

    DesenhaCenario();
    desenhaTiros();
    desenhaTirosInimigos();



	glutSwapBuffers();
}

void keyboardUp (unsigned char key, int x, int y )
{
    switch(key){
        case ' ':
            vetorTeclas[0] = false;
            break;
        default:
            break;
    }
}

void arrow_keysUp ( int a_keys, int x, int y )
{
	switch ( a_keys )
	{
        case GLUT_KEY_LEFT:
            vetorTeclas[1] = false;
            break;
        case GLUT_KEY_RIGHT:
            vetorTeclas[2] = false;
            break;
		case GLUT_KEY_UP:       // Se soltar UP
            vetorTeclas[3] = false;
			break;
	    case GLUT_KEY_DOWN:     // Se soltar DOWN
             vetorTeclas[4] = false;
			break;
		default:
			break;
	}
}

void keyboard ( unsigned char key, int x, int y )
{

	switch ( key )
	{
		case 27:        // Termina o programa qdo
			exit ( 0 );   // a tecla ESC for pressionada
			break;
        case ' ':
            vetorTeclas[0] = true;
        break;
		default:
			break;
	}
}

void arrow_keys ( int a_keys, int x, int y )
{
	switch ( a_keys )
	{
        case GLUT_KEY_LEFT:
            vetorTeclas[1] = true;
            break;
        case GLUT_KEY_RIGHT:
            vetorTeclas[2] = true;
            break;
		case GLUT_KEY_UP:       // Se pressionar UP
            vetorTeclas[3] = true;
			break;
	    case GLUT_KEY_DOWN:     // Se pressionar DOWN
             vetorTeclas[4] = true;
			break;
		default:
			break;
	}
}

int  main ( int argc, char** argv )
{
    vida = 3;
    vidaAnt = vida;

    qntAtInimigos = qntInimigos;

    tirosInimigos = new tiro[qntInimigos*3];
    for(int i = 0; i<qntInimigos; i++) {
        tirosInimigos[i].existe = false;
    }

    Universo = new Instancia[qntInimigos+1];

    abc = new Point*[qntInimigos];

	for(int i=0; i<qntInimigos; i++){
		abc[i]=new Point[3];

		int maior = 50;
        int menor = -50;

		int aleatoriox1 = rand()%(maior-menor+1) + menor;
        int aleatorioy1 = rand()%(maior-menor+1) + menor;

        int aleatoriox2 = rand()%(maior-menor+1) + menor;
        int aleatorioy2 = rand()%(maior-menor+1) + menor;

        int aleatoriox3 = rand()%(maior-menor+1) + menor;
        int aleatorioy3 = rand()%(maior-menor+1) + menor;

		abc[i][0] = {aleatoriox1,aleatorioy1};
		abc[i][1] = {aleatoriox2,aleatorioy2};
		abc[i][2] = {aleatoriox3,aleatorioy3};

	}



    cout << "Programa OpenGL" << endl;

    glutInit            ( &argc, argv );
    glutInitWindowPosition (0,0);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );

    glutInitWindowSize  ( 650, 500);

    glutCreateWindow    ( "Primeiro Programa em OpenGL" );


    //glutMouseFunc(bezier);
    init ();
    cout << "Vida: " << vida << endl;
    glutDisplayFunc ( display );

    glutIdleFunc(animate);

    glutReshapeFunc ( reshape );

    glutKeyboardFunc ( keyboard );
    glutKeyboardUpFunc ( keyboardUp );

    glutSpecialFunc ( arrow_keys );
    glutSpecialUpFunc (arrow_keysUp);

    glutMainLoop ( );

    return 0;
}
