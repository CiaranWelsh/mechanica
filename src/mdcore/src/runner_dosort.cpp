/*******************************************************************************
 * This file is part of mdcore.
 * Coypright (c) 2013 Pedro Gonnet (pedro.gonnet@durham.ac.uk)
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 ******************************************************************************/

/* Include configuration header */
#include "mdcore_config.h"

/* Include some standard header files */
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <limits.h>

/* Include some conditional headers. */
#include "mdcore_config.h"
#ifdef __SSE__
    #include <xmmintrin.h>
#endif
#ifdef HAVE_SETAFFINITY
    #include <sched.h>
#endif
#ifdef WITH_MPI
    #include <mpi.h>
#endif

/* Include local headers */
#include "cycle.h"
#include "errs.h"
#include "fptype.h"
#include "lock.h"
#include <MxParticle.h>
#include <space_cell.h>
#include "space.h"
#include "potential.h"
#include "potential_eval.h"
#include "engine.h"
#include "runner.h"

/* the error macro. */
#define error(id)				( runner_err = errs_register( id , runner_err_msg[-(id)] , __LINE__ , __FUNCTION__ , __FILE__ ) )

/* list of error messages. */
extern char *runner_err_msg[];
extern unsigned int runner_rcount;


/**
 * @brief Fill in the pairwise Verlet list entries for the given cell pair
 *        if needed and compute the interactions.
 * 
 * @param r The #runner computing the pair.
 * @param c The cell.
 * @param flags Bitmask for the sorting directions.
 *
 * @return #runner_err_ok or <0 on error (see #runner_err)
 *
 * This routine differs from #runner_dopair_verlet in that instead of
 * storing a Verlet table, the sorted particle ids are stored. This
 * requires only (size_i + size_j) entries as opposed to size_i*size_j
 * for the Verlet table, yet may be less efficient since particles
 * within the skin along the cell-pair axis are inspected, as opposed
 * to particles simply within the skin of each other.
 *
 */
 
__attribute__ ((flatten)) int runner_dosort ( struct runner *r , struct space_cell *c , int flags ) {

    struct MxParticle *p;
    struct space *s;
    int i, k, sid;
    struct MxParticle *parts;
    struct engine *eng;
    unsigned int *iparts;
    FPTYPE dscale;
    FPTYPE shiftn[3], bias;
    int count;
    
    
    /* break early if one of the cells is empty */
    count = c->count;
    if ( count == 0 )
        return runner_err_ok;
    
    /* get the space and cutoff */
    eng = r->e;
    s = &(eng->s);
    bias = sqrt( s->h[0]*s->h[0] + s->h[1]*s->h[1] + s->h[2]*s->h[2] );
    dscale = (FPTYPE)SHRT_MAX / (2 * bias);
    
    
    /* Make local copies of the parts if requested. */
    if ( r->e->flags & engine_flag_localparts ) {
        parts = (struct MxParticle *)alloca( sizeof(struct MxParticle) * count );
        memcpy( parts , c->parts , sizeof(struct MxParticle) * count );
        }
    else
        parts = c->parts;
        

    /* Loop over the sort directions. */
    for ( sid = 0 ; sid < 13 ; sid++ ) {
    
        /* In the mask? */
        if ( !( flags & ( 1 << sid ) ) )
            continue;

        /* Get the normalized shift. */
        for ( k = 0 ; k < 3 ; k++ )
            shiftn[k] = cell_shift[ 3*sid + k ];

        /* Get the pointers to the sorted particle data. */
        iparts = &c->sortlist[ count * sid ];

        /* start by filling the particle ids and dists */
        for ( i = 0 ; i < count ; i++ ) {
            p = &( parts[i] );
            iparts[i] = (i << 16) |
                (unsigned int)( dscale * ( bias + p->x[0]*shiftn[0] + p->x[1]*shiftn[1] + p->x[2]*shiftn[2] ) );
            }

        /* Sort this data in descending order. */
        runner_sort_descending( iparts , count );
    
        }

        
    /* since nothing bad happened to us... */
    return runner_err_ok;

    }
    
    
