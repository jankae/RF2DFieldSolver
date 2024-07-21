#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

#include "lattice.h"
#include "worker.h"

/**
 * This function setups each of the cell in the lattice.
 */
void lattice_set_size(struct lattice* lattice, struct rect* size);

/**
 * This function applies the boundary function to each of the cell.
 */
void lattice_apply_bound(struct lattice* lattice, bound_t func, void *ptr);

/**
 * This function applies the weight function to each of the cells.
 */
void lattice_apply_weight(struct lattice* lattice, weight_t func, void *ptr);

/**
 * This function generates the function for each of the cell.
 */
void lattice_generate_function(struct lattice* lattice);

/**
 * This function applies one sequential iteration.
 */
double lattice_iterate(struct lattice* lattice);

struct lattice* lattice_new(struct rect* size, struct point* dim, bound_t func, weight_t w_func, void *ptr) {
    struct cell* cells;
    struct lattice* lattice;
    double (**update)(struct lattice*, struct cell*);

    /* make sure the dimension is useful */
    if(dim->x == 0 || dim->y == 0)
        return NULL;

    /* add two rows and two columns */
    dim->x += 3;
    dim->y += 3;

    /* compute the number of cell */
    uint32_t m = dim->x*dim->y;

    /* allocate memory for the cells */
    cells = malloc(m*sizeof(struct cell));
    if(cells == NULL) goto ERROR;

    /* allocate memory for the functions */
    update = malloc(m*sizeof(double (*)(struct lattice*, struct cell*)));
    if(update == NULL) goto ERROR;

    /* allocate the memory for the lattice structure */
    lattice = malloc(sizeof(struct lattice));
    if(lattice == NULL) goto ERROR;

    /* initialise the lattice structure */
    lattice->dim.x = dim->x;
    lattice->dim.y = dim->y;
    lattice->cells = cells;
    lattice->update = update;
    lattice->abort = false;

    /* apply all the steps for finishing the lattice */
    lattice_set_size(lattice, size);
    lattice_apply_bound(lattice, func, ptr);
    lattice_apply_weight(lattice, w_func, ptr);
    lattice_generate_function(lattice);

    return lattice;

ERROR:
    if(cells   != NULL) free(cells);
    if(update  != NULL) free(update);
    if(lattice != NULL) free(lattice);

    return NULL;
}

void lattice_delete(struct lattice* lattice) {
    /* free all the allocated memory */
    free(lattice->cells);
    free(lattice->update);
    free(lattice);
}

void lattice_print(struct lattice* lattice) {
    /* extract the dimension of the lattice */
    uint32_t w = lattice->dim.x;
    uint32_t h = lattice->dim.y;

    for(uint32_t j = 0; j < h; j++) {
        for(uint32_t i = 0; i < w; i++) {
            /* extract the pointer to the specified cell */
            struct cell* cell = &lattice->cells[i+j*w];

            /* don't print cells if they contain neumann condition */
            if(cell->cond == NEUMANN)
                fprintf(stderr, "          ,");
            else
                fprintf(stderr, "% 10.5f,", cell->value);
        }
        fprintf(stderr, "\n");
    }
}

void lattice_set_size(struct lattice* lattice, struct rect* size) {
    /* extract the dimension of the lattice */
    int32_t w = lattice->dim.x;
    int32_t h = lattice->dim.y;

    /* compute the subdivisions */
    double dx = size->x/(w-3);
    double dy = size->y/(h-3);

    for(int32_t j = -1; j+1 < h; j++) {
        for(int32_t i = -1; i+1 < w; i++) {
            /* extract the pointer to the specified cell */
            struct cell* cell = &lattice->cells[(i+1)+(j+1)*w];

            /* initialise the cell */
            cell->pos.x = i*dx;
            cell->pos.y = j*dy;
            cell->index.x = i+1;
            cell->index.y = j+1;
            cell->value = 0;

            /* the limits of the lattice are Neumann conditions */
            if(i == -1 || j == -1 || i == w-2 || j == h-2) {
                /* we don't need the adjacent cells */
                cell->adj[0] = 0;
                cell->adj[1] = 0;
                cell->adj[2] = 0;
                cell->adj[3] = 0;

                /* we don't need the adjacent cells */
                cell->diag[0] = 0;
                cell->diag[1] = 0;
                cell->diag[2] = 0;
                cell->diag[3] = 0;

                cell->cond = NEUMANN;
            } else {
                /* compute the index of the adjacent cells */
                cell->adj[0] = (i+1)+(j-0)*w;
                cell->adj[1] = (i+1)+(j+2)*w;
                cell->adj[2] = (i-0)+(j+1)*w;
                cell->adj[3] = (i+2)+(j+1)*w;

                /* compute the index of the diagonal cells */
                cell->diag[0] = (i+2)+(j-0)*w;
                cell->diag[1] = (i+2)+(j+2)*w;
                cell->diag[2] = (i-0)+(j+2)*w;
                cell->diag[3] = (i-0)+(j-0)*w;

                cell->cond = UNSET;
            }
        }
    }
}

void lattice_apply_bound(struct lattice* lattice, bound_t func, void *ptr) {
    /* extract the dimension of the lattice */
    uint32_t w = lattice->dim.x;
    uint32_t h = lattice->dim.y;

    /* used for returning data from the boundary function */
    struct bound bound = {NONE, 0};

    /* apply the boundary function to each cell */
    for(uint32_t j = 0; j < h; j++) {
        for(uint32_t i = 0; i < w; i++) {
            /* extract the pointer to the specified cell */
            struct cell* cell = &lattice->cells[i+j*w];

            /* make sure the cell isn't already set */
            if(cell->cond != UNSET)
                continue;

            /* apply the boundary function */
            if(func(ptr, &bound, &cell->pos) == NULL)
                continue;

            /* update the cell */
            cell->value = bound.value;
            cell->cond  = bound.cond;
        }
    }
}

void lattice_apply_weight(struct lattice* lattice, weight_t func, void *ptr) {
    /* extract the dimension of the lattice */
    uint32_t w = lattice->dim.x;
    uint32_t h = lattice->dim.y;

    /* apply the weight function to each cell */
    for(uint32_t j = 0; j < h; j++) {
        for(uint32_t i = 0; i < w; i++) {
            /* extract the pointer to the specified cell */
            struct cell* cell = &lattice->cells[i+j*w];

            /* update the cell */
            cell->weight = func(ptr, &cell->pos);
        }
    }
}

/* definition of the supported configurations */
#define GETFORMULA_MIDDLE_0     (  v1*w1+  v2*w2+  v3*w3+  v4*w4)/(w1+w2+w3+w4)
#define GETFORMULA_SIDE_1       (     2*v2*w2+  v3*w3+  v4*w4)/(2*w2+w3+w4)
#define GETFORMULA_SIDE_2       (2*v1*w1     +  v3*w3+  v4*w4)/(2*w1+w3+w4)
#define GETFORMULA_SIDE_3       (  v1*w1+  v2*w2+    +2*v4*w4)/(2*w4+w1+w2)
#define GETFORMULA_SIDE_4       (  v1*w1+  v2*w2+2*v3*w3     )/(2*w3+w1+w2)
#define GETFORMULA_CORNER_1     (   v2*w2+v3*w3   )/(w2+w3)
#define GETFORMULA_CORNER_2     (v1*w1   +v3*w3   )/(w1+w3)
#define GETFORMULA_CORNER_3     (v1*w1      +v4*w4)/(w1+w4)
#define GETFORMULA_CORNER_4     (   v2*w2   +v4*w4)/(w2+w4)
#define GETFORMULA_INV_CORNER_1 (  v1*w1+2*v2*w2+2*v3*w3+  v4+w4)/(w1+2*w2+2*w3+w4)
#define GETFORMULA_INV_CORNER_2 (2*v1*w1+  v2*w2+2*v3*w3+  v4*w4)/(2*w1+w2+2*w3+w4)
#define GETFORMULA_INV_CORNER_3 (2*v1*w1+  v2*w2+  v3*w3+2*v4*w4)/(2*w1+w2+w3+2*w4)
#define GETFORMULA_INV_CORNER_4 (  v1*w1+2*v2*w2+  v3*w3+2*v4*w4)/(w1+2*w2+w3+2*w4)

/**
 * This macro generates function for the supported configurations.
 * Even though not each function needs all the adjacent point, the
 * compiler should optimize away points that are not needed.
 */
#define MAKE_FUNCPOINTS(NUM,IDX) \
double func_##NUM##_##IDX (struct lattice* lattice, struct cell* cell) {\
    uint32_t i1 = cell->adj[0];           \
    uint32_t i2 = cell->adj[1];           \
    uint32_t i3 = cell->adj[2];           \
    uint32_t i4 = cell->adj[3];           \
                                          \
    double v1 = lattice->cells[i1].value; \
    double v2 = lattice->cells[i2].value; \
    double v3 = lattice->cells[i3].value; \
    double v4 = lattice->cells[i4].value; \
                                          \
    double w1 = lattice->cells[i1].weight;\
    double w2 = lattice->cells[i2].weight;\
    double w3 = lattice->cells[i3].weight;\
    double w4 = lattice->cells[i4].weight;\
                                          \
    (void)v1;(void)v2;(void)v3;(void)v4;  \
    (void)w1;(void)w2;(void)w3;(void)w4;  \
    return GETFORMULA_##NUM##_##IDX ;     \
}

/* we generate the function for each configuration */
MAKE_FUNCPOINTS(MIDDLE,0)
MAKE_FUNCPOINTS(SIDE,1)
MAKE_FUNCPOINTS(SIDE,2)
MAKE_FUNCPOINTS(SIDE,3)
MAKE_FUNCPOINTS(SIDE,4)
MAKE_FUNCPOINTS(CORNER,1)
MAKE_FUNCPOINTS(CORNER,2)
MAKE_FUNCPOINTS(CORNER,3)
MAKE_FUNCPOINTS(CORNER,4)
MAKE_FUNCPOINTS(INV_CORNER,1)
MAKE_FUNCPOINTS(INV_CORNER,2)
MAKE_FUNCPOINTS(INV_CORNER,3)
MAKE_FUNCPOINTS(INV_CORNER,4)

void lattice_generate_function(struct lattice* lattice) {
    /* extract the dimension of the lattice */
    int32_t w = lattice->dim.x;
    int32_t h = lattice->dim.y;

    for(int32_t j = 0; j < h; j++) {
        for(int32_t i = 0; i < w; i++) {
            /* compute the index of the cell */
            uint32_t index = i+j*w;

            /* extract the pointer to the specified cell */
            struct cell* cell = &lattice->cells[index];

            /* we ignore neumann or dirichlet conditions */
            if(cell->cond == NEUMANN || cell->cond == DIRICHLET) {
                lattice->update[index] = NULL;

                continue;
            }

            /* extracts each adjacent cell */
            struct cell* a1 = &lattice->cells[cell->adj[0]];
            struct cell* a2 = &lattice->cells[cell->adj[1]];
            struct cell* a3 = &lattice->cells[cell->adj[2]];
            struct cell* a4 = &lattice->cells[cell->adj[3]];

            struct cell* d1 = &lattice->cells[cell->diag[0]];
            struct cell* d2 = &lattice->cells[cell->diag[1]];
            struct cell* d3 = &lattice->cells[cell->diag[2]];
            struct cell* d4 = &lattice->cells[cell->diag[3]];

            /* check if the adjacent cells are neumann boundary */
            int A1 = (a1->cond == NEUMANN) ? 1 : 0;
            int A2 = (a2->cond == NEUMANN) ? 1 : 0;
            int A3 = (a3->cond == NEUMANN) ? 1 : 0;
            int A4 = (a4->cond == NEUMANN) ? 1 : 0;

            /* check if the diagonal cells are neumann boundary */
            int D1 = (d1->cond == NEUMANN) ? 1 : 0;
            int D2 = (d2->cond == NEUMANN) ? 1 : 0;
            int D3 = (d3->cond == NEUMANN) ? 1 : 0;
            int D4 = (d4->cond == NEUMANN) ? 1 : 0;

            /* generate a function the supported configurations */
            #define f lattice->update[index]
            if(!A1 && !A2 && !A3 && !A4) {
                if     ( D1 && !D2 && !D3 && !D4) f = &func_INV_CORNER_1;
                else if(!D1 &&  D2 && !D3 && !D4) f = &func_INV_CORNER_2;
                else if(!D1 && !D2 &&  D3 && !D4) f = &func_INV_CORNER_3;
                else if(!D1 && !D2 && !D3 &&  D4) f = &func_INV_CORNER_4;
                else f = &func_MIDDLE_0;
            }
            else if( A1 && !A2 && !A3 && !A4) f = &func_SIDE_1;
            else if(!A1 &&  A2 && !A3 && !A4) f = &func_SIDE_2;
            else if(!A1 && !A2 &&  A3 && !A4) f = &func_SIDE_3;
            else if(!A1 && !A2 && !A3 &&  A4) f = &func_SIDE_4;
            else if( A1 && !A2 && !A3 &&  A4) f = &func_CORNER_1;
            else if(!A1 &&  A2 && !A3 &&  A4) f = &func_CORNER_2;
            else if(!A1 &&  A2 &&  A3 && !A4) f = &func_CORNER_3;
            else if( A1 && !A2 &&  A3 && !A4) f = &func_CORNER_4;
            else f = &func_MIDDLE_0;
            #undef f
        }
    }
}

uint32_t lattice_compute(struct lattice* lattice, double threshold) {
    uint32_t iterations = 0;

    /* apply iterations until the threshold is bigger */
    while(lattice_iterate(lattice) > threshold)
        iterations++;

    return iterations;
}

double lattice_iterate(struct lattice* lattice) {
    /* extract the dimension of the lattice */
    int32_t w = lattice->dim.x;
    int32_t h = lattice->dim.y;

    /* the largest difference */
    double diff = 0;

    for(int32_t j = 0; j < h; j++) {
        for(int32_t i = 0; i < w; i++) {
            /* compute the index of the cell */
            uint32_t index = i+j*w;
            double value, check;

            /* extract the pointer to the specified cell */
            struct cell* cell = &lattice->cells[index];

            /* make sure the cell can be updated */
            if(lattice->update[index] == NULL)
                continue;

            /* compute the new value */
            value = (*lattice->update[index])(lattice, cell);
            check = fabs(value-cell->value);
            if(check > diff) diff = check;

            /* update the cell */
            cell->value = value;
        }
    }

    return diff;
}

uint32_t lattice_compute_threaded(struct lattice* lattice, struct config* conf, progress_callback_t cb, void *cb_ptr) {
    struct worker* worker;
    struct worker* next;
    uint32_t iterations = 0;

    /* create the first worker and wait */
    worker = worker_new(NULL, lattice, conf, cb, cb_ptr);
    pthread_join(worker->thread, NULL);

    /* traver the linked list */
    next = worker->next;
    worker->next = NULL;
    while(next != NULL) {
        /* count the total number of iterations */
        iterations += next->iterations;

        /* free the workers */
        next = next->next;
        if(next != NULL)
           worker_delete(next->previous);
    }

    /* free the last worker */
    worker_delete(worker);

    return iterations;
}

