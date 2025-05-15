#include "utils.h"
#include "account.h"
#include "atm.h"


globals_t *globals = NULL;

int main(int argc, char *argv[])
{
    // globals init
    global_init();


    global_free();

}