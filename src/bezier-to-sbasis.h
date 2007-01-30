/*
 * bezier-to-sbasis.h
 *
 * Copyright 2006 Nathan Hurst <njh@mail.csse.monash.edu.au>
 * Copyright 2006 Michael G. Sloan <mgsloan@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef _BEZIER_TO_SBASIS
#define _BEZIER_TO_SBASIS

#include "multidim-sbasis.h"

namespace Geom{

template <typename T, unsigned D, unsigned order>
struct bezier_to_sbasis_impl {
    static inline MultidimSBasis<D> compute(T const &handles) {
        return multiply(BezOrd(1, 0), bezier_to_sbasis_impl<T, D, order-1>::compute(handles)) +
            multiply(BezOrd(0, 1), bezier_to_sbasis_impl<T, D, order-1>::compute(handles+1));
    }
};

template <typename T, unsigned D>
struct bezier_to_sbasis_impl<T, D, 1> {
    static inline MultidimSBasis<D> compute(T const &handles) {
        MultidimSBasis<D> mdsb;
        for(unsigned d = 0 ; d < D; d++) {
            mdsb[d] = BezOrd(handles[0][d], handles[1][d]);
        }
        return mdsb;
    }
};

template <typename T, unsigned D>
struct bezier_to_sbasis_impl<T, D, 0> {
    static inline MultidimSBasis<D> compute(T const &handles) {
        MultidimSBasis<D> mdsb;
        for(unsigned d = 0 ; d < D; d++) {
            mdsb[d] = BezOrd(handles[0][d], handles[0][d]);
        }
        return mdsb;
    }
};

template <unsigned D, unsigned order, typename T>
inline MultidimSBasis<D>
bezier_to_sbasis(T const &handles) {
    return bezier_to_sbasis_impl<T, D, order>::compute(handles);
}

};
#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
