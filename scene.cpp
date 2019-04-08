//--------------------------------------------------------------------------------------------
//
// File: 	scene.cpp
// Author:	P. Katarzynski (CCE)
//
// Description: Implementacja klasy sceny OpenGL
//
//--------------------------------------------------------------------------------------------
//									ZALEZNOSI 
//--------------------------------------------------------------------------------------------

#include "scene.h"

//--------------------------------------------------------------------------------------------
// zglasza wyjatek z komunikatem do debuggowania 

int tmpIloscKatow = 4;//ustalanie ilosci katow, uwaga na memleak
//--------------------------------------------------------------------------------------------
Scene::Scene(int new_width,int new_height)
{	
	width = new_width;
	height = new_height;	
}
//--------------------------------------------------------------------------------------------
// Domyslny destruktor 
Scene::~Scene()
{
	// usun program przetwarzania 
	if (glIsProgram(program)) glDeleteProgram(program);
	
	// usun bufory 
	glDeleteBuffers(VBO_cnt,VBOs); 

	// usun tablice atrybutow wierzcholkow 
	glDeleteVertexArrays(VAO_cnt,VAOs);
}
//--------------------------------------------------------------------------------------------
// przygotowuje programy cienionwania 
void Scene::PreparePrograms()
{
	program  = glCreateProgram();
	if (!glIsProgram(program)) ThrowException("Nie udalo sie utworzyc programu");
	
	vertex_shader = LoadShader(GL_VERTEX_SHADER,"vs.glsl");
	glAttachShader(program,vertex_shader);

	fragment_shader = LoadShader(GL_FRAGMENT_SHADER,"fs.glsl");
	glAttachShader(program,fragment_shader);
	
	// linkowanie programu 
	glLinkProgram(program);

	GLint link_status; 
    glGetProgramiv( program, GL_LINK_STATUS, &link_status); 
    if( link_status == GL_FALSE ) 
    { 
        // pobranie i wyœwietlenie komunikatu b³êdu 
        GLint logLength; 
        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logLength ); 
        char *log = new char[logLength]; 
        glGetProgramInfoLog( program, logLength, NULL, log ); 
        PrintLog(log);
        delete[] log; 
		ThrowException("Blad linkowania programu");
    }
	else
		PrintLog("Program zlinkowany");

	// walidowanie programu 
	glValidateProgram(program); 
	GLint validate_status;
    // sprawdzenie poprawnoœci walidacji obiektu programu 
    glGetProgramiv( program, GL_VALIDATE_STATUS, &validate_status ); 
    if( validate_status == GL_FALSE ) 
    { 
        // pobranie i wyœwietlenie komunikatu b³êdu 
        GLint logLength; 
        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logLength ); 
        char *log = new char[logLength]; 
        glGetProgramInfoLog( program, logLength, NULL, log ); 
        PrintLog(log);
        delete[] log; 
        ThrowException("Blad walidacji programu");
    } 
	else
		PrintLog("Program prawidlowy");
	
	glUseProgram(program);
}
//--------------------------------------------------------------------------------------------
// przygotowuje obiekty do wyswietlenia 
void Scene::PrepareObjects()
{  
	// przygotuj tablice VAO
	glGenVertexArrays(VAO_cnt, VAOs);
	// przygotuj bufory VBO
	glGenBuffers(VBO_cnt, VBOs);
	float vtab[9]; // tablica 12 wierzcholkow
				   // wypelnij wspolrzednymi (x,y,z)
	vtab[0] = -0.5f; vtab[1] = -0.5f; vtab[2] = 0.0f;
	vtab[3] = 0.0f; vtab[4] = 0.5f; vtab[5] = 0.0f;
	vtab[6] = 0.5f; vtab[7] = -0.5f; vtab[8] = 0.0f;
	// podlacz pierwszy obiekt z VAOs
	glBindVertexArray(VAOs[0]);
	// podlacz pierwszy bufor VBOs
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
	// wypelnij bufor wspolrzednymi wierzcholka
	glBufferData(GL_ARRAY_BUFFER, sizeof(vtab), vtab, GL_STATIC_DRAW);
	// wybierz atrybut indeksie 0 (wskazany w shaderze)
	glEnableVertexAttribArray(0);
	// powiaz dane z bufora ze wskazanym atrybutem
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//ilosc katow definiuje sie w globalnej deklaracji
	Polygon(tmpIloscKatow, 0.5, VAOs[1], VBOs[1]);
	//Epicycloid(1000, 0.2, 0.35, VAOs[2], VBOs[2]);//przygotowanie epicykloidy; ma bug w postaci lini ktorej nie powinno byc
	//Epicycloid2(1000, 0.2, 0.35);//przygotowanie epicykloidy2//prostrza, bez buga, nwm czy poprawniejsza
}
//--------------------------------------------------------------------------------------------
// Odpowiada za skalowanie sceny przy zmianach rozmiaru okna
void Scene::Resize(int new_width, int new_height)
{
	// przypisz nowe gabaryty do pol klasy 
	width = new_width;
	// uwzgledniaj obecnosc kontrolki wizualnej 
	height = new_height-100; 	
  	// rozszerz obszar renderowania do obszaru o wymiarach 'width' x 'height'
	glViewport(0, 100, width, height);	
}
//--------------------------------------------------------------------------------------------
// laduje program shadera z zewnetrznego pliku 
GLuint Scene::LoadShader(GLenum type,const char *file_name)
{  
  // zmienna plikowa 
  FILE *fil = NULL; 
  // sproboj otworzyc plik 
  fil = fopen(file_name,"rb");
  // sprawdz, czy plik sie otworzyl  
  sprintf(_msg,"Nie mozna otworzyc %s",file_name);
  if (fil == NULL)  ThrowException(_msg);

  // okresl rozmiar pliku
  fseek( fil, 0, SEEK_END );
  long int file_size = ftell(fil); 
  // przewin na poczatek 
  rewind(fil); 
  // utworzenie bufora na kod zrodlowy programu shadera
  GLchar *srcBuf = new GLchar[(file_size+1)*sizeof(GLchar)];

  // przeczytanie kodu shadera z pliku 
  fread(srcBuf,1,file_size,fil);

  // zamknij plik 
  fclose(fil);

  // tekst programu MUSI miec NULL na koncu
  srcBuf[file_size] = 0x00;

  // utworzenie obiektu shadera
  GLuint shader = glCreateShader(type);

  // przypisanie zrodla do shadera 
  glShaderSource(shader,1,const_cast<const GLchar**>(&srcBuf),NULL);

  // sprzatanie 
  delete[] srcBuf;

  // proba skompilowania programu shadera 
  glCompileShader(shader);

  // sprawdzenie czy sie udalo 
  GLint compile_status;
  glGetShaderiv(shader,GL_COMPILE_STATUS,&compile_status);

  if (compile_status != GL_TRUE) // nie udalo sie 
  {
	    GLint logLength; 
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLength ); 
        char *log = new char[logLength]; 
        glGetShaderInfoLog( shader, logLength, NULL, log ); 
		sprintf(_msg,"Blad kompilacji pliku shadera %s",file_name);
		PrintLog(_msg);
        PrintLog(log);
		ThrowException("Blad kompilacji shadera");
	    delete []log;
  }
  else
  {
	  sprintf(_msg,"Plik shadera %s skompilowany",file_name);
	  PrintLog(_msg);
  }
  
  return shader; // zwroc id shadera 
}
//--------------------------------------------------------------------------------------------
// inicjuje proces renderowania OpenGL
void Scene::Init()
{
	// inicjalizacja modulu glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		sprintf(_msg, "GLew error: %s\n", glewGetErrorString(err));
		ThrowException(_msg);
	}
	// pobierz informacje o wersji openGL
	sprintf(_msg, "OpenGL vendor: ");
	strcat(_msg, (const char*)glGetString(GL_VENDOR));
	PrintLog(_msg);
	sprintf(_msg, "OpenGL renderer: ");
	strcat(_msg, (const char*)glGetString(GL_RENDERER));
	PrintLog(_msg);
	sprintf(_msg, "OpenGL version: ");
	strcat(_msg, (const char*)glGetString(GL_VERSION));
	PrintLog(_msg);
	// ustaw kolor tla sceny (RGB Z=1.0)
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// przygotuj programy shaderow
	PreparePrograms();
	// przygotuj obiekty do wyswietlenia
	PrepareObjects();
	
	
}
//--------------------------------------------------------------------------------------------
// kontrola naciskania klawiszy klawiatury
void Scene::KeyPressed(unsigned char key, int x, int y) 
{            
	if (key == ESCAPE) ThrowException("Zatrzymaj program");
}
//--------------------------------------------------------------------------------------------
// generuje zbior wspolrzednych wielokata foremnego 
// r - promien okregu opisanego na wielokacie 
// n - liczba katow 
// VAO identyfikator VAO 
// VBO identyfikator VBO
void Scene::Polygon(int n, float r,GLuint VAO, GLuint VBO)
{
	if (n<3) ThrowException("Wielokat: nieprawidlowy parametr");
	
	// TODO: zmodyfikuj pomocnicze zmienne 
	float kat = 360.0/n;
	float dkat = 3.1415*2/n; 

	float *coords=NULL; // tablica ze wspolrzednymi (x,y,z) wierzcholkow 
	
	coords = new float[3 * n];

	for (int i = 0; i < n; i++)
	{
		//TODO: tutaj wpisz kod generujacy wspolrzedne wierzcholkow
		coords[3 * i] = r*cos(i*dkat);
		coords[3 * i+1] = r*sin(i*dkat);
		coords[3 * i + 2] = 0;
	}
	
	// podlacz obiekt z VAO
	glBindVertexArray(VAO);
	
	// podlacz bufor VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	
	// wypelnij bufor wspolrzednymi wierzcholka 
	glBufferData(GL_ARRAY_BUFFER, n*3*sizeof(float), coords, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0); 
	// powiaz dane z bufora ze wskazanym atrybutem
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);	

	delete []coords;
}
//--------------------------------------------------------------------------------------------
void Scene::Epicycloid(int n, float R, float r, GLuint VAO, GLuint VBO)
{	
	float deg = 3.1415 / 36;
	float *coords = new float[3 * n];
	for (int i = 0; i < n; i++) {
		coords[3 * i + 0] = (R + r)*cos(deg*i) - r*cos(((R + r) / (r))*deg*i);
		coords[3 * i + 1] = (R + r)*sin(deg*i) - r*sin(((R + r) / (r))*deg*i);
		coords[3 * i + 2] = 0.0;
	}
	glBindVertexArray(VAO);//bindowanie do z VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);//bindowanie z VBO
	glBufferData(GL_ARRAY_BUFFER, n * 3 * sizeof(float), coords, GL_STATIC_DRAW);//wypelnienie buforu wspolrzednymi
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);//powiaz dane razem
	delete[]coords;//grbcll
}
void Scene::Epicycloid2(int n, float R, float r)
{
	float deg = 3.1415 / 36;
	float inc = 5 * 2 * 3.1415 / 180;
	glBegin(GL_LINE_STRIP);
	glColor4f(0, 0, 1, 1);//to zdaje sie nie dzialac, linia jest zawsze czarna
	for (int i = 0; i < n; i++) {
		float x = 0;
		float y = 0;
		x= (R + r)*cos(deg) - r*cos(((R + r) / (r))*deg);
		y= (R + r)*sin(deg) - r*sin(((R + r) / (r))*deg);
		glVertex2d(x, y);
		deg += inc;
	}
	glEnd();
}
//--------------------------------------------------------------------------------------------

// rysuje scene OpenGL 
void Scene::Draw()
{
	// czyscimy bufor kolorow
	glClear(GL_COLOR_BUFFER_BIT);
	// wybierz obiekt identyfikowany przez VAO
	glBindVertexArray(VAOs[0]);
	// narysuj go
	glDrawArrays(GL_LINE_LOOP, 0, 3);
	
	glBindVertexArray(VAOs[1]);
	glDrawArrays(GL_LINE_LOOP, 0, tmpIloscKatow);
	
	glBindVertexArray(VAOs[2]);//bindowanie
	//glDrawArrays(GL_LINE_LOOP, 0, 1000);//draw epicycloid
	
}
//------------------------------- KONIEC PLIKU -----------------------------------------------