#include <stdio.h>
#include "lib1718.h"

int main(int argc, const char * argv[]) {
    char * query = "INSERT QUERY HERE";
    bool ifOk = executeQuery(query);
    printf("%d ", ifOk);
    return 0;
}

