#include "pipe.h"
#include "ndninterface.h"
int main() {
    pipe_t *p = pipe_new(sizeof(char), 0);
    pipe_producer_t * pro = pipe_producer_new(p);
    getbyname("/example/data/2", pro);
    return 0;
}
