#ifndef texture_H
#define texture_H

#include "common.h"

// klasa implementujaca obsluge obiektu tekstury w OpenGL
class glTexture
{

public:

glTexture(char *FileName); // domyslny konstruktor 
~glTexture(); // domyslny destruktor
GLuint Bind(); // ustawia aktualny obiekt tekstury 
void swapFilter();

private: 
void LoadBMP(char *filename,int &_width, int &_height, char **pixels);
UINT TextureId; // id tekstury 
UINT SamplerId; // id samplera
bool trigger;

char _msg[1024];
};


#endif