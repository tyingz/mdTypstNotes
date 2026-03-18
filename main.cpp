#include "editor.hpp"

int main(int argc, char *argv[]) 
{
    if (argc !=2)
    {
        std::cerr<<"Debe dar de argumento el archivo a editar.";
        return 1;
    }
    Editor editThis{argv[1]};
    editThis.general();

    return 0;
}
