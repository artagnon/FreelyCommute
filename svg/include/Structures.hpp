#pragma once

namespace fc::svg
{
  enum PaintType
  {
    _PAINT_NONE = 0,
    _PAINT_COLOR = 1,
    _PAINT_LINEAR_GRADIENT = 2,
    _PAINT_RADIAL_GRADIENT = 3
  };

  enum SpreadType
  {
    _SPREAD_PAD = 0,
    _SPREAD_REFLECT = 1,
    _SPREAD_REPEAT = 2
  };

  enum LineJoin
  {
    _JOIN_MITER = 0,
    _JOIN_ROUND = 1,
    _JOIN_BEVEL = 2
  };

  enum LineCap
  {
    _CAP_BUTT = 0,
    _CAP_ROUND = 1,
    _CAP_SQUARE = 2
  };

  enum FillRule
  {
    _FILLRULE_NONZERO = 0,
    _FILLRULE_EVENODD = 1
  };

  enum Flags
  {
    _FLAGS_VISIBLE = 0x01
  };

  typedef struct GradientStop
  {
    unsigned int color;
    float offset;
  } GradientStop;

  typedef struct Gradient
  {
    float xform[6];
    char spread;
    float fx, fy;
    int nstops;
    GradientStop stops[1];
  } Gradient;

  typedef struct Paint
  {
    char type;
    union
    {
      unsigned int color;
      Gradient *gradient;
    };
  } Paint;

  typedef struct Path
  {
    float *pts;        // Cubic bezier points: x0,y0, [cpx1,cpx1,cpx2,cpy2,x1,y1], ...
    int npts;          // Total number of bezier points.
    char closed;       // Flag indicating if Shapes should be treated as closed.
    float bounds[4];   // Tight bounding box of the Shape [minx,miny,maxx,maxy].
    struct Path *next; // Pointer to next Path, or NULL if last element.
  } Path;

  typedef struct Shape
  {
    char id[64];              // Optional 'id' attr of the Shape or its group
    Paint fill;               // Fill Paint
    Paint stroke;             // Stroke Paint
    float opacity;            // Opacity of the Shape.
    float strokeWidth;        // Stroke width (scaled).
    float strokeDashOffset;   // Stroke dash offset (scaled).
    float strokeDashArray[8]; // Stroke dash array (scaled).
    char strokeDashCount;     // Number of dash values in dash array.
    char strokeLineJoin;      // Stroke join type.
    char strokeLineCap;       // Stroke cap type.
    float miterLimit;         // Miter limit
    char fillRule;            // Fill rule, see fillRule.
    unsigned char flags;      // Logical or of _FLAGS_* flags
    float bounds[4];          // Tight bounding box of the Shape [minx,miny,maxx,maxy].
    Path *paths;              // Linked list of Paths in the Image.
    struct Shape *next;       // Pointer to next Shape, or NULL if last element.
  } Shape;

  typedef struct Image
  {
    float width;   // Width of the Image.
    float height;  // Height of the Image.
    Shape *shapes; // Linked list of Shapes in the Image.
  } Image;

  enum GradientUnits
  {
    _USER_SPACE = 0,
    _OBJECT_SPACE = 1
  };

#define _MAX_DASHES 8

  enum Units
  {
    _UNITS_USER,
    _UNITS_PX,
    _UNITS_PT,
    _UNITS_PC,
    _UNITS_MM,
    _UNITS_CM,
    _UNITS_IN,
    _UNITS_PERCENT,
    _UNITS_EM,
    _UNITS_EX
  };

  typedef struct Coordinate
  {
    float value;
    int units;
  } Coordinate;

  typedef struct LinearData
  {
    Coordinate x1, y1, x2, y2;
  } LinearData;

  typedef struct RadialData
  {
    Coordinate cx, cy, r, fx, fy;
  } RadialData;

  typedef struct GradientData
  {
    char id[64];
    char ref[64];
    char type;
    union
    {
      LinearData linear;
      RadialData radial;
    };
    char spread;
    char units;
    float xform[6];
    int nstops;
    GradientStop *stops;
    struct GradientData *next;
  } GradientData;

  typedef struct Attrib
  {
    char id[64];
    float xform[6];
    unsigned int fillColor;
    unsigned int strokeColor;
    float opacity;
    float fillOpacity;
    float strokeOpacity;
    char fillGradient[64];
    char strokeGradient[64];
    float strokeWidth;
    float strokeDashOffset;
    float strokeDashArray[_MAX_DASHES];
    int strokeDashCount;
    char strokeLineJoin;
    char strokeLineCap;
    float miterLimit;
    char fillRule;
    float fontSize;
    unsigned int stopColor;
    float stopOpacity;
    float stopOffset;
    char hasFill;
    char hasStroke;
    char visible;
  } Attrib;

#define _MAX_ATTR 128

  typedef struct Parser
  {
    Attrib attr[_MAX_ATTR];
    int attrHead;
    float *pts;
    int npts;
    int cpts;
    Path *plist;
    Image *image;
    GradientData *gradients;
    Shape *shapesTail;
    float viewMinx, viewMiny, viewWidth, viewHeight;
    int alignX, alignY, alignType;
    float dpi;
    char PathFlag;
    char defsFlag;
  } Parser;

#define _PI (3.14159265358979323846264338327f)
#define _KAPPA90 (0.5522847493f) // Length proportional to radius of a cubic bezier handle for 90deg arcs.

#define _ALIGN_MIN 0
#define _ALIGN_MID 1
#define _ALIGN_MAX 2
#define _ALIGN_NONE 0
#define _ALIGN_MEET 1
#define _ALIGN_SLICE 2

#define _NOTUSED(v)                    \
  do                                   \
  {                                    \
    (void)(1 ? (void)0 : ((void)(v))); \
  } while (0)

#define _RGB(r, g, b) (((unsigned int)r) | ((unsigned int)g << 8) | ((unsigned int)b << 16))

  static inline float minf(float a, float b)
  {
    return a < b ? a : b;
  }
  static inline float maxf(float a, float b) { return a > b ? a : b; }

  // Forwards
  void deleteImage(Image *image);
}
