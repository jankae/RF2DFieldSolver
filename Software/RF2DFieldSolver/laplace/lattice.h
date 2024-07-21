#ifndef INCLUDE_LATTICE_H
#define INCLUDE_LATTICE_H

#include <stdint.h>
#include <stdbool.h>

#include "tuple.h"
#include "worker.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This enumeration defines the possible conditions that
 * can be applied to the problem.
 */
enum condition {
    UNSET,
    NONE,
    NEUMANN,
    DIRICHLET,
};

/**
 * This structure represent a cell in the matrix.
 */
struct cell {
    /**
     * This is the position of the cell in space.
     */
    struct rect pos;
    /**
     * This is the index position of the cell in the matrix.
     */
    struct point index;
    /**
     * This is the current value contained in the cell.
     */
    double value;
    /**
     * This is the condition applied to this cell.
     */
    enum condition cond;
    /**
     * This is the weight applied to this cell.
     */
    double weight;
    /**
     * These are the indexes of the four adjacent cells.
     *
     *      [ ] [1] [ ]
     *      [3] [x] [4]
     *      [ ] [2] [ ]
     *
     */
    uint32_t adj[4];
    /**
     * These are the indexes of the four diagonal cells.
     *
     *      [4] [ ] [1]
     *      [ ] [x] [ ]
     *      [3] [ ] [2]
     *
     */
    uint32_t diag[4];
};

typedef double (*weight_t)(void *ptr, struct rect*);

/**
 * This structure represent the entire matrix used for
 * solving the laplace equation with conditions.
 */
struct lattice {
    /**
     * This contains the size of the matrix.
     */
    struct point dim;
    /**
     * This is the actual matrix of cell.
     */
    struct cell* cells;
    /**
     * This is the matrix containing all the update functions
     * for each of the cell.
     */
    double (**update)(struct lattice*, struct cell*);
    /**
     * Set this to true if all threads should abort their calculation as soon as possible
     */
    bool abort;
};

/**
 * This structure contains the conditions returned by the
 * boundaries function.
 */
struct bound {
    /**
     * This contains the value of the cell.
     */
    double value;
    /**
     * This contains the condition applied to the cell.
     */
    enum condition cond;
};

/**
 * This is the definition of the boundaries function. During the
 * creation of the lattice, this function is called for each cell.
 * This allows the user to define custom boundary conditions for
 * each individual cells.
 *
 * Only the spatial position of the cell is needed for defining the
 * boundary conditions. This function must returns the same structure
 * that has been passed in argument.
 */
typedef struct bound* (*bound_t)(void *ptr, struct bound*, struct rect*);

/**
 * This function creates a lattice ready to be computed. It first
 * starts by allocating memory and appliying the boundary function.
 * After this step, it will generate a function that will update each
 * cell based on the adjacent cells.
 *
 * Be warned that the final matrix contains 2 more columns and rows
 * in order to facilitate the computing. These added cells won't be
 * sent to the boundary function.
 *
 * @param size
 *        This rectangle represents the spatial size of the problem.
 * @param dim
 *        This point represents the resolution of the matrix.
 * @param func
 *        This is a pointer to the boundary function.
 *
 * @return The pointer to the new lattice if everyhthing went as
 *         expected, else @{code NULL} value.
 */
struct lattice* lattice_new(struct rect* size, struct point* dim, bound_t func, weight_t w_func, void *ptr);

/**
 * This function frees the memory of a lattice.
 *
 * @param lattice
 *        This is a pointer to the lattice to free.
 */
void lattice_delete(struct lattice* lattice);

/**
 * This function prints the value of each cell inside a lattice.
 *
 * @param lattice
 *        This is a pointer to the lattice to free.
 */
void lattice_print(struct lattice* lattice);

/**
 * This function computes the laplace equation sequentially for a
 * given lattice.
 *
 * @param lattice
 *        This is a pointer to the lattice.
 * @param threshold
 *        This is the threhold used to stop the computation.
 *
 * @return The number of iterations.
 */
uint32_t lattice_compute(struct lattice* lattice, double threshold);

/**
 * This function computes the laplace equation in parrallel for a
 * given lattice. The number of iteration might varies from one
 * execution to an other.
 *
 * @param lattice
 *        This is a pointer to the lattice.
 * @param conf
 *        This is a pointer the configuration of the computation.
 *
 * @return The number of iterations.
 */
uint32_t lattice_compute_threaded(struct lattice* lattice, struct config* conf, progress_callback_t cb, void *cb_ptr);

#ifdef __cplusplus
}
#endif

#endif
