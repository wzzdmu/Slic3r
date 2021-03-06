#ifndef slic3r_FillBase_hpp_
#define slic3r_FillBase_hpp_

#include <assert.h>
#include <memory.h>
#include <float.h>
#include <stdint.h>

#include "../libslic3r.h"
#include "../BoundingBox.hpp"
#include "../ExPolygon.hpp"
#include "../Polyline.hpp"
#include "../PrintConfig.hpp"

namespace Slic3r {

class Surface;

struct FillParams
{
    public:
    FillParams() : density(0), dont_connect(false), dont_adjust(false), complete(false) {};

    // Fill density, fraction in <0, 1>
    float       density;

    // Don't connect the fill lines around the inner perimeter.
    bool        dont_connect;

    // Don't adjust spacing to fill the space evenly.
    bool        dont_adjust;

    // For Honeycomb.
    // we were requested to complete each loop;
    // in this case we don't try to make more continuous paths
    bool        complete;
};

class Fill
{
public:
    // Index of the layer.
    size_t      layer_id;
    
    // Z coordinate of the top print surface, in unscaled coordinates
    coordf_t    z;
    
    // in unscaled coordinates
    coordf_t    spacing;
    
    // in radians, ccw, 0 = East
    float       angle;
    
    // In scaled coordinates. Maximum lenght of a perimeter segment connecting two infill lines.
    // Used by the FillRectilinear2, FillGrid2, FillTriangles, FillStars and FillCubic.
    // If left to zero, the links will not be limited.
    coord_t     link_max_length;
    
    // In scaled coordinates. Used by the concentric infill pattern to clip the loops to create extrusion paths.
    coord_t     loop_clipping;
    
    // In scaled coordinates. Bounding box of the 2D projection of the object.
    BoundingBox bounding_box;

public:
    virtual ~Fill() {}

    static Fill* new_from_type(const InfillPattern type);
    static Fill* new_from_type(const std::string &type);

    void set_bounding_box(const BoundingBox &bb) { this->bounding_box = bb; }

    // Use bridge flow for the fill?
    virtual bool use_bridge_flow() const { return false; }

    // Do not sort the fill lines to optimize the print head path?
    virtual bool no_sort() const { return false; }

    // Perform the fill.
    virtual Polylines fill_surface(const Surface &surface, const FillParams &params);

    static coord_t adjust_solid_spacing(const coord_t width, const coord_t distance);

protected:
    Fill() :
        layer_id(size_t(-1)),
        z(0.f),
        spacing(0.f),
        angle(0),
        link_max_length(0),
        loop_clipping(0)
        {};

    // The expolygon may be modified by the method to avoid a copy.
    virtual void _fill_surface_single(
        const FillParams                &params, 
        unsigned int                     thickness_layers,
        const std::pair<float, Point>   &direction, 
        ExPolygon                       &expolygon, 
        Polylines*                      polylines_out) {};

    virtual float _layer_angle(size_t idx) const {
        return (idx % 2) == 0 ? (M_PI/2.) : 0;
    }

    std::pair<float, Point> _infill_direction(const Surface &surface) const;
};

} // namespace Slic3r

#endif // slic3r_FillBase_hpp_
