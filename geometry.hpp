// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "coords.hpp"

namespace hjx::geom {

// guardrails for now
constexpr float min_fence = 0.0001;
constexpr float max_fence = 1000;

constexpr float zeroish = 1e-8;

class circle {
    xy center_;
    float radius_;

public:
    circle(xy center, float radius): center_(center), radius_(radius) {
        ASSERT_LT(center.abs(), max_fence);
        ASSERT_GT_LT(radius, min_fence, max_fence);
    }

    xy center() const { return center_; }
    float radius() const { return radius_; }

    friend std::ostream &operator<<(std::ostream &os, const circle &circ) {
        return os << "[circle " << circ.center()
                  << " " << honest_float{circ.radius()} << "]";
    }
};


class obb {
    xy center_;
    xy half_extents_;
    float cos_, sin_;

public:
    obb(xy center, xy half_extents, float cos, float sin):
        center_(center), half_extents_(half_extents), cos_(cos), sin_(sin)
    {
        ASSERT_LT(center.abs(), max_fence);
        ASSERT_GT_LT(half_extents.x(), min_fence, max_fence);
        ASSERT_GT_LT(half_extents.y(), min_fence, max_fence);
        ASSERT_EQFISH(cos * cos + sin * sin, 1);
    }

    obb(xy center, xy half_extents, angle theta):
        obb(center, half_extents, theta.cos(), theta.sin()) {}

    static obb from_axis(xy center, xy half_extents, xy axis) {
        xy u = axis.normalize();
        return obb{center, half_extents, u.x(), u.y()};
    }

    xy center() const { return center_; }
    xy half_extents()   const { return half_extents_; }
    xy right()  const { return xy{ cos_, sin_}; }   // local +x in world
    xy up()     const { return xy{-sin_, cos_}; }   // local +y in world

    xy to_local(xy p) const {
        xy v = p - center_;
        return xy{v.dot(right()), v.dot(up())};
    }

    xy dir_to_local(xy v) const { return xy{v.dot(right()), v.dot(up())}; }

    xy dir_to_world(xy v) const { return right()*v.x() + up()*v.y(); }

    friend std::ostream &operator<<(std::ostream &os, const obb &b) {
        return os << "[obb " << b.center() << " ±" << b.half_extents()
                  << " →(" << b.cos_ << "," << b.sin_ << ")]";
    }
};


class triangle {
    xy vertices[3];
    xy normals[3]; // unit outward normal of edge i: vertex(i) -> vertex(i+1)
    // consider - compute the normals on demand?

    static int indexize(int i) {
        // index 3 mapping to 0 is useful for f(vertex(i), vertex(i + 1))
        ASSERT_GE_LE(i, 0, 3);
#if 1
        return i % 3;
#else
        // consider after perf testing
        // 0 => 0
        // 1 => 2 (note winding)
        // 2 => 1
        // 3 => 0
        return (4 >> i) & 3;
#endif
    }

public:
    triangle(xy a, xy b, xy c): vertices{a, b, c} {
        xy ab = b - a;
        xy ac = c - a;
        float twice_area = ab.x() * ac.y() - ab.y() * ac.x();

        // Canonicalize to positive signed area.
        //
        // NB: "positive area" is the invariant, not "counter-clockwise" (with imgui's
        // y-down axes a positive-area triangle renders clockwise).  The algebra is
        // unaffected--only the terminology changes, so don't reason about this in
        // CW/CCW terms.
        if (twice_area < 0) {
            std::swap(vertices[1], vertices[2]);
            twice_area = -twice_area;
        }

        // sliver guard: near-collinear vertices make the normals garbage
        ASSERT_GT(twice_area, min_fence * min_fence);

        for (int i = 0; i < 3; i++) {
            xy e = vertex(i + 1) - vertex(i);
            // Vertices are canonicalized to positive signed area above, which is what
            // fixes which of e's two perps faces outward: always (e.y, -e.x).
            normals[i] = xy{e.y(), -e.x()}.normalize();
        }
    }

    xy vertex(int i) const { return vertices[indexize(i)]; }
    xy normal(int i) const { return normals[indexize(i)]; }

    friend std::ostream &operator<<(std::ostream &os, const triangle &tri) {
        return os << "[triangle " << tri.vertex(0)
                  << " " << tri.vertex(1)
                  << " " << tri.vertex(2) << "]";
    }
};


struct intersection {
    xy impact{};
    xy normal{};
    float t = nan::make(); // [0,1]

    explicit operator bool() const { return bool(impact); }

    friend std::ostream &operator<<(std::ostream &os, const intersection &meet) {
        if (!meet) return os << "nil";
        return os << "[" << meet.impact
                  << " t=" << honest_float(meet.t)
                  << " ⟂" << meet.normal << "]";
    }
};


namespace detail {

// swept-point-vs-disc primitive
// returns distance along path [0, 1] else NaN
//
// no degenerate guard (caller handles a≈0)
// no t-range clamp
// no started-inside policy
//
// f is obj relative to disc center
// a is precompted dot(delta, delta)
//
// this is the one place the quadratic lives
// circle sweep and OBB corner tests wrap it with their policies
inline float disc_entry(xy f, xy delta, float R, float a) {
    float h = f.dot(delta);
    float c = f.autodot() - R*R;
    float disc = h*h - a*c; // (b² - 4ac) / 4
    if (disc < 0) return nan::make();
    return (-h - std::sqrt(disc)) / a;
}

// ray p+Δt vs disc radius R at c.  returns first t in [0,1] or NaN.
// t<0 is rejected rather than clamped: if the ray starts inside the arc, the entry
// happened before this step, and sweep_local's started-inside branch already handles it.
inline float ray_disc(xy p, xy delta, xy c, float R, float a) {
    float t = disc_entry(p - c, delta, R, a);
    return t >= 0 && t <= 1 ? t : nan::make();
}

// disc at a box corner--on hit writes outward local normal.  sharp box => miss.
inline float corner_hit(xy p, xy delta, xy corner, float R, float a, xy &nrm) {
    if (R <= zeroish) return nan::make();
    float t = ray_disc(p, delta, corner, R, a);
    if (std::isnan(t)) return nan::make();
    nrm = ((p + delta*t) - corner).normalize();
    return t;
}

// swept point p+Δt vs box-local AABB (half-extents ext) rounded by R.
// returns t in [0,1] or NaN.  on hit writes outward local unit normal.
inline float sweep_local(xy p, xy delta, xy ext, float R, xy &nrm) {
    float dsmag = delta.autodot();

    // zeroish-length sweep: static rounded-rect overlap test
    if (dsmag <= zeroish*zeroish) {
        xy q{std::clamp(p.x(), -ext.x(), ext.x()),
             std::clamp(p.y(), -ext.y(), ext.y())};

        xy off = p - q;
        if (off.autodot() > R*R) return nan::make();

        nrm = off.autodot() <= zeroish*zeroish ? xy{1, 0} : off.normalize();

        return 0;
    }

    float tmin = 0, tmax = 1, esign = 0;
    int axis_idx = -1;

    // slab rejection test against the inflated AABB (ext + R per axis)
    for (size_t i = 0; i < 2; i++) {
        float E = ext.slot(i) + R;

        if (std::fabs(delta.slot(i)) <= zeroish) {
            if (p.slot(i) < -E || p.slot(i) > E) return nan::make(); // parallel & outside
            continue;
        }

        float inv = 1.f / delta.slot(i);
        float t1 = (-E - p.slot(i)) * inv;
        float t2 = (E - p.slot(i)) * inv;
        float s = -1;

        if (t1 > t2) { std::swap(t1, t2); s = 1; }
        if (t1 > tmin) { tmin = t1; axis_idx = i; esign = s; }
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return nan::make();
    }

    if (tmin > 1) return nan::make();

    // started already inside the inflated AABB
    if (axis_idx < 0) {
        xy q{std::clamp(p.x(), -ext.x(), ext.x()),
             std::clamp(p.y(), -ext.y(), ext.y())};
        xy off = p - q;

        if (off.autodot() <= R*R) { // overlapping the rounded rect at t=0
            nrm = off.autodot() <= zeroish*zeroish ? -delta.normalize() : off.normalize();
            return 0;
        }

        // corner notch: inside the inflated box but outside the rounded rect
        // only the adjacent corner disc is reachable going forward
        xy corner{p.x() > 0 ? ext.x() : -ext.x(),
                  p.y() > 0 ? ext.y() : -ext.y()};
        return corner_hit(p, delta, corner, R, dsmag, nrm);
    }

    // contact on inflated AABB
    // past the original extent on the *other* axis => corner arc
    xy q = p + delta * tmin;
    if (axis_idx == 0) {
        if (std::fabs(q.y()) <= ext.y()) { nrm = xy{esign, 0}; return tmin; }

        xy corner{esign * ext.x(),
                  q.y() > 0 ? ext.y() : -ext.y()};

        return corner_hit(p, delta, corner, R, dsmag, nrm);
    }
    else {
        if (std::fabs(q.x()) <= ext.x()) { nrm = xy{0, esign}; return tmin; }

        xy corner{q.x() > 0 ? ext.x() : -ext.x(),
                  esign * ext.y()};

        return corner_hit(p, delta, corner, R, dsmag, nrm);
    }
}


inline xy closest_on_segment(xy p, xy v0, xy v1) {
    xy d = v1 - v0;
    float u = std::clamp((p - v0).dot(d) / d.autodot(), 0.f, 1.f);
    return v0 + d * u;
}

// swept point p+Δt vs triangle rounded by R (world frame; triangle carries its own
// orientation, unlike the OBB there is no local frame to enter).  returns t in [0,1]
// or NaN.  on hit writes outward unit normal.
//
// rounded triangle = 3 faces offset outward by R + 3 vertex discs radius R.  min-t
// over the six primitives.  faces reject receding and already-past starts; vertex
// discs go through ray_disc, inheriting its t<0 rejection, which is what makes the
// notch (inside the offset wedge, outside the disc) come out right: sweeping away
// finds nothing, sweeping in enters the cap.
inline float sweep_tri(xy p, xy delta, const triangle &tri, float R, xy &nrm) {
    // signed distance to each face line; all <= 0 means inside the solid
    float d[3];
    for (int i = 0; i < 3; i++) d[i] = (p - tri.vertex(i)).dot(tri.normal(i));
    ASSERT_MSG(d[0] < 0 || d[1] < 0 || d[2] < 0, tri);

    // closest point on the solid triangle (exact, not the offset-plane
    // approximation--that would call the notch a penetration)
    xy q = p;

    if (d[0] > 0 || d[1] > 0 || d[2] > 0) {
        float best2 = inf;

        for (int i = 0; i < 3; i++) {
            xy qi = closest_on_segment(p, tri.vertex(i), tri.vertex(i + 1));
            float di2 = (p - qi).autodot();
            if (di2 < best2) { best2 = di2; q = qi; }
        }
    }

    xy off = p - q;

    float dsmag = delta.autodot();

    // zeroish-length sweep: static rounded-triangle overlap test
    if (dsmag <= zeroish*zeroish) {
        if (off.autodot() > R*R) return nan::make();

        nrm = off.autodot() <= zeroish*zeroish
            ? tri.normal(d[1] > d[0] ? (d[2] > d[1] ? 2 : 1) : (d[2] > d[0] ? 2 : 0))
            : off.normalize();

        return 0;
    }

    // started inside the rounded triangle: starting penetration
    if (off.autodot() <= R*R) {
        nrm = off.autodot() <= zeroish*zeroish ? -delta.normalize() : off.normalize();
        return 0;
    }

    float best = nan::make();

    // three faces, offset outward by R; entering crossings only
    for (int i = 0; i < 3; i++) {
        xy n = tri.normal(i);
        float dn = delta.dot(n);
        if (dn >= -zeroish) continue; // parallel or receding
        float s = (p - tri.vertex(i)).dot(n) - R;
        if (s < 0) continue; // already past the offset line
        float t = s / -dn;
        if (t > 1 || (!std::isnan(best) && t >= best)) continue;
        xy hit = p + delta * t;
        xy e = tri.vertex(i + 1) - tri.vertex(i);
        float u = (hit - tri.vertex(i)).dot(e);
        if (u < 0 || u > e.autodot()) continue; // off the face span: cap territory
        best = t;
        nrm = n;
    }

    // three vertex caps; sharp triangle => no caps
    if (R > zeroish) {
        for (int i = 0; i < 3; i++) {
            float t = ray_disc(p, delta, tri.vertex(i), R, dsmag);
            if (std::isnan(t) || (!std::isnan(best) && t >= best)) continue;
            best = t;
            nrm = ((p + delta*t) - tri.vertex(i)).normalize();
        }
    }

    return best;
}

}


// point-vs-circle sweep
// returns distance along path [0,1] else NaN
inline float intersect(xy obj, xy delta, const circle &target) {
    float R = target.radius();
    xy f = obj - target.center();
    float a = delta.autodot();

    // zeroish-length sweep: static overlap test
    if (a <= zeroish*zeroish) return f.autodot() <= R*R ? 0.f : nan::make();

    float t = detail::disc_entry(f, delta, R, a);
    if (std::isnan(t) || t > 1) return nan::make();

    // started inside or behind: re-test overlap at the start
    if (t < 0) return f.autodot() - R*R <= 0 ? 0.f : nan::make();

    return t;
}


inline intersection intersect(const circle &obj, xy delta, const circle &target) {
    circle inflated{target.center(), obj.radius() + target.radius()};
    float t = intersect(obj.center(), delta, inflated);
    if (std::isnan(t)) return intersection{};

    xy displaced = obj.center() + delta * t;
    xy offset = displaced - target.center();
    xy normal = offset.autodot() <= zeroish*zeroish
        ? -delta.normalize()
        : offset.normalize();

    return {
        .impact = displaced - normal * obj.radius(),
        .normal = normal,
        .t = t
    };
}


// point-vs-obb sweep
// returns distance along path [0,1] else NaN
inline float intersect(xy obj, xy delta, const obb &target) {
    xy nrm;
    return detail::sweep_local(target.to_local(obj),
                               target.dir_to_local(delta),
                               target.half_extents(),
                               0,
                               nrm);
}

inline intersection intersect(const circle &obj, xy delta, const obb &target) {
    xy nlocal;

    float t = detail::sweep_local(target.to_local(obj.center()),
                                  target.dir_to_local(delta),
                                  target.half_extents(),
                                  obj.radius(),
                                  nlocal);

    if (std::isnan(t)) return intersection{};

    xy normal = target.dir_to_world(nlocal);
    xy displaced = obj.center() + delta * t;

    return {
        .impact = displaced - normal * obj.radius(), // mover-surface contact
        .normal = normal,
        .t = t
    };
}


// point-vs-triangle sweep
// returns distance along path [0,1] else NaN
inline float intersect(xy obj, xy delta, const triangle &target) {
    xy nrm;
    return detail::sweep_tri(obj, delta, target, 0, nrm);
}

inline intersection intersect(const circle &obj, xy delta, const triangle &target) {
    xy normal;

    float t = detail::sweep_tri(obj.center(), delta, target, obj.radius(), normal);

    if (std::isnan(t)) return intersection{};

    xy displaced = obj.center() + delta * t;

    return {
        .impact = displaced - normal * obj.radius(), // mover-surface contact
        .normal = normal,
        .t = t
    };
}

}
