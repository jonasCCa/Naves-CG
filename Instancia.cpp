//
//  Instancia.cpp
//  OpenGLTest
//
//  Created by Márcio Sarroglia Pinho on 22/09/20.
//  Copyright © 2020 Márcio Sarroglia Pinho. All rights reserved.
//

#include "Instancia.h"


void DesenhaPersonagem(int modelo);
void DesenhaRetangulo();


Instancia::Instancia(){
    this->ativo = true;
}
// ***********************************************************
//  void InstanciaPonto(Ponto3D *p, Ponto3D *out)
//  Esta funcao calcula as coordenadas de um ponto no
//  sistema de referencia do universo (SRU), ou seja,
//  aplica as rotacoes, escalas e translacoes a um
//  ponto no sistema de referencia do objeto (SRO).
// ***********************************************************
void InstanciaPonto(Ponto &p, Ponto &out)
{
    GLfloat ponto_novo[4];
    GLfloat matriz_gl[4][4];
    int  i;

    glGetFloatv(GL_MODELVIEW_MATRIX,&matriz_gl[0][0]);

    for(i=0;i<4;i++)
    {
        ponto_novo[i]= matriz_gl[0][i] * p.x+
        matriz_gl[1][i] * p.y+
        matriz_gl[2][i] * p.z+
        matriz_gl[3][i];
    }
    out.x=ponto_novo[0];
    out.y=ponto_novo[1];
    out.z=ponto_novo[2];
}

Ponto InstanciaPonto(Ponto P)
{
    Ponto temp;
    InstanciaPonto(P, temp);
    return temp;
}

void Instancia::desenha()
{
    // aplica as transformacoes geometricas no modelo
    // desenha a geometria do objeto

    glPushMatrix();
    glTranslatef(posicao.x, posicao.y, 0);
    glRotatef(rotacao, 0, 0, 1);
    Ponto PosicaoDoPersonagem;
    Ponto Origem (0,0,0);
    InstanciaPonto(Origem, PosicaoDoPersonagem);
    //PosicaoDoPersonagem.imprime(); cout << endl;

    DesenhaPersonagem(modelo);
    glPopMatrix();
}

