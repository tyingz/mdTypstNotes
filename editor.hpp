#include "raylib.h"
#include <string>
#include <vector>

#define WIDTH_SCREEN 1200
#define HEIGHT_SCREEN 800
#define FONT_SIZE 30
#define CURSOR_COLOR GREEN


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
    int y_actual{};
    int x_actual{};

    int letra{};
    int tecla{};
    MODE mode{Normal};

    Texture2D textura {};
    Rectangle areaRecorte {};
    Vector2 posicionDestino = { 20, 20 };

    Font jetbrainsFont; 

public:

    void renderCursor()
    {
        Vector2 size = MeasureTextEx(jetbrainsFont, "A", FONT_SIZE, 0);
        DrawRectangle(x_actual * size.x, y_actual * size.y, size.x, size.y, Fade(CURSOR_COLOR, 0.5f));
    }

    void renderScreen()
    {
        BeginDrawing();
        ClearBackground(BLACK);
        float distanciaEntreFilas{};
        for (size_t i{};i<buffer.size();i++)
        {
            Vector2 posicion = { 0, distanciaEntreFilas };
            DrawTextEx(jetbrainsFont,buffer[i].c_str(), posicion, FONT_SIZE,0, WHITE);
            distanciaEntreFilas+=FONT_SIZE;
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

    void handleInsertMode()
    {
        if (tecla == KEY_ENTER)
        {
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
        }
        else if (letra == 'k' && y_actual>0)
        {
            y_actual-=1;
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

