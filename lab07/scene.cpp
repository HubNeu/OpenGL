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

//--------------------------------------------------------------------------------------------
Scene::Scene(int new_width,int new_height)
{	
	width = new_width;
	height = new_height;	
	rot_x = 0.0;
	rot_y = 0.0;
	Axes = NULL;		
	LightAmbient = 0.8;	
	Cam_angle = 0.0;
	Cam_r = 5.0;
	err = 1;	
}
//--------------------------------------------------------------------------------------------
// Domyslny destruktor 
Scene::~Scene()
{
	// usun program przetwarzania 
	if (glIsProgram(program)) glDeleteProgram(program);
	if (Axes) delete Axes;	
}
//--------------------------------------------------------------------------------------------
// przygotowuje programy cienionwania 
void Scene::PreparePrograms()
{
	err = 0; // ustaw flage bledu
	program  = glCreateProgram();
	if (!glIsProgram(program)) {err = 1; ThrowException("Nie udalo sie utworzyc programu");}
	
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
        // pobranie i wy�wietlenie komunikatu b��du 
        GLint logLength; 
        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logLength ); 
        char *log = new char[logLength]; 
        glGetProgramInfoLog( program, logLength, NULL, log ); 
        PrintLog(log);
        delete[] log; 
		err = 1;
		ThrowException("Blad linkowania programu");
    }
	else
		PrintLog("Program zlinkowany");

	// walidowanie programu 
	glValidateProgram(program); 
	GLint validate_status;
    // sprawdzenie poprawno�ci walidacji obiektu programu 
    glGetProgramiv( program, GL_VALIDATE_STATUS, &validate_status ); 
    if( validate_status == GL_FALSE ) 
    { 
        // pobranie i wy�wietlenie komunikatu b��du 
        GLint logLength; 
        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logLength ); 
        char *log = new char[logLength]; 
        glGetProgramInfoLog( program, logLength, NULL, log ); 
        PrintLog(log);
        delete[] log; 
		err = 1;
        ThrowException("Blad walidacji programu");
    } 
	else
		PrintLog("Program prawidlowy");
	
	glUseProgram(program);
}
//--------------------------------------------------------------------------------------------
void Scene::PrepareObjects()
{  
	Axes = new glObject();	
	Moon = new glSphere(1,"textures\\moon.bmp");
	Prn = new glPrinter("tahoma.ttf", 24);

	Axes->BeginObject(GL_LINES);
	Axes->SetColor(1.0,0.0,0.0); // os X w kolorze czerwonym
	Axes->AddVertex(0.0,0.0,0.0);
	Axes->AddVertex(10.0,0.0,0.0);
	Axes->SetColor(0.0,1.0,0.0); // os Y w kolorze zielonym 
	Axes->AddVertex(0.0,0.0,0.0);
	Axes->AddVertex(0.0,10.0,0.0);
	Axes->SetColor(0.0,0.0,1.0); // os Z w kolorze niebieskim 
	Axes->AddVertex(0.0,0.0,0.0);
	Axes->AddVertex(0.0,0.0,10.0);
	Axes->EndObject();

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
	
	mProjection = glm::perspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
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
	// inicjalizacja modu�u glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		sprintf(_msg, "GLew error: %s\n", glewGetErrorString(err));	
		err = 1;
		ThrowException(_msg);
	}
	
	// pobierz informacje o wersji openGL 
	sprintf(_msg,"OpenGL vendor: ");
	strcat(_msg,(const char*)glGetString( GL_VENDOR ));
	PrintLog(_msg);

	sprintf(_msg,"OpenGL renderer: ");
	strcat(_msg,(const char*)glGetString( GL_RENDERER));
	PrintLog(_msg);

	sprintf(_msg,"OpenGL version: ");
	strcat(_msg,(const char*)glGetString( GL_VERSION));
	PrintLog(_msg);

	//  ustaw kolor tla sceny (RGB Z=1.0)
	glClearColor(0.0f, 0.0f, 0.3f, 0.0f);
	
	// przygotuj programy shaderow
	PreparePrograms();
	
	if (err) return; 

	// przygotuj obiekty do wyswietlenia 
	PrepareObjects();
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glEnable(GL_TEXTURE_2D);
}
//--------------------------------------------------------------------------------------------
// przeprowadza animacje sceny 
void Scene::Animate()
{
	rot_y +=5.0;
}
//--------------------------------------------------------------------------------------------
// kontrola naciskania klawiszy klawiatury
void Scene::KeyPressed(unsigned char key, int x, int y) 
{            
	if (key == ESCAPE) ThrowException("Zatrzymaj program");
	switch(key)
	{
		case 37: {rot_y -= 5.0f; break;}
		case 38: {rot_x -= 5.0f; break;}
		case 39: {rot_y += 5.0f; break;}
		case 40: {rot_x += 5.0f; break;}		
		case 112: {LightAmbient += 0.1f; break;} // F1		
		case 113: {LightAmbient -= 0.1f; break;} //F2		
		
		case 114: { break;} //F3		
		case 115: { break;} //F4		

	    case 116: { break;} //F5		
		case 117: { break;} //F6		
		
		case 87: {Cam_r -= 0.5f; break;} //W
		case 83: {Cam_r += 0.5f; break;} //S		
		case 65: {Cam_angle -= 5.0f; break;} //A
		case 68: {Cam_angle += 5.0f; break;} //D

		case 32: {
					SaveAsBmp("zdjecie");//spacja
					break;
				 }
	}
	
}
//--------------------------------------------------------------------------------------------
void Scene::SaveAsBmp(char *filename)
{	
	int _width = this->width;      // bitmap height 
	int _height = this->height;    // bitmap width
	while (_width*3 % 4) _width--;  // adjust bmp width to meet 4B row padding rule
	int img_size = _width*_height*3; // image size (each pixel is coded by 3Bytes)

	int storage_4B; // four byte storage used for saving integer bytes into bmp file 	
	FILE *fil; // prepare file handle
	fopen_s(&fil,filename,"wb"); // open the file in binary mode
	rewind(fil);
	//*********************** START OF THE HEADER **********************************
	// BM signature
	storage_4B = 0x00004d42;
	fwrite((char *)&storage_4B,1,2,fil);
	// file size
	storage_4B = 54 + img_size;
	fwrite((char *)&storage_4B,1,4,fil);
	// four empty bytes 
	storage_4B = 0;
	fwrite((char *)&storage_4B,1,4,fil);
	// pixeltable address
	// od ktorego bitu zaczyna sie obraz (14 + 40)
	storage_4B = 54; fwrite((char *)&storage_4B, 1, 4, fil);

	//rozmiar naglowka (tego drugiego, dluzszego = 40)
	storage_4B = 40; fwrite((char *)&storage_4B, 1, 4, fil);

	//szerokosc 
	storage_4B = _width; fwrite((char *)&storage_4B, 1, 4, fil);

	//dlugosc
	storage_4B = _height; fwrite((char *)&storage_4B, 1, 4, fil);

	//liczba warstw (1)
	storage_4B = 1; fwrite((char *)&storage_4B, 1, 2, fil);

	//bitow na piksel (1,2,4,8,16,24)
	storage_4B = 24; fwrite((char *)&storage_4B, 1, 2, fil);

	//komopresja (0 = bez kompresji)
	storage_4B = 0; fwrite((char *)&storage_4B, 1, 4, fil);

	//rozmiar rysunku (0 jesli bez kompresji)
	storage_4B = 0; fwrite((char *)&storage_4B, 1, 4, fil);

	//rozdzielczosc pozioma px/metr
	storage_4B = 2835; fwrite((char *)&storage_4B, 1, 4, fil);

	//rozdzielczosc pionowa px/metr
	storage_4B = 2835; fwrite((char *)&storage_4B, 1, 4, fil);

	//liczba kolorow w palecie 2^24 = 16M
	storage_4B = 16000000; fwrite((char *)&storage_4B, 1, 4, fil);

	//liczba waznych kolorow 0=wszystkie wazne
	storage_4B = 0; fwrite((char *)&storage_4B, 1, 4, fil);

	//*********************** END OF THE HEADER **********************************

	unsigned char *pixels; // room for pixeltable
	pixels = (unsigned char *)malloc(img_size * sizeof(unsigned char));
	// read pixels from colorbuffer
	glReadPixels(0, 0, _width, _height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	// store pixels in the file
	fwrite(pixels, 1, img_size, fil);
	fflush(fil); // clear file cache
	fclose(fil); // close the file
	free(pixels); // release memory

	
}
//--------------------------------------------------------------------------------------------
// rysuje scene OpenGL 
void Scene::Draw()
{	
	if (err) return; // sprawdz flage bledu (np. kompilacja shadera)

	// czyscimy bufor kolorow
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	int _ModelView = glGetUniformLocation(program, "modelViewMatrix");
	int _Projection = glGetUniformLocation(program, "projectionMatrix");
	int _LightDirection = glGetUniformLocation(program, "LightDirection");
	int _LightColor = glGetUniformLocation(program, "LightColor");
	int _LightAmbient = glGetUniformLocation(program, "LightAmbient");
	int _NormalMatrix = glGetUniformLocation(program, "normalMatrix");
	int _Sampler = glGetUniformLocation(program, "gSampler");
	int _ShadingMode = glGetUniformLocation(program, "ShadingMode");
	int _lightModel = glGetUniformLocation(program, "lightModel");

	// ustaw sampler tekstury 
	glUniform1i(_Sampler, 0);

	// ustaw macierz projekcji na perspektywiczna
	glUniformMatrix4fv(_Projection, 1, GL_FALSE, glm::value_ptr(mProjection));
	
	// inicjuj macierz MV z polozeniem obserwatora
	glm::mat4 mModelView = glm::lookAt(glm::vec3(Cam_r*cos(PI*Cam_angle/180.0), 0.0f, Cam_r*sin(PI*Cam_angle/180.0)), 
										glm::vec3(0.0f),
										glm::vec3(0.0f, 1.0f, 0.0f));
	// wyslij MV do shadera
	glUniformMatrix4fv(_ModelView, 1, GL_FALSE, glm::value_ptr(mModelView));

	
	// ustaw i wyslij kierunek swiatla
	glm::vec3 LightDirection = glm::vec3(0.0,-1.0,0.0); 
	glUniform3fv(_LightDirection,1,glm::value_ptr(LightDirection));

	// ustaw i wyslij kolor swiatla 
	glm::vec3 LightColor = glm::vec3(1.0,1.0,1.0);
	glUniform3fv(_LightColor,1,glm::value_ptr(LightColor));
	
	// wyslij parametr swiatla ambient
	glUniform1f(_LightAmbient,LightAmbient);	

	// wyslij macierz normlanych 	
	glUniformMatrix4fv(_NormalMatrix, 1, GL_FALSE, 
			glm::value_ptr(glm::transpose(glm::inverse(mModelView))));	
		
	//------------------------------------------------------------------------------------------------------
	// Rysowanie w trybie perspektywicznym 
	//------------------------------------------------------------------------------------------------------

	glUniform1i(_ShadingMode, 1);
	glUniform1f(_lightModel, false);
	Axes->Draw();
	glUniform1f(_lightModel, true);
		
	glm::mat4 mTransform = glm::mat4(1.0);
	mTransform = glm::rotate(glm::mat4(1.0), rot_x, glm::vec3(1.0f, 0.0f, 0.0f));
	mTransform = glm::rotate(mTransform, rot_y, glm::vec3(0.0f, 1.0f, 0.0f));
	
	glUniformMatrix4fv(_NormalMatrix, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(mTransform))));	

	glUniformMatrix4fv(_ModelView, 1, GL_FALSE, glm::value_ptr(mModelView*mTransform));
		
	glUniform1i(_ShadingMode, 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	Moon->Draw();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glUniform1i(_ShadingMode, 1);
	//napis rzutowany
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDisable(GL_DEPTH_TEST);
	
	mTransform = glm::translate(mTransform, glm::vec3(-0.8f, -1.5f, 0.0f));
	glUniformMatrix4fv(_ModelView, 1, GL_FALSE, glm::value_ptr(mModelView*mTransform));
	mTransform = glm::scale(mTransform, glm::vec3(0.5 / float(Prn->CharWidth), 0.5 / float(Prn->CharHeight), 1.0));
	glUniformMatrix4fv(_NormalMatrix, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(mTransform))));
	glUniformMatrix4fv(_ModelView, 1, GL_FALSE, glm::value_ptr(mModelView*mTransform));
	std::string obrot = std::to_string(rot_y);
	for (int i = 0; i < 6; i++)
	{
		Prn->Draw(int(obrot[i]));
		mTransform = glm::translate(mTransform, glm::vec3(18, 0, 0));
		glUniformMatrix4fv(_ModelView, 1, GL_FALSE, glm::value_ptr(mModelView*mTransform));
	}
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	//koniec napisu rzutowanego

	mTransform = glm::scale(mTransform, glm::vec3(0.5 / float(Prn->CharWidth), 0.5 / float(Prn->CharHeight), 1.0));
	glUniformMatrix4fv(_NormalMatrix, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(mTransform))));
	glUniformMatrix4fv(_ModelView, 1, GL_FALSE,glm::value_ptr(mModelView*mTransform));
	glUniform1i(_ShadingMode, 1);
	
	//--------------------------------------------------
	// Rysowanie w trybie ortogonalnym
	//--------------------------------------------------

	glm::mat4 mOrto = glm::ortho(0.0f,float(width),0.0f,float(height));
	mModelView = glm::mat4(1.0);
	mTransform = glm::mat4(1.0);
	// ustaw macierz projekcji na ortogonalna
	glUniformMatrix4fv(_Projection, 1, GL_FALSE,glm::value_ptr(mOrto));
	// ustaw przeksztalcenia macierzowe
	glUniformMatrix4fv(_ModelView, 1, GL_FALSE,glm::value_ptr(mModelView*mTransform));
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDisable(GL_DEPTH_TEST);
    //std::string obrot = std::to_string(rot_y);
	for (int i = 0; i < 6; i++)
	{
		Prn->Draw(int(obrot[i]));
		mTransform = glm::translate(mTransform, glm::vec3(18, 0, 0));
		glUniformMatrix4fv(_ModelView, 1, GL_FALSE, glm::value_ptr(mModelView*mTransform));
	}
	
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}	
//------------------------------- KONIEC PLIKU -----------------------------------------------