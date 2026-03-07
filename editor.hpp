#include "raylib.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#define WIDTH_SCREEN 1200
#define HEIGHT_SCREEN 800
#define FONT_SIZE 50
#define CURSOR_COLOR GREEN
#define SET_SCROLL 4


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

    void renderScreen()
    {
        BeginDrawing();
        ClearBackground(BLACK);
        float distanciaEntreFilas{};
        for (size_t fila{};fila<buffer.size();fila++)
        {
            if (fila+y_min<buffer.size())
            {
                Vector2 posicion = { 0, distanciaEntreFilas };
                DrawTextEx(jetbrainsFont,buffer[fila+y_min].c_str(), posicion, FONT_SIZE,0, WHITE);
                distanciaEntreFilas+=FONT_SIZE;
            }
        }
        if (textura.id>0)
        {
            DrawTextureRec(textura, areaRecorte, posicionDestino, WHITE);
        }
        renderCursor();
        EndDrawing();
    }

    void actualizarRenderizado()
    {
        UnloadTexture(textura); 
        textura = LoadTexture("./temp/1.png"); 
        areaRecorte = { 0.0f, 100.0f, (float)textura.width, 100.0f};
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
                x_actual+=1; //luego cuando agregue backspace toquetear
            }
            else
            {
                buffer[y_actual].push_back(letra);
                x_actual+=1; //luego cuando agregue backspace toquetear
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
        // actualizarRenderizado();
        while (!WindowShouldClose()) {
            letra = GetCharPressed();
            tecla = GetKeyPressed();
            if (letra || tecla)
            {
                if (mode == MODE::Insert)
                {
                    if (tecla == KEY_ESCAPE)
                    {
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

