/*
 * MxCell.h
 *
 *  Created on: Jul 7, 2017
 *      Author: andy
 */

#ifndef SRC_MXCELL_H_
#define SRC_MXCELL_H_

#include "mechanica_private.h"
#include <vector>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Vector3.h>

namespace Magnum {
/** @brief Three-component unsigned integer vector */
typedef Math::Vector3<UnsignedShort> Vector3us;
}

using namespace Magnum;

struct MxMesh;

/**
 * Use unsigned as indexes, saves memory space, but need to define
 * what exactly an invalid index is
 */

template <class T>
inline bool is_valid(T val) {return val < std::numeric_limits<T>::max();};

template <class T>
inline T invalid() {return std::numeric_limits<T>::max();};

template <>
inline Vector3us invalid<Vector3us>() {return {invalid<ushort>(), invalid<ushort>(), invalid<ushort>()};};

struct MxMeshVertex {
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;
};


/**
 * A partial face data structure, represents 1/2 of a triangular face. This represents the
 * side of the face that belongs to a cell struct. Partial faces share geomory (the vertex
 * indices at each apex), but have local attributes. 
 *
 * The vertex index ordering is different in in each side of the partial face pair. We construct
 * the index ordering on each half such that the vertex winding causes the face normal to 
 * point outwards from the cell. So, in each partial face half, the face normal points towards
 * the other cell. 
 */
struct MxPartialFace {

    /**
     * indices of the 3 vertices in the MxMesh that make up this partial face,
     * in the correct winding order
     */
    Vector3ui vertices;


    /**
     * index of the three neighbors of this face, these
     * neighbors are indices in the faces of the MxCell
     *
     * Don't expect ever to be more than 65,000 faces in a cell
     */
    Vector3us neighbors;

    /**
     * index of the neighboring cell that this partial face shares
     * a face with. We keep track of the neighboring cell here, this
     * lets the containing cell enumerate all neighboring cells via
     * connectivity through the partial faces.
     */
    uint neighborCell;

    /**
     * index in the neighboring cell's boundary array of the mirroring
     * partial face that matches this face.
     */
    ushort mirrorFace;

    double mass;


    /**
     * Last field in this struct, create a struct where the number of fields
     * are determined at runtime, and lets us allocate the entire struct in
     * a single contiguous memory block.
     *
     * But to get things working quickly, we'll just use a std::vector here
     * for now, lets us get the rest of the system up an running quickly
     * with just std::vector.
     */
    std::vector<double> fields;

    MxPartialFace(Vector3ui const& vert):
        vertices(vert),
        neighbors{invalid<ushort>(),invalid<ushort>(),invalid<ushort>()},
        neighborCell{invalid<uint>()},
        mirrorFace{invalid<ushort>()},
        mass{0}
    {
    }
};

/**
 * Represents a closed region of space. This corresponds spatially closely to the
 * concept of plant or animal cells, or rooms in a building. Each cell has a
 * well-defined boundary and a well-defined bounded spatial region. The boundary
 * of a cell is a fully-connected manifold surface.
 *
 * The MxCell maintains a set of local partial faces to represent the boundary surface
 * triangles. This cell knows what's inside and outside, hence it knows which way to order
 * the index winding so that the normal points the correct way.
 */
struct MxCell {
    /**
     * The MxMesh that this cell belongs to. Lots of method on this
     * cell require access to vertex info, simpler to add an ivar instead
     * of passing in a mesh pointer each time. 
     */
    MxMesh *mesh;

    /**
     * the closed set of faces that define the boundary of this cell
     */
    std::vector<MxPartialFace> boundary;

    /**
     * fields that belong to this cell.
     */
    std::vector<double> fields;

    /**
     * iterate over the boundary partial faces and connect them all together.
     * This should be used when a new cell is created from a set of indices.
     *
     * Returns true if the boundary connected successfully, false if the
     * boundary in non-manifold.
     */
    bool connectBoundary();

    enum VolumeMethod { ConvexTrapezoidSum, GeneralDivergence };

    /**
     * Calculate the volume of this cell. 
     */
    float volume(VolumeMethod vm = ConvexTrapezoidSum);

    /**
     * calculate the total area
     */
    float area();


    inline uint faceCount() { return boundary.size(); }

    /**
     * Even though we share position for vertices, each face has
     * different color, normal, etc... so need separate vertices
     * for each face. Not a problem because we compute these, don't
     * waste memory.
     */
    inline uint vertexCount() {return 3 * boundary.size(); };

    void writeBufferData();
};

#endif /* SRC_MXCELL_H_ */
