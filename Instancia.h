//
//  Instancia.hpp
//  OpenGLTest
//
//  Created by Márcio Sarroglia Pinho on 22/09/20.
//  Copyright © 2020 Márcio Sarroglia Pinho. All rights reserved.
//


#ifndef Instancia_hpp
#define Instancia_hpp
#include <iostream>
using namespace std;

#include "Poligono.h"

class Instancia{
    //Poligono *modelo;

public:
    Instancia();
    Ponto posicao, escala;
    float rotacao;
    int modelo;
    float ativo;
    void desenha();
};


#endif /* Instancia_hpp */
