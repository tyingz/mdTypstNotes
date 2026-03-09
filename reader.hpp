#include "raylib.h"
#include "options.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <unordered_set>
#include <unordered_map>

#include <filesystem>
namespace fs = std::filesystem;
/*
   La idea es por ahora leer unicamente expresiones 
   $$ 
   EXPRESION DE TYPST 
   $$
   guardarlas en el vector con su posicion de inicio de $$ y final $$
   y luego mandarlas a un renderizador para que las renderice arriba, nose aun
   si esta es la mejor idea.. Seguro va a ir mutando.
*/

struct TypstBlock
{
    size_t inicioFila{};
    size_t finalFila{};
    size_t id{};
    Rectangle areaEcuation{};
    bool flag{};
};

class Reader
{
private:
    size_t m_id{};
    // std::string configsTypstTopFile{"#set page(width: auto, height: auto, margin: 5pt)\n"};
    std::string configsTypstTopFile{"#set page(width: 1000pt, height: 35pt, margin: 5pt)\n#show math.equation.where(block: true): align.with(left)\n#set page(fill: black)\n#set text(fill: white)\n"};
    //TODO Dejar de hardcodear height y width ya que son sensibles a FONT_SIZE y sreenwidth
    //TODO Dejar de hardcodear height y width ya que son sensibles a FONT_SIZE y sreenwidth
    //TODO Dejar de hardcodear height y width ya que son sensibles a FONT_SIZE y sreenwidth
    //TODO Dejar de hardcodear height y width ya que son sensibles a FONT_SIZE y sreenwidth
    //El auto no me gusta xq quiero que sea larga la ec renderizada.
public:
    std::vector<TypstBlock> parser(const std::vector<std::string> &buffer,size_t y_min,
            size_t y_max)
    {
        std::vector<TypstBlock> vectorTypstBlock{};
        size_t tempInicio{};
        size_t tempFinal{};
        std::vector<std::string> equationsToCompile{};
        std::string bufferEquation{};
        bool reading{false};
        for (size_t fila{y_min};fila<std::min(buffer.size(),y_min+y_max);fila++)
        {
            if (buffer[fila].find("$")!=std::string::npos)
            {
                if (reading == true)
                {
                    tempFinal = fila;
                    bufferEquation.append(buffer[fila]);

                    TypstBlock bloqueActual{};

                    bloqueActual.inicioFila=tempInicio;
                    bloqueActual.finalFila=tempFinal;

                    Rectangle Area;
                    Area.x=(float)tempInicio;
                    Area.y=(float)tempFinal;
                    Area.width=WIDTH_SCREEN;
                    Area.height=(tempFinal-tempInicio)*FONT_SIZE;
                    bloqueActual.areaEcuation = Area;

                    m_id = std::hash<std::string>{}(bufferEquation);
                    bloqueActual.id = m_id;

                    std::string locationFile = "./temp/file";
                    locationFile.append(std::to_string(m_id));
                    locationFile.append(".typ");

                    std::ofstream file {locationFile};
                    file << configsTypstTopFile;
                    file << bufferEquation;
                    file.close();

                    std::string command = "typst compile ";
                    command.append(locationFile);

                    std::string locationImage{};
                    locationImage.append("./temp/formula");
                    locationImage.append(std::to_string(m_id));
                    locationImage.append(".png");

                    command.append(" ");
                    command.append(locationImage);
                    command.append(" --ppi 300");

                    if (!fs::exists(locationImage))
                    {
                        if (std::system(command.c_str())==0)
                        {
                            vectorTypstBlock.push_back(bloqueActual);
                            tempFinal = 0;
                            tempInicio = 0;
                            bufferEquation.clear();
                            reading = false;
                        }
                        else
                        {
                            throw std::logic_error("NO LOADEO LA IMAGEN BIEN.");
                        }
                    }
                    else
                    {
                        vectorTypstBlock.push_back(bloqueActual);
                        tempFinal = 0;
                        tempInicio = 0;
                        bufferEquation.clear();
                        reading = false;
                    }
                }
                else
                {
                    tempInicio = fila;
                    bufferEquation.append(buffer[fila]);
                    reading = true;
                }
            }
            else
            {
                if (reading == true)
                {
                    bufferEquation.append(buffer[fila]);
                }
            }
        }
        return vectorTypstBlock;
    }

};









