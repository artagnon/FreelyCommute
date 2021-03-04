#pragma once

#include <string>
#include <cctype>

#include "Structures.hpp"
#include "XmlParse.hpp"

namespace fc::svg
{
	static void xformIdentity(float *t)
	{
		t[0] = 1.0f;
		t[1] = 0.0f;
		t[2] = 0.0f;
		t[3] = 1.0f;
		t[4] = 0.0f;
		t[5] = 0.0f;
	}

	static void xformSetTranslation(float *t, float tx, float ty)
	{
		t[0] = 1.0f;
		t[1] = 0.0f;
		t[2] = 0.0f;
		t[3] = 1.0f;
		t[4] = tx;
		t[5] = ty;
	}

	static void xformSetScale(float *t, float sx, float sy)
	{
		t[0] = sx;
		t[1] = 0.0f;
		t[2] = 0.0f;
		t[3] = sy;
		t[4] = 0.0f;
		t[5] = 0.0f;
	}

	static void xformSetSkewX(float *t, float a)
	{
		t[0] = 1.0f;
		t[1] = 0.0f;
		t[2] = tanf(a);
		t[3] = 1.0f;
		t[4] = 0.0f;
		t[5] = 0.0f;
	}

	static void xformSetSkewY(float *t, float a)
	{
		t[0] = 1.0f;
		t[1] = tanf(a);
		t[2] = 0.0f;
		t[3] = 1.0f;
		t[4] = 0.0f;
		t[5] = 0.0f;
	}

	static void xformSetRotation(float *t, float a)
	{
		float cs = cosf(a), sn = sinf(a);
		t[0] = cs;
		t[1] = sn;
		t[2] = -sn;
		t[3] = cs;
		t[4] = 0.0f;
		t[5] = 0.0f;
	}

	static void xformMultiply(float *t, float *s)
	{
		float t0 = t[0] * s[0] + t[1] * s[2];
		float t2 = t[2] * s[0] + t[3] * s[2];
		float t4 = t[4] * s[0] + t[5] * s[2] + s[4];
		t[1] = t[0] * s[1] + t[1] * s[3];
		t[3] = t[2] * s[1] + t[3] * s[3];
		t[5] = t[4] * s[1] + t[5] * s[3] + s[5];
		t[0] = t0;
		t[2] = t2;
		t[4] = t4;
	}

	static void xformInverse(float *inv, float *t)
	{
		double invdet, det = (double)t[0] * t[3] - (double)t[2] * t[1];
		if (det > -1e-6 && det < 1e-6)
		{
			xformIdentity(t);
			return;
		}
		invdet = 1.0 / det;
		inv[0] = (float)(t[3] * invdet);
		inv[2] = (float)(-t[2] * invdet);
		inv[4] = (float)(((double)t[2] * t[5] - (double)t[3] * t[4]) * invdet);
		inv[1] = (float)(-t[1] * invdet);
		inv[3] = (float)(t[0] * invdet);
		inv[5] = (float)(((double)t[1] * t[4] - (double)t[0] * t[5]) * invdet);
	}

	static void xformPremultiply(float *t, float *s)
	{
		float s2[6];
		memcpy(s2, s, sizeof(float) * 6);
		xformMultiply(s2, t);
		memcpy(t, s2, sizeof(float) * 6);
	}

	static void xformPoint(float *dx, float *dy, float x, float y, float *t)
	{
		*dx = x * t[0] + y * t[2] + t[4];
		*dy = x * t[1] + y * t[3] + t[5];
	}

	static void xformVec(float *dx, float *dy, float x, float y, float *t)
	{
		*dx = x * t[0] + y * t[2];
		*dy = x * t[1] + y * t[3];
	}

#define _EPSILON (1e-12)

	static int ptInBounds(float *pt, float *bounds)
	{
		return pt[0] >= bounds[0] && pt[0] <= bounds[2] && pt[1] >= bounds[1] && pt[1] <= bounds[3];
	}

	static double evalBezier(double t, double p0, double p1, double p2, double p3)
	{
		double it = 1.0 - t;
		return it * it * it * p0 + 3.0 * it * it * t * p1 + 3.0 * it * t * t * p2 + t * t * t * p3;
	}

	static void curveBounds(float *bounds, float *curve)
	{
		int i, j, count;
		double roots[2], a, b, c, b2ac, t, v;
		float *v0 = &curve[0];
		float *v1 = &curve[2];
		float *v2 = &curve[4];
		float *v3 = &curve[6];

		// Start the bounding box by end points
		bounds[0] = minf(v0[0], v3[0]);
		bounds[1] = minf(v0[1], v3[1]);
		bounds[2] = maxf(v0[0], v3[0]);
		bounds[3] = maxf(v0[1], v3[1]);

		// Bezier curve fits inside the convex hull of it's control points.
		// If control points are inside the bounds, we're done.
		if (ptInBounds(v1, bounds) && ptInBounds(v2, bounds))
			return;

		// Add bezier curve inflection points in X and Y.
		for (i = 0; i < 2; i++)
		{
			a = -3.0 * v0[i] + 9.0 * v1[i] - 9.0 * v2[i] + 3.0 * v3[i];
			b = 6.0 * v0[i] - 12.0 * v1[i] + 6.0 * v2[i];
			c = 3.0 * v1[i] - 3.0 * v0[i];
			count = 0;
			if (fabs(a) < _EPSILON)
			{
				if (fabs(b) > _EPSILON)
				{
					t = -c / b;
					if (t > _EPSILON && t < 1.0 - _EPSILON)
						roots[count++] = t;
				}
			}
			else
			{
				b2ac = b * b - 4.0 * c * a;
				if (b2ac > _EPSILON)
				{
					t = (-b + sqrt(b2ac)) / (2.0 * a);
					if (t > _EPSILON && t < 1.0 - _EPSILON)
						roots[count++] = t;
					t = (-b - sqrt(b2ac)) / (2.0 * a);
					if (t > _EPSILON && t < 1.0 - _EPSILON)
						roots[count++] = t;
				}
			}
			for (j = 0; j < count; j++)
			{
				v = evalBezier(roots[j], v0[i], v1[i], v2[i], v3[i]);
				bounds[0 + i] = minf(bounds[0 + i], (float)v);
				bounds[2 + i] = maxf(bounds[2 + i], (float)v);
			}
		}
	}

	static Parser *createParser()
	{
		Parser *p;
		p = (Parser *)malloc(sizeof(Parser));
		if (p == nullptr)
			goto error;
		memset(p, 0, sizeof(Parser));

		p->image = (Image *)malloc(sizeof(Image));
		if (p->image == nullptr)
			goto error;
		memset(p->image, 0, sizeof(Image));

		// Init style
		xformIdentity(p->attr[0].xform);
		memset(p->attr[0].id, 0, sizeof p->attr[0].id);
		p->attr[0].fillColor = _RGB(0, 0, 0);
		p->attr[0].strokeColor = _RGB(0, 0, 0);
		p->attr[0].opacity = 1;
		p->attr[0].fillOpacity = 1;
		p->attr[0].strokeOpacity = 1;
		p->attr[0].stopOpacity = 1;
		p->attr[0].strokeWidth = 1;
		p->attr[0].strokeLineJoin = _JOIN_MITER;
		p->attr[0].strokeLineCap = _CAP_BUTT;
		p->attr[0].miterLimit = 4;
		p->attr[0].fillRule = _FILLRULE_NONZERO;
		p->attr[0].hasFill = 1;
		p->attr[0].visible = 1;

		return p;

	error:
		if (p)
		{
			if (p->image)
				free(p->image);
			free(p);
		}
		return nullptr;
	}

	static void deletePaths(Path *path)
	{
		while (path)
		{
			Path *next = path->next;
			if (path->pts != nullptr)
				free(path->pts);
			free(path);
			path = next;
		}
	}

	static void deletePaint(Paint *paint)
	{
		if (paint->type == _PAINT_LINEAR_GRADIENT || paint->type == _PAINT_RADIAL_GRADIENT)
			free(paint->gradient);
	}

	static void deleteGradientData(GradientData *grad)
	{
		GradientData *next;
		while (grad != nullptr)
		{
			next = grad->next;
			free(grad->stops);
			free(grad);
			grad = next;
		}
	}

	static void deleteParser(Parser *p)
	{
		if (p != nullptr)
		{
			deletePaths(p->plist);
			deleteGradientData(p->gradients);
			deleteImage(p->image);
			free(p->pts);
			free(p);
		}
	}

	static void resetPath(Parser *p)
	{
		p->npts = 0;
	}

	static void addPoint(Parser *p, float x, float y)
	{
		if (p->npts + 1 > p->cpts)
		{
			p->cpts = p->cpts ? p->cpts * 2 : 8;
			p->pts = (float *)realloc(p->pts, p->cpts * 2 * sizeof(float));
			if (!p->pts)
				return;
		}
		p->pts[p->npts * 2 + 0] = x;
		p->pts[p->npts * 2 + 1] = y;
		p->npts++;
	}

	static void moveTo(Parser *p, float x, float y)
	{
		if (p->npts > 0)
		{
			p->pts[(p->npts - 1) * 2 + 0] = x;
			p->pts[(p->npts - 1) * 2 + 1] = y;
		}
		else
		{
			addPoint(p, x, y);
		}
	}

	static void lineTo(Parser *p, float x, float y)
	{
		float px, py, dx, dy;
		if (p->npts > 0)
		{
			px = p->pts[(p->npts - 1) * 2 + 0];
			py = p->pts[(p->npts - 1) * 2 + 1];
			dx = x - px;
			dy = y - py;
			addPoint(p, px + dx / 3.0f, py + dy / 3.0f);
			addPoint(p, x - dx / 3.0f, y - dy / 3.0f);
			addPoint(p, x, y);
		}
	}

	static void cubicBezTo(Parser *p, float cpx1, float cpy1, float cpx2, float cpy2, float x, float y)
	{
		if (p->npts > 0)
		{
			addPoint(p, cpx1, cpy1);
			addPoint(p, cpx2, cpy2);
			addPoint(p, x, y);
		}
	}

	static Attrib *getAttr(Parser *p)
	{
		return &p->attr[p->attrHead];
	}

	static void pushAttr(Parser *p)
	{
		if (p->attrHead < _MAX_ATTR - 1)
		{
			p->attrHead++;
			memcpy(&p->attr[p->attrHead], &p->attr[p->attrHead - 1], sizeof(Attrib));
		}
	}

	static void popAttr(Parser *p)
	{
		if (p->attrHead > 0)
			p->attrHead--;
	}

	static float actualOrigX(Parser *p)
	{
		return p->viewMinx;
	}

	static float actualOrigY(Parser *p)
	{
		return p->viewMiny;
	}

	static float actualWidth(Parser *p)
	{
		return p->viewWidth;
	}

	static float actualHeight(Parser *p)
	{
		return p->viewHeight;
	}

	static float actualLength(Parser *p)
	{
		float w = actualWidth(p), h = actualHeight(p);
		return sqrtf(w * w + h * h) / sqrtf(2.0f);
	}

	static float convertToPixels(Parser *p, Coordinate c, float orig, float length)
	{
		Attrib *attr = getAttr(p);
		switch (c.units)
		{
		case _UNITS_USER:
			return c.value;
		case _UNITS_PX:
			return c.value;
		case _UNITS_PT:
			return c.value / 72.0f * p->dpi;
		case _UNITS_PC:
			return c.value / 6.0f * p->dpi;
		case _UNITS_MM:
			return c.value / 25.4f * p->dpi;
		case _UNITS_CM:
			return c.value / 2.54f * p->dpi;
		case _UNITS_IN:
			return c.value * p->dpi;
		case _UNITS_EM:
			return c.value * attr->fontSize;
		case _UNITS_EX:
			return c.value * attr->fontSize * 0.52f; // x-height of Helvetica.
		case _UNITS_PERCENT:
			return orig + c.value / 100.0f * length;
		default:
			return c.value;
		}
		return c.value;
	}

	static GradientData *findGradientData(Parser *p, const char *id)
	{
		GradientData *grad = p->gradients;
		if (id == nullptr || *id == '\0')
			return nullptr;
		while (grad != nullptr)
		{
			if (strcmp(grad->id, id) == 0)
				return grad;
			grad = grad->next;
		}
		return nullptr;
	}

	static Gradient *createGradient(Parser *p, const char *id, const float *localBounds, char *paintType)
	{
		Attrib *attr = getAttr(p);
		GradientData *data = nullptr;
		GradientData *ref = nullptr;
		GradientStop *stops = nullptr;
		Gradient *grad;
		float ox, oy, sw, sh, sl;
		int nstops = 0;
		int refIter;

		data = findGradientData(p, id);
		if (data == nullptr)
			return nullptr;

		// TODO: use ref to fill in all unset values too.
		ref = data;
		refIter = 0;
		while (ref != nullptr)
		{
			GradientData *nextRef = nullptr;
			if (stops == nullptr && ref->stops != nullptr)
			{
				stops = ref->stops;
				nstops = ref->nstops;
				break;
			}
			nextRef = findGradientData(p, ref->ref);
			if (nextRef == ref)
				break; // prevent infite loops on malformed data
			ref = nextRef;
			refIter++;
			if (refIter > 32)
				break; // prevent infite loops on malformed data
		}
		if (stops == nullptr)
			return nullptr;

		grad = (Gradient *)malloc(sizeof(Gradient) + sizeof(GradientStop) * (nstops - 1));
		if (grad == nullptr)
			return nullptr;

		// The shape width and height.
		if (data->units == _OBJECT_SPACE)
		{
			ox = localBounds[0];
			oy = localBounds[1];
			sw = localBounds[2] - localBounds[0];
			sh = localBounds[3] - localBounds[1];
		}
		else
		{
			ox = actualOrigX(p);
			oy = actualOrigY(p);
			sw = actualWidth(p);
			sh = actualHeight(p);
		}
		sl = sqrtf(sw * sw + sh * sh) / sqrtf(2.0f);

		if (data->type == _PAINT_LINEAR_GRADIENT)
		{
			float x1, y1, x2, y2, dx, dy;
			x1 = convertToPixels(p, data->linear.x1, ox, sw);
			y1 = convertToPixels(p, data->linear.y1, oy, sh);
			x2 = convertToPixels(p, data->linear.x2, ox, sw);
			y2 = convertToPixels(p, data->linear.y2, oy, sh);
			// Calculate transform aligned to the line
			dx = x2 - x1;
			dy = y2 - y1;
			grad->xform[0] = dy;
			grad->xform[1] = -dx;
			grad->xform[2] = dx;
			grad->xform[3] = dy;
			grad->xform[4] = x1;
			grad->xform[5] = y1;
		}
		else
		{
			float cx, cy, fx, fy, r;
			cx = convertToPixels(p, data->radial.cx, ox, sw);
			cy = convertToPixels(p, data->radial.cy, oy, sh);
			fx = convertToPixels(p, data->radial.fx, ox, sw);
			fy = convertToPixels(p, data->radial.fy, oy, sh);
			r = convertToPixels(p, data->radial.r, 0, sl);
			// Calculate transform aligned to the circle
			grad->xform[0] = r;
			grad->xform[1] = 0;
			grad->xform[2] = 0;
			grad->xform[3] = r;
			grad->xform[4] = cx;
			grad->xform[5] = cy;
			grad->fx = fx / r;
			grad->fy = fy / r;
		}

		xformMultiply(grad->xform, data->xform);
		xformMultiply(grad->xform, attr->xform);

		grad->spread = data->spread;
		memcpy(grad->stops, stops, nstops * sizeof(GradientStop));
		grad->nstops = nstops;

		*paintType = data->type;

		return grad;
	}

	static float getAverageScale(float *t)
	{
		float sx = sqrtf(t[0] * t[0] + t[2] * t[2]);
		float sy = sqrtf(t[1] * t[1] + t[3] * t[3]);
		return (sx + sy) * 0.5f;
	}

	static void getLocalBounds(float *bounds, Shape *shape, float *xform)
	{
		Path *path;
		float curve[4 * 2], cBounds[4];
		int i, first = 1;
		for (path = shape->paths; path != nullptr; path = path->next)
		{
			xformPoint(&curve[0], &curve[1], path->pts[0], path->pts[1], xform);
			for (i = 0; i < path->npts - 1; i += 3)
			{
				xformPoint(&curve[2], &curve[3], path->pts[(i + 1) * 2], path->pts[(i + 1) * 2 + 1], xform);
				xformPoint(&curve[4], &curve[5], path->pts[(i + 2) * 2], path->pts[(i + 2) * 2 + 1], xform);
				xformPoint(&curve[6], &curve[7], path->pts[(i + 3) * 2], path->pts[(i + 3) * 2 + 1], xform);
				curveBounds(cBounds, curve);
				if (first)
				{
					bounds[0] = cBounds[0];
					bounds[1] = cBounds[1];
					bounds[2] = cBounds[2];
					bounds[3] = cBounds[3];
					first = 0;
				}
				else
				{
					bounds[0] = minf(bounds[0], cBounds[0]);
					bounds[1] = minf(bounds[1], cBounds[1]);
					bounds[2] = maxf(bounds[2], cBounds[2]);
					bounds[3] = maxf(bounds[3], cBounds[3]);
				}
				curve[0] = curve[6];
				curve[1] = curve[7];
			}
		}
	}

	static void addShape(Parser *p)
	{
		Attrib *attr = getAttr(p);
		float scale = 1.0f;
		Shape *shape;
		Path *path;
		int i;

		if (p->plist == nullptr)
			return;

		shape = (Shape *)malloc(sizeof(Shape));
		if (shape == nullptr)
			goto error;
		memset(shape, 0, sizeof(Shape));

		memcpy(shape->id, attr->id, sizeof shape->id);
		scale = getAverageScale(attr->xform);
		shape->strokeWidth = attr->strokeWidth * scale;
		shape->strokeDashOffset = attr->strokeDashOffset * scale;
		shape->strokeDashCount = (char)attr->strokeDashCount;
		for (i = 0; i < attr->strokeDashCount; i++)
			shape->strokeDashArray[i] = attr->strokeDashArray[i] * scale;
		shape->strokeLineJoin = attr->strokeLineJoin;
		shape->strokeLineCap = attr->strokeLineCap;
		shape->miterLimit = attr->miterLimit;
		shape->fillRule = attr->fillRule;
		shape->opacity = attr->opacity;

		shape->paths = p->plist;
		p->plist = nullptr;

		// Calculate shape bounds
		shape->bounds[0] = shape->paths->bounds[0];
		shape->bounds[1] = shape->paths->bounds[1];
		shape->bounds[2] = shape->paths->bounds[2];
		shape->bounds[3] = shape->paths->bounds[3];
		for (path = shape->paths->next; path != nullptr; path = path->next)
		{
			shape->bounds[0] = minf(shape->bounds[0], path->bounds[0]);
			shape->bounds[1] = minf(shape->bounds[1], path->bounds[1]);
			shape->bounds[2] = maxf(shape->bounds[2], path->bounds[2]);
			shape->bounds[3] = maxf(shape->bounds[3], path->bounds[3]);
		}

		// Set fill
		if (attr->hasFill == 0)
		{
			shape->fill.type = _PAINT_NONE;
		}
		else if (attr->hasFill == 1)
		{
			shape->fill.type = _PAINT_COLOR;
			shape->fill.color = attr->fillColor;
			shape->fill.color |= (unsigned int)(attr->fillOpacity * 255) << 24;
		}
		else if (attr->hasFill == 2)
		{
			float inv[6], localBounds[4];
			xformInverse(inv, attr->xform);
			getLocalBounds(localBounds, shape, inv);
			shape->fill.gradient = createGradient(p, attr->fillGradient, localBounds, &shape->fill.type);
			if (shape->fill.gradient == nullptr)
			{
				shape->fill.type = _PAINT_NONE;
			}
		}

		// Set stroke
		if (attr->hasStroke == 0)
		{
			shape->stroke.type = _PAINT_NONE;
		}
		else if (attr->hasStroke == 1)
		{
			shape->stroke.type = _PAINT_COLOR;
			shape->stroke.color = attr->strokeColor;
			shape->stroke.color |= (unsigned int)(attr->strokeOpacity * 255) << 24;
		}
		else if (attr->hasStroke == 2)
		{
			float inv[6], localBounds[4];
			xformInverse(inv, attr->xform);
			getLocalBounds(localBounds, shape, inv);
			shape->stroke.gradient = createGradient(p, attr->strokeGradient, localBounds, &shape->stroke.type);
			if (shape->stroke.gradient == nullptr)
				shape->stroke.type = _PAINT_NONE;
		}

		// Set flags
		shape->flags = (attr->visible ? _FLAGS_VISIBLE : 0x00);

		// Add to tail
		if (p->image->shapes == nullptr)
			p->image->shapes = shape;
		else
			p->shapesTail->next = shape;
		p->shapesTail = shape;

		return;

	error:
		if (shape)
			free(shape);
	}

	static void addPath(Parser *p, char closed)
	{
		Attrib *attr = getAttr(p);
		Path *path = nullptr;
		float bounds[4];
		float *curve;
		int i;

		if (p->npts < 4)
			return;

		if (closed)
			lineTo(p, p->pts[0], p->pts[1]);

		// Expect 1 + N*3 points (N = number of cubic bezier segments).
		if ((p->npts % 3) != 1)
			return;

		path = (Path *)malloc(sizeof(Path));
		if (path == nullptr)
			goto error;
		memset(path, 0, sizeof(Path));

		path->pts = (float *)malloc(p->npts * 2 * sizeof(float));
		if (path->pts == nullptr)
			goto error;
		path->closed = closed;
		path->npts = p->npts;

		// Transform Path.
		for (i = 0; i < p->npts; ++i)
			xformPoint(&path->pts[i * 2], &path->pts[i * 2 + 1], p->pts[i * 2], p->pts[i * 2 + 1], attr->xform);

		// Find bounds
		for (i = 0; i < path->npts - 1; i += 3)
		{
			curve = &path->pts[i * 2];
			curveBounds(bounds, curve);
			if (i == 0)
			{
				path->bounds[0] = bounds[0];
				path->bounds[1] = bounds[1];
				path->bounds[2] = bounds[2];
				path->bounds[3] = bounds[3];
			}
			else
			{
				path->bounds[0] = minf(path->bounds[0], bounds[0]);
				path->bounds[1] = minf(path->bounds[1], bounds[1]);
				path->bounds[2] = maxf(path->bounds[2], bounds[2]);
				path->bounds[3] = maxf(path->bounds[3], bounds[3]);
			}
		}

		path->next = p->plist;
		p->plist = path;

		return;

	error:
		if (path != nullptr)
		{
			if (path->pts != nullptr)
				free(path->pts);
			free(path);
		}
	}

	// We roll our own string to float because the std library one uses locale and messes things up.
	static double atof(const char *s)
	{
		char *cur = (char *)s;
		char *end = nullptr;
		double res = 0.0, sign = 1.0;
		long long intPart = 0, fracPart = 0;
		char hasIntPart = 0, hasFracPart = 0;

		// Parse optional sign
		if (*cur == '+')
		{
			cur++;
		}
		else if (*cur == '-')
		{
			sign = -1;
			cur++;
		}

		// Parse integer part
		if (std::isdigit(*cur))
		{
			// Parse digit sequence
			intPart = strtoll(cur, &end, 10);
			if (cur != end)
			{
				res = (double)intPart;
				hasIntPart = 1;
				cur = end;
			}
		}

		// Parse fractional part.
		if (*cur == '.')
		{
			cur++; // Skip '.'
			if (std::isdigit(*cur))
			{
				// Parse digit sequence
				fracPart = strtoll(cur, &end, 10);
				if (cur != end)
				{
					res += (double)fracPart / pow(10.0, (double)(end - cur));
					hasFracPart = 1;
					cur = end;
				}
			}
		}

		// A valid number should have integer or fractional part.
		if (!hasIntPart && !hasFracPart)
			return 0.0;

		// Parse optional exponent
		if (*cur == 'e' || *cur == 'E')
		{
			long expPart = 0;
			cur++;													 // skip 'E'
			expPart = strtol(cur, &end, 10); // Parse digit sequence with sign
			if (cur != end)
			{
				res *= pow(10.0, (double)expPart);
			}
		}

		return res * sign;
	}

	static const char *parseNumber(const char *s, char *it, const int size)
	{
		const int last = size - 1;
		int i = 0;

		// sign
		if (*s == '-' || *s == '+')
		{
			if (i < last)
				it[i++] = *s;
			s++;
		}
		// integer part
		while (*s && std::isdigit(*s))
		{
			if (i < last)
				it[i++] = *s;
			s++;
		}
		if (*s == '.')
		{
			// decimal point
			if (i < last)
				it[i++] = *s;
			s++;
			// fraction part
			while (*s && std::isdigit(*s))
			{
				if (i < last)
					it[i++] = *s;
				s++;
			}
		}
		// exponent
		if ((*s == 'e' || *s == 'E') && (s[1] != 'm' && s[1] != 'x'))
		{
			if (i < last)
				it[i++] = *s;
			s++;
			if (*s == '-' || *s == '+')
			{
				if (i < last)
					it[i++] = *s;
				s++;
			}
			while (*s && std::isdigit(*s))
			{
				if (i < last)
					it[i++] = *s;
				s++;
			}
		}
		it[i] = '\0';

		return s;
	}

	static const char *getNextPathItem(const char *s, char *it)
	{
		it[0] = '\0';
		// Skip white spaces and commas
		while (*s && (isspace(*s) || *s == ','))
			s++;
		if (!*s)
			return s;
		if (*s == '-' || *s == '+' || *s == '.' || std::isdigit(*s))
		{
			s = parseNumber(s, it, 64);
		}
		else
		{
			// Parse command
			it[0] = *s++;
			it[1] = '\0';
			return s;
		}

		return s;
	}

	static unsigned int parseColorHex(const char *str)
	{
		unsigned int c = 0, r = 0, g = 0, b = 0;
		int n = 0;
		str++; // skip #
		// Calculate number of characters.
		while (str[n] && !isspace(str[n]))
			n++;
		if (n == 6)
		{
			sscanf(str, "%x", &c);
		}
		else if (n == 3)
		{
			sscanf(str, "%x", &c);
			c = (c & 0xf) | ((c & 0xf0) << 4) | ((c & 0xf00) << 8);
			c |= c << 4;
		}
		r = (c >> 16) & 0xff;
		g = (c >> 8) & 0xff;
		b = c & 0xff;
		return _RGB(r, g, b);
	}

	static unsigned int parseColorRGB(const char *str)
	{
		int r = -1, g = -1, b = -1;
		char s1[32] = "", s2[32] = "";
		sscanf(str + 4, "%d%[%%, \t]%d%[%%, \t]%d", &r, s1, &g, s2, &b);
		if (strchr(s1, '%'))
		{
			return _RGB((r * 255) / 100, (g * 255) / 100, (b * 255) / 100);
		}
		else
		{
			return _RGB(r, g, b);
		}
	}

	typedef struct NamedColor
	{
		const char *name;
		unsigned int color;
	} NamedColor;

	NamedColor colors[] = {

			{"red", _RGB(255, 0, 0)},
			{"green", _RGB(0, 128, 0)},
			{"blue", _RGB(0, 0, 255)},
			{"yellow", _RGB(255, 255, 0)},
			{"cyan", _RGB(0, 255, 255)},
			{"magenta", _RGB(255, 0, 255)},
			{"black", _RGB(0, 0, 0)},
			{"grey", _RGB(128, 128, 128)},
			{"gray", _RGB(128, 128, 128)},
			{"white", _RGB(255, 255, 255)},
			{"aliceblue", _RGB(240, 248, 255)},
			{"antiquewhite", _RGB(250, 235, 215)},
			{"aqua", _RGB(0, 255, 255)},
			{"aquamarine", _RGB(127, 255, 212)},
			{"azure", _RGB(240, 255, 255)},
			{"beige", _RGB(245, 245, 220)},
			{"bisque", _RGB(255, 228, 196)},
			{"blanchedalmond", _RGB(255, 235, 205)},
			{"blueviolet", _RGB(138, 43, 226)},
			{"brown", _RGB(165, 42, 42)},
			{"burlywood", _RGB(222, 184, 135)},
			{"cadetblue", _RGB(95, 158, 160)},
			{"chartreuse", _RGB(127, 255, 0)},
			{"chocolate", _RGB(210, 105, 30)},
			{"coral", _RGB(255, 127, 80)},
			{"cornflowerblue", _RGB(100, 149, 237)},
			{"cornsilk", _RGB(255, 248, 220)},
			{"crimson", _RGB(220, 20, 60)},
			{"darkblue", _RGB(0, 0, 139)},
			{"darkcyan", _RGB(0, 139, 139)},
			{"darkgoldenrod", _RGB(184, 134, 11)},
			{"darkgray", _RGB(169, 169, 169)},
			{"darkgreen", _RGB(0, 100, 0)},
			{"darkgrey", _RGB(169, 169, 169)},
			{"darkkhaki", _RGB(189, 183, 107)},
			{"darkmagenta", _RGB(139, 0, 139)},
			{"darkolivegreen", _RGB(85, 107, 47)},
			{"darkorange", _RGB(255, 140, 0)},
			{"darkorchid", _RGB(153, 50, 204)},
			{"darkred", _RGB(139, 0, 0)},
			{"darksalmon", _RGB(233, 150, 122)},
			{"darkseagreen", _RGB(143, 188, 143)},
			{"darkslateblue", _RGB(72, 61, 139)},
			{"darkslategray", _RGB(47, 79, 79)},
			{"darkslategrey", _RGB(47, 79, 79)},
			{"darkturquoise", _RGB(0, 206, 209)},
			{"darkviolet", _RGB(148, 0, 211)},
			{"deeppink", _RGB(255, 20, 147)},
			{"deepskyblue", _RGB(0, 191, 255)},
			{"dimgray", _RGB(105, 105, 105)},
			{"dimgrey", _RGB(105, 105, 105)},
			{"dodgerblue", _RGB(30, 144, 255)},
			{"firebrick", _RGB(178, 34, 34)},
			{"floralwhite", _RGB(255, 250, 240)},
			{"forestgreen", _RGB(34, 139, 34)},
			{"fuchsia", _RGB(255, 0, 255)},
			{"gainsboro", _RGB(220, 220, 220)},
			{"ghostwhite", _RGB(248, 248, 255)},
			{"gold", _RGB(255, 215, 0)},
			{"goldenrod", _RGB(218, 165, 32)},
			{"greenyellow", _RGB(173, 255, 47)},
			{"honeydew", _RGB(240, 255, 240)},
			{"hotpink", _RGB(255, 105, 180)},
			{"indianred", _RGB(205, 92, 92)},
			{"indigo", _RGB(75, 0, 130)},
			{"ivory", _RGB(255, 255, 240)},
			{"khaki", _RGB(240, 230, 140)},
			{"lavender", _RGB(230, 230, 250)},
			{"lavenderblush", _RGB(255, 240, 245)},
			{"lawngreen", _RGB(124, 252, 0)},
			{"lemonchiffon", _RGB(255, 250, 205)},
			{"lightblue", _RGB(173, 216, 230)},
			{"lightcoral", _RGB(240, 128, 128)},
			{"lightcyan", _RGB(224, 255, 255)},
			{"lightgoldenrodyellow", _RGB(250, 250, 210)},
			{"lightgray", _RGB(211, 211, 211)},
			{"lightgreen", _RGB(144, 238, 144)},
			{"lightgrey", _RGB(211, 211, 211)},
			{"lightpink", _RGB(255, 182, 193)},
			{"lightsalmon", _RGB(255, 160, 122)},
			{"lightseagreen", _RGB(32, 178, 170)},
			{"lightskyblue", _RGB(135, 206, 250)},
			{"lightslategray", _RGB(119, 136, 153)},
			{"lightslategrey", _RGB(119, 136, 153)},
			{"lightsteelblue", _RGB(176, 196, 222)},
			{"lightyellow", _RGB(255, 255, 224)},
			{"lime", _RGB(0, 255, 0)},
			{"limegreen", _RGB(50, 205, 50)},
			{"linen", _RGB(250, 240, 230)},
			{"maroon", _RGB(128, 0, 0)},
			{"mediumaquamarine", _RGB(102, 205, 170)},
			{"mediumblue", _RGB(0, 0, 205)},
			{"mediumorchid", _RGB(186, 85, 211)},
			{"mediumpurple", _RGB(147, 112, 219)},
			{"mediumseagreen", _RGB(60, 179, 113)},
			{"mediumslateblue", _RGB(123, 104, 238)},
			{"mediumspringgreen", _RGB(0, 250, 154)},
			{"mediumturquoise", _RGB(72, 209, 204)},
			{"mediumvioletred", _RGB(199, 21, 133)},
			{"midnightblue", _RGB(25, 25, 112)},
			{"mintcream", _RGB(245, 255, 250)},
			{"mistyrose", _RGB(255, 228, 225)},
			{"moccasin", _RGB(255, 228, 181)},
			{"navajowhite", _RGB(255, 222, 173)},
			{"navy", _RGB(0, 0, 128)},
			{"oldlace", _RGB(253, 245, 230)},
			{"olive", _RGB(128, 128, 0)},
			{"olivedrab", _RGB(107, 142, 35)},
			{"orange", _RGB(255, 165, 0)},
			{"orangered", _RGB(255, 69, 0)},
			{"orchid", _RGB(218, 112, 214)},
			{"palegoldenrod", _RGB(238, 232, 170)},
			{"palegreen", _RGB(152, 251, 152)},
			{"paleturquoise", _RGB(175, 238, 238)},
			{"palevioletred", _RGB(219, 112, 147)},
			{"papayawhip", _RGB(255, 239, 213)},
			{"peachpuff", _RGB(255, 218, 185)},
			{"peru", _RGB(205, 133, 63)},
			{"pink", _RGB(255, 192, 203)},
			{"plum", _RGB(221, 160, 221)},
			{"powderblue", _RGB(176, 224, 230)},
			{"purple", _RGB(128, 0, 128)},
			{"rosybrown", _RGB(188, 143, 143)},
			{"royalblue", _RGB(65, 105, 225)},
			{"saddlebrown", _RGB(139, 69, 19)},
			{"salmon", _RGB(250, 128, 114)},
			{"sandybrown", _RGB(244, 164, 96)},
			{"seagreen", _RGB(46, 139, 87)},
			{"seashell", _RGB(255, 245, 238)},
			{"sienna", _RGB(160, 82, 45)},
			{"silver", _RGB(192, 192, 192)},
			{"skyblue", _RGB(135, 206, 235)},
			{"slateblue", _RGB(106, 90, 205)},
			{"slategray", _RGB(112, 128, 144)},
			{"slategrey", _RGB(112, 128, 144)},
			{"snow", _RGB(255, 250, 250)},
			{"springgreen", _RGB(0, 255, 127)},
			{"steelblue", _RGB(70, 130, 180)},
			{"tan", _RGB(210, 180, 140)},
			{"teal", _RGB(0, 128, 128)},
			{"thistle", _RGB(216, 191, 216)},
			{"tomato", _RGB(255, 99, 71)},
			{"turquoise", _RGB(64, 224, 208)},
			{"violet", _RGB(238, 130, 238)},
			{"wheat", _RGB(245, 222, 179)},
			{"whitesmoke", _RGB(245, 245, 245)},
			{"yellowgreen", _RGB(154, 205, 50)},
	};

	static unsigned int parseColorName(const char *str)
	{
		int i, ncolors = sizeof(colors) / sizeof(NamedColor);

		for (i = 0; i < ncolors; i++)
		{
			if (strcmp(colors[i].name, str) == 0)
			{
				return colors[i].color;
			}
		}

		return _RGB(128, 128, 128);
	}

	static unsigned int parseColor(const char *str)
	{
		size_t len = 0;
		while (*str == ' ')
			++str;
		len = strlen(str);
		if (len >= 1 && *str == '#')
			return parseColorHex(str);
		else if (len >= 4 && str[0] == 'r' && str[1] == 'g' && str[2] == 'b' && str[3] == '(')
			return parseColorRGB(str);
		return parseColorName(str);
	}

	static float parseOpacity(const char *str)
	{
		float val = atof(str);
		if (val < 0.0f)
			val = 0.0f;
		if (val > 1.0f)
			val = 1.0f;
		return val;
	}

	static float parseMiterLimit(const char *str)
	{
		float val = atof(str);
		if (val < 0.0f)
			val = 0.0f;
		return val;
	}

	static int parseUnits(const char *Units)
	{
		if (Units[0] == 'p' && Units[1] == 'x')
			return _UNITS_PX;
		else if (Units[0] == 'p' && Units[1] == 't')
			return _UNITS_PT;
		else if (Units[0] == 'p' && Units[1] == 'c')
			return _UNITS_PC;
		else if (Units[0] == 'm' && Units[1] == 'm')
			return _UNITS_MM;
		else if (Units[0] == 'c' && Units[1] == 'm')
			return _UNITS_CM;
		else if (Units[0] == 'i' && Units[1] == 'n')
			return _UNITS_IN;
		else if (Units[0] == '%')
			return _UNITS_PERCENT;
		else if (Units[0] == 'e' && Units[1] == 'm')
			return _UNITS_EM;
		else if (Units[0] == 'e' && Units[1] == 'x')
			return _UNITS_EX;
		return _UNITS_USER;
	}

	static int isCoordinate(const char *s)
	{
		// optional sign
		if (*s == '-' || *s == '+')
			s++;
		// must have at least one digit, or start by a dot
		return (std::isdigit(*s) || *s == '.');
	}

	static Coordinate parseCoordinateRaw(const char *str)
	{
		Coordinate coord = {0, _UNITS_USER};
		char buf[64];
		coord.units = parseUnits(parseNumber(str, buf, 64));
		coord.value = atof(buf);
		return coord;
	}

	static Coordinate coord(float v, int Units)
	{
		Coordinate coord = {v, Units};
		return coord;
	}

	static float parseCoordinate(Parser *p, const char *str, float orig, float length)
	{
		Coordinate coord = parseCoordinateRaw(str);
		return convertToPixels(p, coord, orig, length);
	}

	static int parseTransformArgs(const char *str, float *args, int maxNa, int *na)
	{
		const char *end;
		const char *ptr;
		char it[64];

		*na = 0;
		ptr = str;
		while (*ptr && *ptr != '(')
			++ptr;
		if (*ptr == 0)
			return 1;
		end = ptr;
		while (*end && *end != ')')
			++end;
		if (*end == 0)
			return 1;

		while (ptr < end)
		{
			if (*ptr == '-' || *ptr == '+' || *ptr == '.' || std::isdigit(*ptr))
			{
				if (*na >= maxNa)
					return 0;
				ptr = parseNumber(ptr, it, 64);
				args[(*na)++] = (float)atof(it);
			}
			else
			{
				++ptr;
			}
		}
		return (int)(end - str);
	}

	static int parseMatrix(float *xform, const char *str)
	{
		float t[6];
		int na = 0;
		int len = parseTransformArgs(str, t, 6, &na);
		if (na != 6)
			return len;
		memcpy(xform, t, sizeof(float) * 6);
		return len;
	}

	static int parseTranslate(float *xform, const char *str)
	{
		float args[2];
		float t[6];
		int na = 0;
		int len = parseTransformArgs(str, args, 2, &na);
		if (na == 1)
			args[1] = 0.0;

		xformSetTranslation(t, args[0], args[1]);
		memcpy(xform, t, sizeof(float) * 6);
		return len;
	}

	static int parseScale(float *xform, const char *str)
	{
		float args[2];
		int na = 0;
		float t[6];
		int len = parseTransformArgs(str, args, 2, &na);
		if (na == 1)
			args[1] = args[0];
		xformSetScale(t, args[0], args[1]);
		memcpy(xform, t, sizeof(float) * 6);
		return len;
	}

	static int parseSkewX(float *xform, const char *str)
	{
		float args[1];
		int na = 0;
		float t[6];
		int len = parseTransformArgs(str, args, 1, &na);
		xformSetSkewX(t, args[0] / 180.0f * _PI);
		memcpy(xform, t, sizeof(float) * 6);
		return len;
	}

	static int parseSkewY(float *xform, const char *str)
	{
		float args[1];
		int na = 0;
		float t[6];
		int len = parseTransformArgs(str, args, 1, &na);
		xformSetSkewY(t, args[0] / 180.0f * _PI);
		memcpy(xform, t, sizeof(float) * 6);
		return len;
	}

	static int parseRotate(float *xform, const char *str)
	{
		float args[3];
		int na = 0;
		float m[6];
		float t[6];
		int len = parseTransformArgs(str, args, 3, &na);
		if (na == 1)
			args[1] = args[2] = 0.0f;
		xformIdentity(m);

		if (na > 1)
		{
			xformSetTranslation(t, -args[1], -args[2]);
			xformMultiply(m, t);
		}

		xformSetRotation(t, args[0] / 180.0f * _PI);
		xformMultiply(m, t);

		if (na > 1)
		{
			xformSetTranslation(t, args[1], args[2]);
			xformMultiply(m, t);
		}

		memcpy(xform, m, sizeof(float) * 6);

		return len;
	}

	static void parseTransform(float *xform, const char *str)
	{
		float t[6];
		int len;
		xformIdentity(xform);
		while (*str)
		{
			if (strncmp(str, "matrix", 6) == 0)
				len = parseMatrix(t, str);
			else if (strncmp(str, "translate", 9) == 0)
				len = parseTranslate(t, str);
			else if (strncmp(str, "scale", 5) == 0)
				len = parseScale(t, str);
			else if (strncmp(str, "rotate", 6) == 0)
				len = parseRotate(t, str);
			else if (strncmp(str, "skewX", 5) == 0)
				len = parseSkewX(t, str);
			else if (strncmp(str, "skewY", 5) == 0)
				len = parseSkewY(t, str);
			else
			{
				++str;
				continue;
			}
			if (len != 0)
			{
				str += len;
			}
			else
			{
				++str;
				continue;
			}

			xformPremultiply(xform, t);
		}
	}

	static void parseUrl(char *id, const char *str)
	{
		int i = 0;
		str += 4; // "url(";
		if (*str == '#')
			str++;
		while (i < 63 && *str != ')')
		{
			id[i] = *str++;
			i++;
		}
		id[i] = '\0';
	}

	static char parseLineCap(const char *str)
	{
		if (strcmp(str, "butt") == 0)
			return _CAP_BUTT;
		else if (strcmp(str, "round") == 0)
			return _CAP_ROUND;
		else if (strcmp(str, "square") == 0)
			return _CAP_SQUARE;
		// TODO: handle inherit.
		return _CAP_BUTT;
	}

	static char parseLineJoin(const char *str)
	{
		if (strcmp(str, "miter") == 0)
			return _JOIN_MITER;
		else if (strcmp(str, "round") == 0)
			return _JOIN_ROUND;
		else if (strcmp(str, "bevel") == 0)
			return _JOIN_BEVEL;
		// TODO: handle inherit.
		return _JOIN_MITER;
	}

	static char parseFillRule(const char *str)
	{
		if (strcmp(str, "nonzero") == 0)
			return _FILLRULE_NONZERO;
		else if (strcmp(str, "evenodd") == 0)
			return _FILLRULE_EVENODD;
		// TODO: handle inherit.
		return _FILLRULE_NONZERO;
	}

	static const char *getNextDashItem(const char *s, char *it)
	{
		int n = 0;
		it[0] = '\0';
		// Skip white spaces and commas
		while (*s && (isspace(*s) || *s == ','))
			s++;
		// Advance until whitespace, comma or end.
		while (*s && (!isspace(*s) && *s != ','))
		{
			if (n < 63)
				it[n++] = *s;
			s++;
		}
		it[n++] = '\0';
		return s;
	}

	static int parseStrokeDashArray(Parser *p, const char *str, float *strokeDashArray)
	{
		char item[64];
		int count = 0, i;
		float sum = 0.0f;

		// Handle "none"
		if (str[0] == 'n')
			return 0;

		// Parse dashes
		while (*str)
		{
			str = getNextDashItem(str, item);
			if (!*item)
				break;
			if (count < _MAX_DASHES)
				strokeDashArray[count++] = fabsf(parseCoordinate(p, item, 0.0f, actualLength(p)));
		}

		for (i = 0; i < count; i++)
			sum += strokeDashArray[i];
		if (sum <= 1e-6f)
			count = 0;

		return count;
	}

	static void parseStyle(Parser *p, const char *str);

	static int parseAttr(Parser *p, const char *name, const char *value)
	{
		float xform[6];
		Attrib *attr = getAttr(p);
		if (!attr)
			return 0;

		if (strcmp(name, "style") == 0)
		{
			parseStyle(p, value);
		}
		else if (strcmp(name, "display") == 0)
		{
			if (strcmp(value, "none") == 0)
				attr->visible = 0;
			// Don't reset ->visible on display:inline, one display:none hides the whole subtree
		}
		else if (strcmp(name, "fill") == 0)
		{
			if (strcmp(value, "none") == 0)
			{
				attr->hasFill = 0;
			}
			else if (strncmp(value, "url(", 4) == 0)
			{
				attr->hasFill = 2;
				parseUrl(attr->fillGradient, value);
			}
			else
			{
				attr->hasFill = 1;
				attr->fillColor = parseColor(value);
			}
		}
		else if (strcmp(name, "opacity") == 0)
		{
			attr->opacity = parseOpacity(value);
		}
		else if (strcmp(name, "fill-opacity") == 0)
		{
			attr->fillOpacity = parseOpacity(value);
		}
		else if (strcmp(name, "stroke") == 0)
		{
			if (strcmp(value, "none") == 0)
			{
				attr->hasStroke = 0;
			}
			else if (strncmp(value, "url(", 4) == 0)
			{
				attr->hasStroke = 2;
				parseUrl(attr->strokeGradient, value);
			}
			else
			{
				attr->hasStroke = 1;
				attr->strokeColor = parseColor(value);
			}
		}
		else if (strcmp(name, "stroke-width") == 0)
		{
			attr->strokeWidth = parseCoordinate(p, value, 0.0f, actualLength(p));
		}
		else if (strcmp(name, "stroke-dasharray") == 0)
		{
			attr->strokeDashCount = parseStrokeDashArray(p, value, attr->strokeDashArray);
		}
		else if (strcmp(name, "stroke-dashoffset") == 0)
		{
			attr->strokeDashOffset = parseCoordinate(p, value, 0.0f, actualLength(p));
		}
		else if (strcmp(name, "stroke-opacity") == 0)
		{
			attr->strokeOpacity = parseOpacity(value);
		}
		else if (strcmp(name, "stroke-linecap") == 0)
		{
			attr->strokeLineCap = parseLineCap(value);
		}
		else if (strcmp(name, "stroke-linejoin") == 0)
		{
			attr->strokeLineJoin = parseLineJoin(value);
		}
		else if (strcmp(name, "stroke-miterlimit") == 0)
		{
			attr->miterLimit = parseMiterLimit(value);
		}
		else if (strcmp(name, "fill-rule") == 0)
		{
			attr->fillRule = parseFillRule(value);
		}
		else if (strcmp(name, "font-size") == 0)
		{
			attr->fontSize = parseCoordinate(p, value, 0.0f, actualLength(p));
		}
		else if (strcmp(name, "transform") == 0)
		{
			parseTransform(xform, value);
			xformPremultiply(attr->xform, xform);
		}
		else if (strcmp(name, "stop-color") == 0)
		{
			attr->stopColor = parseColor(value);
		}
		else if (strcmp(name, "stop-opacity") == 0)
		{
			attr->stopOpacity = parseOpacity(value);
		}
		else if (strcmp(name, "offset") == 0)
		{
			attr->stopOffset = parseCoordinate(p, value, 0.0f, 1.0f);
		}
		else if (strcmp(name, "id") == 0)
		{
			strncpy(attr->id, value, 63);
			attr->id[63] = '\0';
		}
		else
		{
			return 0;
		}
		return 1;
	}

	static int parseNameValue(Parser *p, const char *start, const char *end)
	{
		const char *str;
		const char *val;
		char name[512];
		char value[512];
		int n;

		str = start;
		while (str < end && *str != ':')
			++str;

		val = str;

		// Right Trim
		while (str > start && (*str == ':' || isspace(*str)))
			--str;
		++str;

		n = (int)(str - start);
		if (n > 511)
			n = 511;
		if (n)
			memcpy(name, start, n);
		name[n] = 0;

		while (val < end && (*val == ':' || isspace(*val)))
			++val;

		n = (int)(end - val);
		if (n > 511)
			n = 511;
		if (n)
			memcpy(value, val, n);
		value[n] = 0;

		return parseAttr(p, name, value);
	}

	static void parseStyle(Parser *p, const char *str)
	{
		const char *start;
		const char *end;

		while (*str)
		{
			// Left Trim
			while (*str && isspace(*str))
				++str;
			start = str;
			while (*str && *str != ';')
				++str;
			end = str;

			// Right Trim
			while (end > start && (*end == ';' || isspace(*end)))
				--end;
			++end;

			parseNameValue(p, start, end);
			if (*str)
				++str;
		}
	}

	static void parseAttribs(Parser *p, const char **attr)
	{
		int i;
		for (i = 0; attr[i]; i += 2)
		{
			if (strcmp(attr[i], "style") == 0)
				parseStyle(p, attr[i + 1]);
			else
				parseAttr(p, attr[i], attr[i + 1]);
		}
	}

	static int getArgsPerElement(char cmd)
	{
		switch (cmd)
		{
		case 'v':
		case 'V':
		case 'h':
		case 'H':
			return 1;
		case 'm':
		case 'M':
		case 'l':
		case 'L':
		case 't':
		case 'T':
			return 2;
		case 'q':
		case 'Q':
		case 's':
		case 'S':
			return 4;
		case 'c':
		case 'C':
			return 6;
		case 'a':
		case 'A':
			return 7;
		case 'z':
		case 'Z':
			return 0;
		}
		return -1;
	}

	static void PathMoveTo(Parser *p, float *cpx, float *cpy, float *args, int rel)
	{
		if (rel)
		{
			*cpx += args[0];
			*cpy += args[1];
		}
		else
		{
			*cpx = args[0];
			*cpy = args[1];
		}
		moveTo(p, *cpx, *cpy);
	}

	static void pathLineTo(Parser *p, float *cpx, float *cpy, float *args, int rel)
	{
		if (rel)
		{
			*cpx += args[0];
			*cpy += args[1];
		}
		else
		{
			*cpx = args[0];
			*cpy = args[1];
		}
		lineTo(p, *cpx, *cpy);
	}

	static void pathHLineTo(Parser *p, float *cpx, float *cpy, float *args, int rel)
	{
		if (rel)
			*cpx += args[0];
		else
			*cpx = args[0];
		lineTo(p, *cpx, *cpy);
	}

	static void pathVLineTo(Parser *p, float *cpx, float *cpy, float *args, int rel)
	{
		if (rel)
			*cpy += args[0];
		else
			*cpy = args[0];
		lineTo(p, *cpx, *cpy);
	}

	static void pathCubicBezTo(Parser *p, float *cpx, float *cpy,
														 float *cpx2, float *cpy2, float *args, int rel)
	{
		float x2, y2, cx1, cy1, cx2, cy2;

		if (rel)
		{
			cx1 = *cpx + args[0];
			cy1 = *cpy + args[1];
			cx2 = *cpx + args[2];
			cy2 = *cpy + args[3];
			x2 = *cpx + args[4];
			y2 = *cpy + args[5];
		}
		else
		{
			cx1 = args[0];
			cy1 = args[1];
			cx2 = args[2];
			cy2 = args[3];
			x2 = args[4];
			y2 = args[5];
		}

		cubicBezTo(p, cx1, cy1, cx2, cy2, x2, y2);

		*cpx2 = cx2;
		*cpy2 = cy2;
		*cpx = x2;
		*cpy = y2;
	}

	static void pathCubicBezShortTo(Parser *p, float *cpx, float *cpy,
																	float *cpx2, float *cpy2, float *args, int rel)
	{
		float x1, y1, x2, y2, cx1, cy1, cx2, cy2;

		x1 = *cpx;
		y1 = *cpy;
		if (rel)
		{
			cx2 = *cpx + args[0];
			cy2 = *cpy + args[1];
			x2 = *cpx + args[2];
			y2 = *cpy + args[3];
		}
		else
		{
			cx2 = args[0];
			cy2 = args[1];
			x2 = args[2];
			y2 = args[3];
		}

		cx1 = 2 * x1 - *cpx2;
		cy1 = 2 * y1 - *cpy2;

		cubicBezTo(p, cx1, cy1, cx2, cy2, x2, y2);

		*cpx2 = cx2;
		*cpy2 = cy2;
		*cpx = x2;
		*cpy = y2;
	}

	static void pathQuadBezTo(Parser *p, float *cpx, float *cpy,
														float *cpx2, float *cpy2, float *args, int rel)
	{
		float x1, y1, x2, y2, cx, cy;
		float cx1, cy1, cx2, cy2;

		x1 = *cpx;
		y1 = *cpy;
		if (rel)
		{
			cx = *cpx + args[0];
			cy = *cpy + args[1];
			x2 = *cpx + args[2];
			y2 = *cpy + args[3];
		}
		else
		{
			cx = args[0];
			cy = args[1];
			x2 = args[2];
			y2 = args[3];
		}

		// Convert to cubic bezier
		cx1 = x1 + 2.0f / 3.0f * (cx - x1);
		cy1 = y1 + 2.0f / 3.0f * (cy - y1);
		cx2 = x2 + 2.0f / 3.0f * (cx - x2);
		cy2 = y2 + 2.0f / 3.0f * (cy - y2);

		cubicBezTo(p, cx1, cy1, cx2, cy2, x2, y2);

		*cpx2 = cx;
		*cpy2 = cy;
		*cpx = x2;
		*cpy = y2;
	}

	static void pathQuadBezShortTo(Parser *p, float *cpx, float *cpy,
																 float *cpx2, float *cpy2, float *args, int rel)
	{
		float x1, y1, x2, y2, cx, cy;
		float cx1, cy1, cx2, cy2;

		x1 = *cpx;
		y1 = *cpy;
		if (rel)
		{
			x2 = *cpx + args[0];
			y2 = *cpy + args[1];
		}
		else
		{
			x2 = args[0];
			y2 = args[1];
		}

		cx = 2 * x1 - *cpx2;
		cy = 2 * y1 - *cpy2;

		// Convert to cubix bezier
		cx1 = x1 + 2.0f / 3.0f * (cx - x1);
		cy1 = y1 + 2.0f / 3.0f * (cy - y1);
		cx2 = x2 + 2.0f / 3.0f * (cx - x2);
		cy2 = y2 + 2.0f / 3.0f * (cy - y2);

		cubicBezTo(p, cx1, cy1, cx2, cy2, x2, y2);

		*cpx2 = cx;
		*cpy2 = cy;
		*cpx = x2;
		*cpy = y2;
	}

	static float sqr(float x) { return x * x; }
	static float vmag(float x, float y) { return sqrtf(x * x + y * y); }

	static float vecrat(float ux, float uy, float vx, float vy)
	{
		return (ux * vx + uy * vy) / (vmag(ux, uy) * vmag(vx, vy));
	}

	static float vecang(float ux, float uy, float vx, float vy)
	{
		float r = vecrat(ux, uy, vx, vy);
		if (r < -1.0f)
			r = -1.0f;
		if (r > 1.0f)
			r = 1.0f;
		return ((ux * vy < uy * vx) ? -1.0f : 1.0f) * acosf(r);
	}

	static void pathArcTo(Parser *p, float *cpx, float *cpy, float *args, int rel)
	{
		// Ported from canvg (https://code.google.com/p/canvg/)
		float rx, ry, rotx;
		float x1, y1, x2, y2, cx, cy, dx, dy, d;
		float x1p, y1p, cxp, cyp, s, sa, sb;
		float ux, uy, vx, vy, a1, da;
		float x, y, tanx, tany, a, px = 0, py = 0, ptanx = 0, ptany = 0, t[6];
		float sinrx, cosrx;
		int fa, fs;
		int i, ndivs;
		float hda, kappa;

		rx = fabsf(args[0]);								// y radius
		ry = fabsf(args[1]);								// x radius
		rotx = args[2] / 180.0f * _PI;			// x rotation angle
		fa = fabsf(args[3]) > 1e-6 ? 1 : 0; // Large arc
		fs = fabsf(args[4]) > 1e-6 ? 1 : 0; // Sweep direction
		x1 = *cpx;													// start point
		y1 = *cpy;
		if (rel)
		{ // end point
			x2 = *cpx + args[5];
			y2 = *cpy + args[6];
		}
		else
		{
			x2 = args[5];
			y2 = args[6];
		}

		dx = x1 - x2;
		dy = y1 - y2;
		d = sqrtf(dx * dx + dy * dy);
		if (d < 1e-6f || rx < 1e-6f || ry < 1e-6f)
		{
			// The arc degenerates to a line
			lineTo(p, x2, y2);
			*cpx = x2;
			*cpy = y2;
			return;
		}

		sinrx = sinf(rotx);
		cosrx = cosf(rotx);

		// Convert to center point parameterization.
		// http://www.w3.org/TR/SVG11/implnote.html#ArcImplementationNotes
		// 1) Compute x1', y1'
		x1p = cosrx * dx / 2.0f + sinrx * dy / 2.0f;
		y1p = -sinrx * dx / 2.0f + cosrx * dy / 2.0f;
		d = sqr(x1p) / sqr(rx) + sqr(y1p) / sqr(ry);
		if (d > 1)
		{
			d = sqrtf(d);
			rx *= d;
			ry *= d;
		}
		// 2) Compute cx', cy'
		s = 0.0f;
		sa = sqr(rx) * sqr(ry) - sqr(rx) * sqr(y1p) - sqr(ry) * sqr(x1p);
		sb = sqr(rx) * sqr(y1p) + sqr(ry) * sqr(x1p);
		if (sa < 0.0f)
			sa = 0.0f;
		if (sb > 0.0f)
			s = sqrtf(sa / sb);
		if (fa == fs)
			s = -s;
		cxp = s * rx * y1p / ry;
		cyp = s * -ry * x1p / rx;

		// 3) Compute cx,cy from cx',cy'
		cx = (x1 + x2) / 2.0f + cosrx * cxp - sinrx * cyp;
		cy = (y1 + y2) / 2.0f + sinrx * cxp + cosrx * cyp;

		// 4) Calculate theta1, and delta theta.
		ux = (x1p - cxp) / rx;
		uy = (y1p - cyp) / ry;
		vx = (-x1p - cxp) / rx;
		vy = (-y1p - cyp) / ry;
		a1 = vecang(1.0f, 0.0f, ux, uy); // Initial angle
		da = vecang(ux, uy, vx, vy);		 // Delta angle

		//	if (vecrat(ux,uy,vx,vy) <= -1.0f) da = _PI;
		//	if (vecrat(ux,uy,vx,vy) >= 1.0f) da = 0;

		if (fs == 0 && da > 0)
			da -= 2 * _PI;
		else if (fs == 1 && da < 0)
			da += 2 * _PI;

		// Approximate the arc using cubic spline segments.
		t[0] = cosrx;
		t[1] = sinrx;
		t[2] = -sinrx;
		t[3] = cosrx;
		t[4] = cx;
		t[5] = cy;

		// Split arc into max 90 degree segments.
		// The loop assumes an iteration per end point (including start and end), this +1.
		ndivs = (int)(fabsf(da) / (_PI * 0.5f) + 1.0f);
		hda = (da / (float)ndivs) / 2.0f;
		kappa = fabsf(4.0f / 3.0f * (1.0f - cosf(hda)) / sinf(hda));
		if (da < 0.0f)
			kappa = -kappa;

		for (i = 0; i <= ndivs; i++)
		{
			a = a1 + da * ((float)i / (float)ndivs);
			dx = cosf(a);
			dy = sinf(a);
			xformPoint(&x, &y, dx * rx, dy * ry, t);											// position
			xformVec(&tanx, &tany, -dy * rx * kappa, dx * ry * kappa, t); // tangent
			if (i > 0)
				cubicBezTo(p, px + ptanx, py + ptany, x - tanx, y - tany, x, y);
			px = x;
			py = y;
			ptanx = tanx;
			ptany = tany;
		}

		*cpx = x2;
		*cpy = y2;
	}

	static void parsePath(Parser *p, const char **attr)
	{
		const char *s = nullptr;
		char cmd = '\0';
		float args[10];
		int nargs;
		int rargs = 0;
		char initPoint;
		float cpx, cpy, cpx2, cpy2;
		const char *tmp[4];
		char closedFlag;
		int i;
		char item[64];

		for (i = 0; attr[i]; i += 2)
		{
			if (strcmp(attr[i], "d") == 0)
			{
				s = attr[i + 1];
			}
			else
			{
				tmp[0] = attr[i];
				tmp[1] = attr[i + 1];
				tmp[2] = 0;
				tmp[3] = 0;
				parseAttribs(p, tmp);
			}
		}

		if (s)
		{
			resetPath(p);
			cpx = 0;
			cpy = 0;
			cpx2 = 0;
			cpy2 = 0;
			initPoint = 0;
			closedFlag = 0;
			nargs = 0;

			while (*s)
			{
				s = getNextPathItem(s, item);
				if (!*item)
					break;
				if (cmd != '\0' && isCoordinate(item))
				{
					if (nargs < 10)
						args[nargs++] = (float)atof(item);
					if (nargs >= rargs)
					{
						switch (cmd)
						{
						case 'm':
						case 'M':
							PathMoveTo(p, &cpx, &cpy, args, cmd == 'm' ? 1 : 0);
							// Moveto can be followed by multiple Coordinate pairs,
							// which should be treated as linetos.
							cmd = (cmd == 'm') ? 'l' : 'L';
							rargs = getArgsPerElement(cmd);
							cpx2 = cpx;
							cpy2 = cpy;
							initPoint = 1;
							break;
						case 'l':
						case 'L':
							pathLineTo(p, &cpx, &cpy, args, cmd == 'l' ? 1 : 0);
							cpx2 = cpx;
							cpy2 = cpy;
							break;
						case 'H':
						case 'h':
							pathHLineTo(p, &cpx, &cpy, args, cmd == 'h' ? 1 : 0);
							cpx2 = cpx;
							cpy2 = cpy;
							break;
						case 'V':
						case 'v':
							pathVLineTo(p, &cpx, &cpy, args, cmd == 'v' ? 1 : 0);
							cpx2 = cpx;
							cpy2 = cpy;
							break;
						case 'C':
						case 'c':
							pathCubicBezTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 'c' ? 1 : 0);
							break;
						case 'S':
						case 's':
							pathCubicBezShortTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 's' ? 1 : 0);
							break;
						case 'Q':
						case 'q':
							pathQuadBezTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 'q' ? 1 : 0);
							break;
						case 'T':
						case 't':
							pathQuadBezShortTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 't' ? 1 : 0);
							break;
						case 'A':
						case 'a':
							pathArcTo(p, &cpx, &cpy, args, cmd == 'a' ? 1 : 0);
							cpx2 = cpx;
							cpy2 = cpy;
							break;
						default:
							if (nargs >= 2)
							{
								cpx = args[nargs - 2];
								cpy = args[nargs - 1];
								cpx2 = cpx;
								cpy2 = cpy;
							}
							break;
						}
						nargs = 0;
					}
				}
				else
				{
					cmd = item[0];
					if (cmd == 'M' || cmd == 'm')
					{
						// Commit Path.
						if (p->npts > 0)
							addPath(p, closedFlag);
						// Start new subPath.
						resetPath(p);
						closedFlag = 0;
						nargs = 0;
					}
					else if (initPoint == 0)
					{
						// Do not allow other commands until initial point has been set (moveTo called once).
						cmd = '\0';
					}
					if (cmd == 'Z' || cmd == 'z')
					{
						closedFlag = 1;
						// Commit Path.
						if (p->npts > 0)
						{
							// Move current point to first point
							cpx = p->pts[0];
							cpy = p->pts[1];
							cpx2 = cpx;
							cpy2 = cpy;
							addPath(p, closedFlag);
						}
						// Start new subPath.
						resetPath(p);
						moveTo(p, cpx, cpy);
						closedFlag = 0;
						nargs = 0;
					}
					rargs = getArgsPerElement(cmd);
					if (rargs == -1)
					{
						// Command not recognized
						cmd = '\0';
						rargs = 0;
					}
				}
			}
			// Commit Path.
			if (p->npts)
				addPath(p, closedFlag);
		}

		addShape(p);
	}

	static void parseRect(Parser *p, const char **attr)
	{
		float x = 0.0f;
		float y = 0.0f;
		float w = 0.0f;
		float h = 0.0f;
		float rx = -1.0f; // marks not set
		float ry = -1.0f;
		int i;

		for (i = 0; attr[i]; i += 2)
		{
			if (!parseAttr(p, attr[i], attr[i + 1]))
			{
				if (strcmp(attr[i], "x") == 0)
					x = parseCoordinate(p, attr[i + 1], actualOrigX(p), actualWidth(p));
				if (strcmp(attr[i], "y") == 0)
					y = parseCoordinate(p, attr[i + 1], actualOrigY(p), actualHeight(p));
				if (strcmp(attr[i], "width") == 0)
					w = parseCoordinate(p, attr[i + 1], 0.0f, actualWidth(p));
				if (strcmp(attr[i], "height") == 0)
					h = parseCoordinate(p, attr[i + 1], 0.0f, actualHeight(p));
				if (strcmp(attr[i], "rx") == 0)
					rx = fabsf(parseCoordinate(p, attr[i + 1], 0.0f, actualWidth(p)));
				if (strcmp(attr[i], "ry") == 0)
					ry = fabsf(parseCoordinate(p, attr[i + 1], 0.0f, actualHeight(p)));
			}
		}

		if (rx < 0.0f && ry > 0.0f)
			rx = ry;
		if (ry < 0.0f && rx > 0.0f)
			ry = rx;
		if (rx < 0.0f)
			rx = 0.0f;
		if (ry < 0.0f)
			ry = 0.0f;
		if (rx > w / 2.0f)
			rx = w / 2.0f;
		if (ry > h / 2.0f)
			ry = h / 2.0f;

		if (w != 0.0f && h != 0.0f)
		{
			resetPath(p);

			if (rx < 0.00001f || ry < 0.0001f)
			{
				moveTo(p, x, y);
				lineTo(p, x + w, y);
				lineTo(p, x + w, y + h);
				lineTo(p, x, y + h);
			}
			else
			{
				// Rounded rectangle
				moveTo(p, x + rx, y);
				lineTo(p, x + w - rx, y);
				cubicBezTo(p, x + w - rx * (1 - _KAPPA90), y, x + w, y + ry * (1 - _KAPPA90), x + w, y + ry);
				lineTo(p, x + w, y + h - ry);
				cubicBezTo(p, x + w, y + h - ry * (1 - _KAPPA90), x + w - rx * (1 - _KAPPA90), y + h, x + w - rx, y + h);
				lineTo(p, x + rx, y + h);
				cubicBezTo(p, x + rx * (1 - _KAPPA90), y + h, x, y + h - ry * (1 - _KAPPA90), x, y + h - ry);
				lineTo(p, x, y + ry);
				cubicBezTo(p, x, y + ry * (1 - _KAPPA90), x + rx * (1 - _KAPPA90), y, x + rx, y);
			}

			addPath(p, 1);

			addShape(p);
		}
	}

	static void parseCircle(Parser *p, const char **attr)
	{
		float cx = 0.0f;
		float cy = 0.0f;
		float r = 0.0f;
		int i;

		for (i = 0; attr[i]; i += 2)
		{
			if (!parseAttr(p, attr[i], attr[i + 1]))
			{
				if (strcmp(attr[i], "cx") == 0)
					cx = parseCoordinate(p, attr[i + 1], actualOrigX(p), actualWidth(p));
				if (strcmp(attr[i], "cy") == 0)
					cy = parseCoordinate(p, attr[i + 1], actualOrigY(p), actualHeight(p));
				if (strcmp(attr[i], "r") == 0)
					r = fabsf(parseCoordinate(p, attr[i + 1], 0.0f, actualLength(p)));
			}
		}

		if (r > 0.0f)
		{
			resetPath(p);

			moveTo(p, cx + r, cy);
			cubicBezTo(p, cx + r, cy + r * _KAPPA90, cx + r * _KAPPA90, cy + r, cx, cy + r);
			cubicBezTo(p, cx - r * _KAPPA90, cy + r, cx - r, cy + r * _KAPPA90, cx - r, cy);
			cubicBezTo(p, cx - r, cy - r * _KAPPA90, cx - r * _KAPPA90, cy - r, cx, cy - r);
			cubicBezTo(p, cx + r * _KAPPA90, cy - r, cx + r, cy - r * _KAPPA90, cx + r, cy);

			addPath(p, 1);

			addShape(p);
		}
	}

	static void parseEllipse(Parser *p, const char **attr)
	{
		float cx = 0.0f;
		float cy = 0.0f;
		float rx = 0.0f;
		float ry = 0.0f;
		int i;

		for (i = 0; attr[i]; i += 2)
		{
			if (!parseAttr(p, attr[i], attr[i + 1]))
			{
				if (strcmp(attr[i], "cx") == 0)
					cx = parseCoordinate(p, attr[i + 1], actualOrigX(p), actualWidth(p));
				if (strcmp(attr[i], "cy") == 0)
					cy = parseCoordinate(p, attr[i + 1], actualOrigY(p), actualHeight(p));
				if (strcmp(attr[i], "rx") == 0)
					rx = fabsf(parseCoordinate(p, attr[i + 1], 0.0f, actualWidth(p)));
				if (strcmp(attr[i], "ry") == 0)
					ry = fabsf(parseCoordinate(p, attr[i + 1], 0.0f, actualHeight(p)));
			}
		}

		if (rx > 0.0f && ry > 0.0f)
		{

			resetPath(p);

			moveTo(p, cx + rx, cy);
			cubicBezTo(p, cx + rx, cy + ry * _KAPPA90, cx + rx * _KAPPA90, cy + ry, cx, cy + ry);
			cubicBezTo(p, cx - rx * _KAPPA90, cy + ry, cx - rx, cy + ry * _KAPPA90, cx - rx, cy);
			cubicBezTo(p, cx - rx, cy - ry * _KAPPA90, cx - rx * _KAPPA90, cy - ry, cx, cy - ry);
			cubicBezTo(p, cx + rx * _KAPPA90, cy - ry, cx + rx, cy - ry * _KAPPA90, cx + rx, cy);

			addPath(p, 1);

			addShape(p);
		}
	}

	static void parseLine(Parser *p, const char **attr)
	{
		float x1 = 0.0;
		float y1 = 0.0;
		float x2 = 0.0;
		float y2 = 0.0;
		int i;

		for (i = 0; attr[i]; i += 2)
		{
			if (!parseAttr(p, attr[i], attr[i + 1]))
			{
				if (strcmp(attr[i], "x1") == 0)
					x1 = parseCoordinate(p, attr[i + 1], actualOrigX(p), actualWidth(p));
				if (strcmp(attr[i], "y1") == 0)
					y1 = parseCoordinate(p, attr[i + 1], actualOrigY(p), actualHeight(p));
				if (strcmp(attr[i], "x2") == 0)
					x2 = parseCoordinate(p, attr[i + 1], actualOrigX(p), actualWidth(p));
				if (strcmp(attr[i], "y2") == 0)
					y2 = parseCoordinate(p, attr[i + 1], actualOrigY(p), actualHeight(p));
			}
		}

		resetPath(p);

		moveTo(p, x1, y1);
		lineTo(p, x2, y2);

		addPath(p, 0);

		addShape(p);
	}

	static void parsePoly(Parser *p, const char **attr, int closeFlag)
	{
		int i;
		const char *s;
		float args[2];
		int nargs, npts = 0;
		char item[64];

		resetPath(p);

		for (i = 0; attr[i]; i += 2)
		{
			if (!parseAttr(p, attr[i], attr[i + 1]))
			{
				if (strcmp(attr[i], "points") == 0)
				{
					s = attr[i + 1];
					nargs = 0;
					while (*s)
					{
						s = getNextPathItem(s, item);
						args[nargs++] = (float)atof(item);
						if (nargs >= 2)
						{
							if (npts == 0)
								moveTo(p, args[0], args[1]);
							else
								lineTo(p, args[0], args[1]);
							nargs = 0;
							npts++;
						}
					}
				}
			}
		}

		addPath(p, (char)closeFlag);

		addShape(p);
	}

	static void parseSVG(Parser *p, const char **attr)
	{
		int i;
		for (i = 0; attr[i]; i += 2)
		{
			if (!parseAttr(p, attr[i], attr[i + 1]))
			{
				if (strcmp(attr[i], "width") == 0)
				{
					p->image->width = parseCoordinate(p, attr[i + 1], 0.0f, 0.0f);
				}
				else if (strcmp(attr[i], "height") == 0)
				{
					p->image->height = parseCoordinate(p, attr[i + 1], 0.0f, 0.0f);
				}
				else if (strcmp(attr[i], "viewBox") == 0)
				{
					const char *s = attr[i + 1];
					char buf[64];
					s = parseNumber(s, buf, 64);
					p->viewMinx = atof(buf);
					while (*s && (isspace(*s) || *s == '%' || *s == ','))
						s++;
					if (!*s)
						return;
					s = parseNumber(s, buf, 64);
					p->viewMiny = atof(buf);
					while (*s && (isspace(*s) || *s == '%' || *s == ','))
						s++;
					if (!*s)
						return;
					s = parseNumber(s, buf, 64);
					p->viewWidth = atof(buf);
					while (*s && (isspace(*s) || *s == '%' || *s == ','))
						s++;
					if (!*s)
						return;
					s = parseNumber(s, buf, 64);
					p->viewHeight = atof(buf);
				}
				else if (strcmp(attr[i], "preserveAspectRatio") == 0)
				{
					if (strstr(attr[i + 1], "none") != 0)
					{
						// No uniform scaling
						p->alignType = _ALIGN_NONE;
					}
					else
					{
						// Parse X align
						if (strstr(attr[i + 1], "xMin") != 0)
							p->alignX = _ALIGN_MIN;
						else if (strstr(attr[i + 1], "xMid") != 0)
							p->alignX = _ALIGN_MID;
						else if (strstr(attr[i + 1], "xMax") != 0)
							p->alignX = _ALIGN_MAX;
						// Parse X align
						if (strstr(attr[i + 1], "yMin") != 0)
							p->alignY = _ALIGN_MIN;
						else if (strstr(attr[i + 1], "yMid") != 0)
							p->alignY = _ALIGN_MID;
						else if (strstr(attr[i + 1], "yMax") != 0)
							p->alignY = _ALIGN_MAX;
						// Parse meet/slice
						p->alignType = _ALIGN_MEET;
						if (strstr(attr[i + 1], "slice") != 0)
							p->alignType = _ALIGN_SLICE;
					}
				}
			}
		}
	}

	static void parseGradient(Parser *p, const char **attr, char type)
	{
		int i;
		GradientData *grad = (GradientData *)malloc(sizeof(GradientData));
		if (grad == nullptr)
			return;
		memset(grad, 0, sizeof(GradientData));
		grad->units = _OBJECT_SPACE;
		grad->type = type;
		if (grad->type == _PAINT_LINEAR_GRADIENT)
		{
			grad->linear.x1 = coord(0.0f, _UNITS_PERCENT);
			grad->linear.y1 = coord(0.0f, _UNITS_PERCENT);
			grad->linear.x2 = coord(100.0f, _UNITS_PERCENT);
			grad->linear.y2 = coord(0.0f, _UNITS_PERCENT);
		}
		else if (grad->type == _PAINT_RADIAL_GRADIENT)
		{
			grad->radial.cx = coord(50.0f, _UNITS_PERCENT);
			grad->radial.cy = coord(50.0f, _UNITS_PERCENT);
			grad->radial.r = coord(50.0f, _UNITS_PERCENT);
		}

		xformIdentity(grad->xform);

		for (i = 0; attr[i]; i += 2)
		{
			if (strcmp(attr[i], "id") == 0)
			{
				strncpy(grad->id, attr[i + 1], 63);
				grad->id[63] = '\0';
			}
			else if (!parseAttr(p, attr[i], attr[i + 1]))
			{
				if (strcmp(attr[i], "GradientUnits") == 0)
				{
					if (strcmp(attr[i + 1], "objectBoundingBox") == 0)
						grad->units = _OBJECT_SPACE;
					else
						grad->units = _USER_SPACE;
				}
				else if (strcmp(attr[i], "gradientTransform") == 0)
				{
					parseTransform(grad->xform, attr[i + 1]);
				}
				else if (strcmp(attr[i], "cx") == 0)
				{
					grad->radial.cx = parseCoordinateRaw(attr[i + 1]);
				}
				else if (strcmp(attr[i], "cy") == 0)
				{
					grad->radial.cy = parseCoordinateRaw(attr[i + 1]);
				}
				else if (strcmp(attr[i], "r") == 0)
				{
					grad->radial.r = parseCoordinateRaw(attr[i + 1]);
				}
				else if (strcmp(attr[i], "fx") == 0)
				{
					grad->radial.fx = parseCoordinateRaw(attr[i + 1]);
				}
				else if (strcmp(attr[i], "fy") == 0)
				{
					grad->radial.fy = parseCoordinateRaw(attr[i + 1]);
				}
				else if (strcmp(attr[i], "x1") == 0)
				{
					grad->linear.x1 = parseCoordinateRaw(attr[i + 1]);
				}
				else if (strcmp(attr[i], "y1") == 0)
				{
					grad->linear.y1 = parseCoordinateRaw(attr[i + 1]);
				}
				else if (strcmp(attr[i], "x2") == 0)
				{
					grad->linear.x2 = parseCoordinateRaw(attr[i + 1]);
				}
				else if (strcmp(attr[i], "y2") == 0)
				{
					grad->linear.y2 = parseCoordinateRaw(attr[i + 1]);
				}
				else if (strcmp(attr[i], "spreadMethod") == 0)
				{
					if (strcmp(attr[i + 1], "pad") == 0)
						grad->spread = _SPREAD_PAD;
					else if (strcmp(attr[i + 1], "reflect") == 0)
						grad->spread = _SPREAD_REFLECT;
					else if (strcmp(attr[i + 1], "repeat") == 0)
						grad->spread = _SPREAD_REPEAT;
				}
				else if (strcmp(attr[i], "xlink:href") == 0)
				{
					const char *href = attr[i + 1];
					strncpy(grad->ref, href + 1, 62);
					grad->ref[62] = '\0';
				}
			}
		}

		grad->next = p->gradients;
		p->gradients = grad;
	}

	static void parseGradientStop(Parser *p, const char **attr)
	{
		Attrib *curAttr = getAttr(p);
		GradientData *grad;
		GradientStop *stop;
		int i, idx;

		curAttr->stopOffset = 0;
		curAttr->stopColor = 0;
		curAttr->stopOpacity = 1.0f;

		for (i = 0; attr[i]; i += 2)
		{
			parseAttr(p, attr[i], attr[i + 1]);
		}

		// Add stop to the last gradient.
		grad = p->gradients;
		if (grad == nullptr)
			return;

		grad->nstops++;
		grad->stops = (GradientStop *)realloc(grad->stops, sizeof(GradientStop) * grad->nstops);
		if (grad->stops == nullptr)
			return;

		// Insert
		idx = grad->nstops - 1;
		for (i = 0; i < grad->nstops - 1; i++)
		{
			if (curAttr->stopOffset < grad->stops[i].offset)
			{
				idx = i;
				break;
			}
		}
		if (idx != grad->nstops - 1)
		{
			for (i = grad->nstops - 1; i > idx; i--)
				grad->stops[i] = grad->stops[i - 1];
		}

		stop = &grad->stops[idx];
		stop->color = curAttr->stopColor;
		stop->color |= (unsigned int)(curAttr->stopOpacity * 255) << 24;
		stop->offset = curAttr->stopOffset;
	}

	static void startElement(void *ud, const char *el, const char **attr)
	{
		Parser *p = (Parser *)ud;

		if (p->defsFlag)
		{
			// Skip everything but gradients in defs
			if (strcmp(el, "linearGradient") == 0)
			{
				parseGradient(p, attr, _PAINT_LINEAR_GRADIENT);
			}
			else if (strcmp(el, "radialGradient") == 0)
			{
				parseGradient(p, attr, _PAINT_RADIAL_GRADIENT);
			}
			else if (strcmp(el, "stop") == 0)
			{
				parseGradientStop(p, attr);
			}
			return;
		}

		if (strcmp(el, "g") == 0)
		{
			pushAttr(p);
			parseAttribs(p, attr);
		}
		else if (strcmp(el, "Path") == 0)
		{
			if (p->PathFlag) // Do not allow nested Paths.
				return;
			pushAttr(p);
			parsePath(p, attr);
			popAttr(p);
		}
		else if (strcmp(el, "rect") == 0)
		{
			pushAttr(p);
			parseRect(p, attr);
			popAttr(p);
		}
		else if (strcmp(el, "circle") == 0)
		{
			pushAttr(p);
			parseCircle(p, attr);
			popAttr(p);
		}
		else if (strcmp(el, "ellipse") == 0)
		{
			pushAttr(p);
			parseEllipse(p, attr);
			popAttr(p);
		}
		else if (strcmp(el, "line") == 0)
		{
			pushAttr(p);
			parseLine(p, attr);
			popAttr(p);
		}
		else if (strcmp(el, "polyline") == 0)
		{
			pushAttr(p);
			parsePoly(p, attr, 0);
			popAttr(p);
		}
		else if (strcmp(el, "polygon") == 0)
		{
			pushAttr(p);
			parsePoly(p, attr, 1);
			popAttr(p);
		}
		else if (strcmp(el, "linearGradient") == 0)
		{
			parseGradient(p, attr, _PAINT_LINEAR_GRADIENT);
		}
		else if (strcmp(el, "radialGradient") == 0)
		{
			parseGradient(p, attr, _PAINT_RADIAL_GRADIENT);
		}
		else if (strcmp(el, "stop") == 0)
		{
			parseGradientStop(p, attr);
		}
		else if (strcmp(el, "defs") == 0)
		{
			p->defsFlag = 1;
		}
		else if (strcmp(el, "svg") == 0)
		{
			parseSVG(p, attr);
		}
	}

	static void endElement(void *ud, const char *el)
	{
		Parser *p = (Parser *)ud;

		if (strcmp(el, "g") == 0)
		{
			popAttr(p);
		}
		else if (strcmp(el, "Path") == 0)
		{
			p->PathFlag = 0;
		}
		else if (strcmp(el, "defs") == 0)
		{
			p->defsFlag = 0;
		}
	}

	static void content(void *ud, const char *s)
	{
		_NOTUSED(ud);
		_NOTUSED(s);
	}

	static void imageBounds(Parser *p, float *bounds)
	{
		Shape *shape;
		shape = p->image->shapes;
		if (shape == nullptr)
		{
			bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0.0;
			return;
		}
		bounds[0] = shape->bounds[0];
		bounds[1] = shape->bounds[1];
		bounds[2] = shape->bounds[2];
		bounds[3] = shape->bounds[3];
		for (shape = shape->next; shape != nullptr; shape = shape->next)
		{
			bounds[0] = minf(bounds[0], shape->bounds[0]);
			bounds[1] = minf(bounds[1], shape->bounds[1]);
			bounds[2] = maxf(bounds[2], shape->bounds[2]);
			bounds[3] = maxf(bounds[3], shape->bounds[3]);
		}
	}

	static float viewAlign(float content, float container, int type)
	{
		if (type == _ALIGN_MIN)
			return 0;
		else if (type == _ALIGN_MAX)
			return container - content;
		// mid
		return (container - content) * 0.5f;
	}

	static void scaleGradient(Gradient *grad, float tx, float ty, float sx, float sy)
	{
		float t[6];
		xformSetTranslation(t, tx, ty);
		xformMultiply(grad->xform, t);

		xformSetScale(t, sx, sy);
		xformMultiply(grad->xform, t);
	}

	static void scaleToViewbox(Parser *p, const char *Units)
	{
		Shape *shape;
		Path *path;
		float tx, ty, sx, sy, us, bounds[4], t[6], avgs;
		int i;
		float *pt;

		// Guess image size if not set completely.
		imageBounds(p, bounds);

		if (p->viewWidth == 0)
		{
			if (p->image->width > 0)
			{
				p->viewWidth = p->image->width;
			}
			else
			{
				p->viewMinx = bounds[0];
				p->viewWidth = bounds[2] - bounds[0];
			}
		}
		if (p->viewHeight == 0)
		{
			if (p->image->height > 0)
			{
				p->viewHeight = p->image->height;
			}
			else
			{
				p->viewMiny = bounds[1];
				p->viewHeight = bounds[3] - bounds[1];
			}
		}
		if (p->image->width == 0)
			p->image->width = p->viewWidth;
		if (p->image->height == 0)
			p->image->height = p->viewHeight;

		tx = -p->viewMinx;
		ty = -p->viewMiny;
		sx = p->viewWidth > 0 ? p->image->width / p->viewWidth : 0;
		sy = p->viewHeight > 0 ? p->image->height / p->viewHeight : 0;
		// Unit scaling
		us = 1.0f / convertToPixels(p, coord(1.0f, parseUnits(Units)), 0.0f, 1.0f);

		// Fix aspect ratio
		if (p->alignType == _ALIGN_MEET)
		{
			// fit whole image into viewbox
			sx = sy = minf(sx, sy);
			tx += viewAlign(p->viewWidth * sx, p->image->width, p->alignX) / sx;
			ty += viewAlign(p->viewHeight * sy, p->image->height, p->alignY) / sy;
		}
		else if (p->alignType == _ALIGN_SLICE)
		{
			// fill whole viewbox with image
			sx = sy = maxf(sx, sy);
			tx += viewAlign(p->viewWidth * sx, p->image->width, p->alignX) / sx;
			ty += viewAlign(p->viewHeight * sy, p->image->height, p->alignY) / sy;
		}

		// Transform
		sx *= us;
		sy *= us;
		avgs = (sx + sy) / 2.0f;
		for (shape = p->image->shapes; shape != nullptr; shape = shape->next)
		{
			shape->bounds[0] = (shape->bounds[0] + tx) * sx;
			shape->bounds[1] = (shape->bounds[1] + ty) * sy;
			shape->bounds[2] = (shape->bounds[2] + tx) * sx;
			shape->bounds[3] = (shape->bounds[3] + ty) * sy;
			for (path = shape->paths; path != nullptr; path = path->next)
			{
				path->bounds[0] = (path->bounds[0] + tx) * sx;
				path->bounds[1] = (path->bounds[1] + ty) * sy;
				path->bounds[2] = (path->bounds[2] + tx) * sx;
				path->bounds[3] = (path->bounds[3] + ty) * sy;
				for (i = 0; i < path->npts; i++)
				{
					pt = &path->pts[i * 2];
					pt[0] = (pt[0] + tx) * sx;
					pt[1] = (pt[1] + ty) * sy;
				}
			}

			if (shape->fill.type == _PAINT_LINEAR_GRADIENT || shape->fill.type == _PAINT_RADIAL_GRADIENT)
			{
				scaleGradient(shape->fill.gradient, tx, ty, sx, sy);
				memcpy(t, shape->fill.gradient->xform, sizeof(float) * 6);
				xformInverse(shape->fill.gradient->xform, t);
			}
			if (shape->stroke.type == _PAINT_LINEAR_GRADIENT || shape->stroke.type == _PAINT_RADIAL_GRADIENT)
			{
				scaleGradient(shape->stroke.gradient, tx, ty, sx, sy);
				memcpy(t, shape->stroke.gradient->xform, sizeof(float) * 6);
				xformInverse(shape->stroke.gradient->xform, t);
			}

			shape->strokeWidth *= avgs;
			shape->strokeDashOffset *= avgs;
			for (i = 0; i < shape->strokeDashCount; i++)
				shape->strokeDashArray[i] *= avgs;
		}
	}

	Image *parse(char *input, const char *Units, float dpi)
	{
		Parser *p;
		Image *ret = 0;

		p = createParser();
		if (p == nullptr)
		{
			return nullptr;
		}
		p->dpi = dpi;

		parseXML(input, startElement, endElement, content, p);

		// Scale to viewBox
		scaleToViewbox(p, Units);

		ret = p->image;
		p->image = nullptr;

		deleteParser(p);

		return ret;
	}

	Image *parseFromFile(const char *filename, const char *Units, float dpi)
	{
		FILE *fp = nullptr;
		size_t size;
		char *data = nullptr;
		Image *image = nullptr;

		fp = fopen(filename, "rb");
		if (!fp)
			goto error;
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		data = (char *)malloc(size + 1);
		if (data == nullptr)
			goto error;
		if (fread(data, 1, size, fp) != size)
			goto error;
		data[size] = '\0'; // Must be null terminated.
		fclose(fp);
		image = parse(data, Units, dpi);
		free(data);

		return image;

	error:
		if (fp)
			fclose(fp);
		if (data)
			free(data);
		if (image)
			deleteImage(image);
		return nullptr;
	}

	Path *duplicatePath(Path *p)
	{
		Path *res = nullptr;

		if (p == nullptr)
			return nullptr;

		res = (Path *)malloc(sizeof(Path));
		if (res == nullptr)
			goto error;
		memset(res, 0, sizeof(Path));

		res->pts = (float *)malloc(p->npts * 2 * sizeof(float));
		if (res->pts == nullptr)
			goto error;
		memcpy(res->pts, p->pts, p->npts * sizeof(float) * 2);
		res->npts = p->npts;

		memcpy(res->bounds, p->bounds, sizeof(p->bounds));

		res->closed = p->closed;

		return res;

	error:
		if (res != nullptr)
		{
			free(res->pts);
			free(res);
		}
		return nullptr;
	}

	void deleteImage(Image *image)
	{
		Shape *snext, *shape;
		if (image == nullptr)
			return;
		shape = image->shapes;
		while (shape != nullptr)
		{
			snext = shape->next;
			deletePaths(shape->paths);
			deletePaint(&shape->fill);
			deletePaint(&shape->stroke);
			free(shape);
			shape = snext;
		}
		free(image);
	}
}
