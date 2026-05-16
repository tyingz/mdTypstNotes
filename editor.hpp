#include "reader.hpp"

enum MODE
{
    Insert,
    Normal,
    Command,
};

class Editor
{
private:
    float FONT_SIZE  = 20;

    std::vector<std::string> buffer{};
    float y_min{};
    float x_min{};

    float y_actual{y_min};
    float x_actual{x_min};

    float y_max{HEIGHT_SCREEN/FONT_SIZE-1-2}; //idem abajo
    float x_max{WIDTH_SCREEN/FONT_SIZE}; //chequear si va float

    int letra{};
    int tecla{};
    MODE mode{Normal};

    Texture2D textura {};
    Rectangle areaRecorte {};
    Vector2 posicionDestino = { 20, 20 };

    Font jetbrainsFont; 

    Reader read{};
    std::vector<TypstBlock> bloques{};
    std::unordered_map<size_t,Texture2D> texturaGuardada{};
    std::unordered_set<int> setInsertRareKeys
    {
        KEY_ESCAPE,
        KEY_ENTER,
        KEY_BACKSPACE,
    };

    std::string bufferCommand{};

    std::string fileName{};
    std::ifstream file{};
    bool salir{false};

    bool letraJ {false};

public:
    Editor(std::string file_name)
    :fileName{file_name}
    {}

    void renderCursor()
    {
        Vector2 sizeText = MeasureTextEx(jetbrainsFont, "A", FONT_SIZE, 0);
        if (mode == MODE::Normal)
        {
            DrawRectangle(x_actual * sizeText.x, (y_actual-y_min)* sizeText.y, sizeText.x, sizeText.y, Fade(CURSOR_COLOR, 0.5f));
        }
        else if (mode == MODE::Insert)
        {
            DrawRectangle(x_actual * sizeText.x, (y_actual-y_min)* sizeText.y, sizeText.x/10, sizeText.y, Fade(CURSOR_COLOR, 0.5f));
        }
    }

    void renderEcuaciones()
    {
        for (auto &bloque : bloques)
        {
            bool cursorEncima = (y_actual >= bloque.inicioFila && y_actual <= bloque.finalFila);
            if (texturaGuardada.count(bloque.id) && !cursorEncima)
            {
                //al final la mejor opcion al menos por ahora es renderizar un cuadrado abajo
                DrawRectangle(x_min*FONT_SIZE,(bloque.inicioFila-y_min)*FONT_SIZE,bloque.areaEcuation.width,bloque.areaEcuation.height+FONT_SIZE,BLACK);
                Texture2D texturaEcuacion=texturaGuardada[bloque.id];
                if (CENTER_EQUATION == true)
                {
                    DrawTexture(texturaEcuacion,(x_max-x_min)*FONT_SIZE/2-(float)texturaEcuacion.width/2,(bloque.inicioFila-y_min)*FONT_SIZE,WHITE);
                }
                else
                {
                    DrawTexture(texturaEcuacion,x_min*FONT_SIZE,(bloque.inicioFila-y_min)*FONT_SIZE,WHITE);
                }
            }
        }
    }

    void renderScreen()
    {
        BeginDrawing();
        ClearBackground(BLACK);
        float distanciaEntreFilas{};
        for (size_t fila{};fila<=y_max;fila++)
        {
            if (fila+y_min<buffer.size())
            {
                Vector2 posicion = { 0, distanciaEntreFilas };
                DrawTextEx(jetbrainsFont,buffer[fila+y_min].c_str(), posicion, FONT_SIZE,0, WHITE);
                distanciaEntreFilas+=FONT_SIZE;
            }
        }
        if (mode == MODE::Normal)
        {
            renderEcuaciones();
        }
        renderStatusBar();
        renderCursor();
        EndDrawing();
    }

    void renderStatusBar()
    {
        Vector2 sizeText = MeasureTextEx(jetbrainsFont, "A", FONT_SIZE, 0);
        DrawRectangle(x_min*sizeText.x, (y_max+1)* sizeText.y, WIDTH_SCREEN, HEIGHT_SCREEN, BLACK);
        DrawRectangle(x_min*sizeText.x, (y_max+2)* sizeText.y, WIDTH_SCREEN, HEIGHT_SCREEN, BROWN);
        Vector2 posicion = { x_min*sizeText.x, (y_max+2)* sizeText.y };
        if (mode==MODE::Normal)
        {
            DrawTextEx(jetbrainsFont,"--NORMAL--",posicion,FONT_SIZE,0,WHITE);
        }
        else if (mode==MODE::Insert)
        {
            DrawTextEx(jetbrainsFont,"--INSERT--",posicion,FONT_SIZE,0,WHITE);
        }
        Vector2 posicionText = { x_min*sizeText.x, (y_max+1)* sizeText.y };
        DrawTextEx(jetbrainsFont,bufferCommand.c_str(),posicionText,FONT_SIZE,0,WHITE);
    }

    void actualizarTexturas()
    {
        bloques = read.parser(buffer,y_min,y_max,x_min,x_max,FONT_SIZE);
        for (auto &val : bloques)
        {
            std::string locationImage{};
            locationImage.append("./temp/formula");
            locationImage.append(std::to_string(val.id));
            locationImage.append(".png");

            val.flag = true;
            if (texturaGuardada.find(val.id)==texturaGuardada.end())
            {
                if (fs::exists(locationImage))
                {
                    Image img = LoadImage(locationImage.c_str());
                    texturaGuardada[val.id] = LoadTextureFromImage(img);
                    UnloadImage(img); 
                    //solamente si no existe en el mapa pero si en mi pc
                }
            }
        }
        //guardo las texturas en el mapa y ahora cuando entre en normal mode
        //tengo que renderizarlas.
        //asi que esta funcion se va a llamar cuando se entre en normal mode
    }

    void handleInsertRareKeys()
    {
        if (tecla == KEY_ESCAPE)
        {
            if (x_actual==buffer[y_actual].size() && x_actual>0)
            {
                x_actual-=1;
            }
            actualizarTexturas();
            mode = MODE::Normal;
        }
        else if (tecla == KEY_ENTER)
        {
            if (y_actual>=y_max+y_min) 
            {
                y_min+=1;
            }

            if (x_actual == buffer[y_actual].size())
            {
                y_actual+=1;
                x_actual =x_min;
                buffer.insert(buffer.begin() + y_actual,"");
            }
            else
            {
                std::string temp = buffer[y_actual].substr(x_actual);
                buffer[y_actual].erase(x_actual);
                y_actual+=1;
                x_actual =x_min;
                buffer.insert(buffer.begin() + y_actual,temp);
            }
        }
        else if (tecla == KEY_BACKSPACE)
        {
            if (x_actual>x_min && buffer[y_actual].size()>=x_actual)
            {
                x_actual-=1;
                buffer[y_actual].erase(x_actual,1);
            }
        }
    }

    void handleInsertMode()
    {
        if (x_actual<buffer[y_actual].size())
        {
            buffer[y_actual].insert(x_actual,1,letra);
            x_actual+=1;
        }
        else
        {
            buffer[y_actual].push_back(letra);
            x_actual+=1;
        }

        if (ESCAPE_JK_OPTION == true && letra == 'j')
        {
            letraJ = true;
        }
    }

    void handleYNavegation()
    {
        if (letra == 'j' && y_actual+1 < buffer.size())
        {
            y_actual+=1;
            if (y_actual>y_max+y_min) 
            {
                y_min+=1;
            }
        }
        else if (letra == 'k' && y_actual>0)
        {
            y_actual-=1;
            if (y_actual < y_min && y_min>0) 
            {
                y_min-=1;
            }
        }
        if (buffer[y_actual].empty())
        {
            x_actual=x_min;
        }
        else
        {
            if (x_actual>=buffer[y_actual].size())
            {
                x_actual = buffer[y_actual].size()-1;
                if (x_actual<x_min)
                {
                    x_actual=x_min;
                }
            }
        }
    }

    void handleXNavegation()
    {
        if (letra == 'h' && x_actual>x_min)
        {
            x_actual-=1;
        }
        else if (letra == 'l' && x_actual<buffer[y_actual].length()-1)
        {
            x_actual+=1;
        }
    }

    void handleNormalMode()
    {
        if (letra == 'i')
        {
            mode = MODE::Insert;
        }
        else if (letra == 'o')
        {
            if (y_actual>=y_max+y_min) 
            {
                y_min+=1;
            }
            y_actual+=1;
            x_actual =x_min;
            buffer.insert(buffer.begin() + y_actual," ");
            mode = MODE::Insert;
        }
        else if (letra == 'a')
        {
            if (x_actual+1<=buffer[y_actual].size())
            {
                x_actual+=1;
            }
            mode = MODE::Insert;
        }
        else if (letra == 'A')
        {
            x_actual = buffer[y_actual].size();
            mode = MODE::Insert;
        }
        else if (letra == '0')
        {
            x_actual = x_min;
        }
        else if (letra == ':')
        {
            mode = MODE::Command;
        }
        else if (letra == 'j' || letra == 'k')
        {
            handleYNavegation();
        }
        else if (letra == 'h' || letra == 'l')
        {
            handleXNavegation();
        }
        return;
    }

    void handleCommandTypes()
    {
        if (bufferCommand == ":q")
        {
            salir = true;
        }
        else if (bufferCommand == ":wq")
        {
            std::ofstream outFile (fileName, std::ios::trunc);
            for (auto &lineas : buffer)
            {
                outFile<<lineas<<'\n';
            }
            outFile.close();
            salir = true;
        }
        else if (bufferCommand == ":w")
        {
            std::ofstream outFile (fileName, std::ios::trunc);
            for (auto &lineas : buffer)
            {
                outFile<<lineas<<'\n';
            }
            outFile.close();
        }
    }

    void handleCommandMode(int firstLetra)
    {
        float x_saved{x_actual};
        float y_saved{y_actual};

        Vector2 sizeText = MeasureTextEx(jetbrainsFont, "A", FONT_SIZE, 0);
        Vector2 posicion = { (x_min)*sizeText.x, (y_max+1)* sizeText.y };
        x_actual = posicion.x;
        y_actual = posicion.y;

        bufferCommand.push_back(':');
        renderScreen();
        // x_actual+=1; creo que es lo mejor que no me deje borrar :
        // o puedo poner algo para que brekee cuando borro ":"
        // TODO por ahora lo trickeo asi asi cree que x_min esta 1 mas
        // y el backspace no lo deja borrarlo.
        bufferCommand.push_back(firstLetra);
        renderScreen();
        x_actual+=1;

        while (!WindowShouldClose()) 
        {
            int letraCommand = GetCharPressed();
            if(letraCommand>0)
            {
                bufferCommand.push_back(letraCommand);
                x_actual+=1;
            }
            int keyCommand = GetKeyPressed();
            if(keyCommand>0)
            {
                if (keyCommand == KEY_ESCAPE)
                {
                    x_actual=x_saved;
                    y_actual=y_saved;
                    actualizarTexturas();
                    mode = MODE::Normal;
                    bufferCommand.clear();
                    break;
                }
                else if (keyCommand == KEY_ENTER)
                {
                    handleCommandTypes();

                    x_actual=x_saved;
                    y_actual=y_saved;
                    actualizarTexturas();
                    mode = MODE::Normal;
                    bufferCommand.clear();
                    break;
                }
                else if (keyCommand == KEY_BACKSPACE)
                {
                    if (x_actual>x_min && bufferCommand.size()>=x_actual)
                    {
                        x_actual-=1;
                        bufferCommand.pop_back();
                        //popbackeo ya que aun ni agregue flechitas
                        //ni tampoco pienso agregar creo
                    }
                }
            }
            renderScreen();
        }
    }

    void clearCache()
    {
        std::string locationFile = "./temp/";
        if (fs::exists(locationFile))
        {
            fs::remove_all(locationFile);
            fs::create_directory("./temp/");
        }
    }

    void createCache()
    {
        std::string locationFile = "./temp/";
        if (!fs::exists(locationFile))
        {
            fs::create_directory("./temp/");
        }
    }

    void readFileToBuffer()
    {
        file.open(fileName);
        if (file.is_open())
        {
            std::string bufferLine{};
            while (std::getline(file,bufferLine))
            {
                buffer.push_back(bufferLine);
            }
        }
        else
        {
            CloseWindow();
            std::logic_error("File no se pudo abrir.");
        }
    }
    
    void handleResizeKeys()
    {
        if (tecla == KEY_EQUAL)
        {
            FONT_SIZE+=2;
        }
        else if (tecla == KEY_MINUS)
        {
            FONT_SIZE-=2;
        }
        y_max= HEIGHT_SCREEN/FONT_SIZE-1-2;
        x_max= WIDTH_SCREEN/FONT_SIZE;
        jetbrainsFont = LoadFontEx("resources/JetBrainsMono-Medium.ttf", FONT_SIZE , NULL, 0);
        SetTextureFilter(jetbrainsFont.texture, TEXTURE_FILTER_BILINEAR);
        bloques.clear();
        clearCache();
        actualizarTexturas();
    }

    void handleCtrlKeys()
    {
        tecla = GetKeyPressed();
        if (tecla == KEY_MINUS || tecla == KEY_EQUAL)
        {
            handleResizeKeys();
        }
    }

    void general()
    {
        InitWindow(WIDTH_SCREEN, HEIGHT_SCREEN, "TYM_TYP");

        jetbrainsFont = LoadFontEx("resources/JetBrainsMono-Medium.ttf", FONT_SIZE , NULL, 0);
        SetTextureFilter(jetbrainsFont.texture, TEXTURE_FILTER_BILINEAR);
        createCache();

        readFileToBuffer();
        actualizarTexturas();

        SetTargetFPS(60); 
        SetExitKey(0); 
        while (!WindowShouldClose()) 
        {
            letra = GetCharPressed();

            if (letra>0)
            {
                if (mode == MODE::Insert)
                {
                    if (letraJ == true && ESCAPE_JK_OPTION == true && letra == 'k')
                    {
                        buffer[y_actual].erase(x_actual-1,1);
                        x_actual-=1;
                        if (x_actual==buffer[y_actual].size() && x_actual>0)
                        {
                            x_actual-=1;
                        }
                        actualizarTexturas();
                        mode=MODE::Normal;
                        letraJ=false;
                    }
                    else
                    {
                        if (letraJ == true) letraJ=false;
                        handleInsertMode();
                    }
                }
                else if (mode == MODE::Normal)
                {
                    handleNormalMode();
                }
                else if (mode == MODE::Command)
                {
                    handleCommandMode(letra);
                    if (salir == true)
                    {
                        break;
                    }
                }
            }

            if (IsKeyDown(KEY_LEFT_CONTROL))
            {
                handleCtrlKeys();
            }

            tecla = GetKeyPressed();
            if (tecla>0)
            {
                if (mode == MODE::Insert)
                {
                    if (setInsertRareKeys.find(tecla)!=setInsertRareKeys.end())
                    {
                        handleInsertRareKeys();
                    }
                }
                else if (mode == MODE::Normal)
                {
                    //aun no agregue ctrl u y esas cosas asiq da igual
                    // handleNormalMode();
                    // quizas deba hacer un handleNormal unico para
                    // teclas especificas estilo el de arriba
                }
                else if(mode == MODE::Command)
                {
                    //ni lo agregue todavia capaz ni va est
                    //en esta parte ya que : lo handlea letra
                    // handleCommandMode();
                }
            }


                // tecla = GetKeyPressed();
                // if (tecla == KEY_KP_ADD || tecla == KEY_EQUAL)
                // {
                //     handleResizeKeys();
                // }


            //parsear la screen hasta encontrar $$ y ahi quitar esa linea y compilar
            //y luego renderizar en esa posicion el png creado
            renderScreen();
        }
        file.close();
        UnloadTexture(textura);
        UnloadFont(jetbrainsFont);
        clearCache();
        CloseWindow();
    }


    
};

