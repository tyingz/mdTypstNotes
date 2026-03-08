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
    std::vector<std::string> buffer{""};
    float y_actual{};
    float x_actual{};

    float y_max{HEIGHT_SCREEN/FONT_SIZE-1}; //idem abajo
    float x_max{WIDTH_SCREEN/FONT_SIZE}; //chequear si va float

    float y_min{};
    float x_min{};

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
        KEY_ENTER,
        KEY_BACKSPACE,
    };

public:

    void renderCursor()
    {
        Vector2 size = MeasureTextEx(jetbrainsFont, "A", FONT_SIZE, 0);
        DrawRectangle(x_actual * size.x, (y_actual-y_min)* size.y, size.x, size.y, Fade(CURSOR_COLOR, 0.5f));
    }

    void renderEcuaciones()
    {
        //la idea es que tape las ecs. a menos que pase el cursor por arriba?
        //aun nose como no renderizar si esta arriba mi cursor asi que
        //voy a renderizar siempre que este en normal mode y luego
        //agrego ese feature
        for (auto &bloque : bloques)
        {
            bool cursorEncima = (y_actual >= bloque.inicioFila && y_actual <= bloque.finalFila);
            if (texturaGuardada.count(bloque.id) && !cursorEncima)
            {
                Texture2D texturaEcuacion=texturaGuardada[bloque.id];
                DrawTexture(texturaEcuacion,x_min,(bloque.inicioFila-y_min)*FONT_SIZE,WHITE);
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
        renderCursor();
        EndDrawing();
    }

    void actualizarTexturas()
    {
        bloques = read.parser(buffer,y_min,y_max);
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
        if (tecla == KEY_ENTER)
        {
            if (y_actual>=y_max+y_min) 
            {
                y_min+=1;
            }

            if (x_actual == buffer[y_actual].size())
            {
                y_actual+=1;
                x_actual =0;
                buffer.insert(buffer.begin() + y_actual,"");
            }
            else
            {
                std::string temp = buffer[y_actual].substr(x_actual);
                buffer[y_actual].erase(x_actual);
                y_actual+=1;
                x_actual =0;
                buffer.insert(buffer.begin() + y_actual,temp);
            }
        }
        else if (tecla == KEY_BACKSPACE)
        {
            if (x_actual>0 && buffer[y_actual].size()>=x_actual)
            {
                x_actual-=1;
                buffer[y_actual].erase(x_actual,1);
            }
        }
    }

    void handleInsertMode()
    {
        if (setInsertRareKeys.find(tecla)!=setInsertRareKeys.end())
        {
            handleInsertRareKeys();
        }
        else
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

        if (x_actual>buffer[y_actual].size())
        {
            x_actual = buffer[y_actual].size();
        }


    }

    void handleXNavegation()
    {
        if (letra == 'h' && x_actual>0)
        {
            x_actual-=1;
        }
        else if (letra == 'l' && x_actual<buffer[y_actual].length())
        {
            x_actual+=1;
        }
    }

    void handleNormalMode()
    {
        if (letra == 'j' || letra == 'k')
        {
            handleYNavegation();
        }
        else if (letra == 'h' || letra == 'l')
        {
            handleXNavegation();
        }
        return;
    }

    void handleCommandMode()
    {

    }

    void general()
    {
        InitWindow(WIDTH_SCREEN, HEIGHT_SCREEN, "TYM_TYP");

        jetbrainsFont = LoadFontEx("resources/JetBrainsMono-Medium.ttf", FONT_SIZE , NULL, 0);
        SetTextureFilter(jetbrainsFont.texture, TEXTURE_FILTER_BILINEAR);

        SetTargetFPS(60); 
        SetExitKey(0); 
        // actualizarTexturas();
        while (!WindowShouldClose()) {
            tecla = GetKeyPressed();
            letra = GetCharPressed();
            if (letra || tecla)
            {
                if (mode == MODE::Insert)
                {
                    if (tecla == KEY_ESCAPE)
                    {
                        actualizarTexturas();
                        mode = MODE::Normal;
                    }
                    else
                    {
                        handleInsertMode();
                    }
                }
                else if (mode == MODE::Normal)
                {
                    if (letra == 'i')
                    {
                        mode = MODE::Insert;
                    }
                    else
                    {
                        handleNormalMode();
                    }
                }
                else if(mode == MODE::Command)
                {
                    handleCommandMode();
                }
            }
            //parsear la screen hasta encontrar $$ y ahi quitar esa linea y compilar
            //y luego renderizar en esa posicion el png creado
            renderScreen();
        }
        UnloadTexture(textura);
        UnloadFont(jetbrainsFont);
        CloseWindow();
    }


    
};

