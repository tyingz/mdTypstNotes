#include "raylib.h"
#include "options.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <algorithm>

#include <filesystem>
namespace fs = std::filesystem;

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
    // std::string configsTypstTopFile{"#set page(width: 1000pt, height: 35pt, margin: 5pt)\n#show math.equation.where(block: true): align.with(left)\n#set page(fill: black)\n#set text(fill: white)\n"};
    std::string configsTypstTopFile{"#set page(width: auto, height: auto, margin: 5pt)\n#show math.equation.where(block: true): align.with(left)\n#set page(fill: black)\n#set text(fill: white)\n"};
public:
    std::vector<TypstBlock> parser(const std::vector<std::string> &buffer,size_t y_min,
            size_t y_max, size_t x_min, size_t x_max, float FONT_SIZE)
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
                    bufferEquation.append(buffer[fila]);

                    TypstBlock bloqueActual{};

                    tempFinal = fila;
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

                    // command.append(" --ppi 400");
                    command.append(" --ppi 300");
                    // aca toquetear segun el size
                    // aca toquetear segun el size
                    // aca toquetear segun el size
                    // aca toquetear segun el size

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
                            // throw std::logic_error("NO LOADEO LA IMAGEN BIEN.");
                            // capaz luego poner alguna textura para que
                            // se de cuenta que la cago
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
                    //aca chequeemos que no haya 2 es decir $ $
                    auto primero = buffer[fila].find("$");
                    if (buffer[fila].find("$",primero+1)!=std::string::npos)
                    {
                        //REORDENAR EL CODIGO PARA REUTILIZAR LA CREACION DE CODIGO
                        //DE ARRIBA PARA EL STRING QUE SE FORMA ENTRE MEDIO DE LOS DOS
                        //$ STRING $...
                    }
                    else
                    {
                        tempInicio = fila;
                        bufferEquation.append(buffer[fila]);
                        reading = true;
                    }
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









