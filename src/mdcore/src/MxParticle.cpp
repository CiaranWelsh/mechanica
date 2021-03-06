/*******************************************************************************
 * This file is part of mdcore.
 * Coypright (c) 2010 Pedro Gonnet (pedro.gonnet@durham.ac.uk)
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


/* include some standard header files */
#include <stdlib.h>
#include <math.h>
#include <MxParticle.h>
#include "fptype.h"

MxParticleType particleTypeType = {
    {
        {.ob_refcnt = 0,
         .ob_type = NULL
        },
        .tp_name = "mechanica.ParticleType",
        .tp_doc = "Custom objects",
        .tp_basicsize = sizeof(MxParticleType),
        .tp_itemsize = 0,
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_new = PyType_GenericNew
    }
};

MxParticleType *MxParticleType_Type = &particleTypeType;


/** ID of the last error */
int particle_err = PARTICLE_ERR_OK;


/**
 * @brief Initialize a #part.
 *
 * @param p The #part to be initialized.
 * @param vid The virtual id of this #part.
 * @param type The numerical id of the particle type.
 * @param flags The particle flags.
 *
 * @return #part_err_ok or < 0 on error (see #part_err).
 *
 */

int particle_init ( struct MxParticle *p , int vid , int type , unsigned int flags ) {

    /* check inputs */
    if ( p == NULL )
        return particle_err = PARTICLE_ERR_NULL;

    /* Set the paticle data. */
    p->vid = vid;
    p->typeId = type;
    p->flags = flags;

    /* all is well... */
    return PARTICLE_ERR_OK;

}

HRESULT MxParticle_Init(PyObject *m)
{
}

int MxParticleCheck(PyObject *o)
{
}
