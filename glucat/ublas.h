#ifndef _GLUCAT_UBLAS_H
#define _GLUCAT_UBLAS_H
#ifndef _GLUCAT_HAVE_UBLAS_LU_H
/***************************************************************************
    GluCat : Generic library of universal Clifford algebra templates
    ublas.h : Declare uBLAS interface
                             -------------------
    begin                : Sun 2001-12-09
    copyright            : (C) 2001 by Paul C. Leopardi
                         : uBLAS interface contributed by Joerg Walter
    email                : leopardi@bigpond.net.au
 ***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of the  *
 *   License, or (at your option) any later version.                       *
 *   See http://www.fsf.org/copyleft/lesser.html for details               *
 ***************************************************************************
 This library is based on a prototype written by Arvind Raja and was
 licensed under the LGPL with permission of the author. See Arvind Raja,
 "Object-oriented implementations of Clifford algebras in C++: a prototype",
 in Ablamowicz, Lounesto and Parra (eds.)
 "Clifford algebras with numeric and symbolic computations", Birkhauser, 1996.
 ***************************************************************************
     See also Arvind Raja's original header comments in glucat.h
 ***************************************************************************/

namespace boost { namespace numeric {
  namespace ublas {
    // uBLAS interface by Joerg Walter
    /// Permutation matrix for use in LU factorize and solve
    template<class T, class A = ublas::unbounded_array<T> >
    class permutation_matrix :
      public ublas::vector<T, A>
    {
    public:
      typedef typename ublas::vector<T, A>::size_type size_type;

      BOOST_UBLAS_EXPLICIT
      BOOST_UBLAS_INLINE
      permutation_matrix (size_type size):
        ublas::vector<T, A> (size) {}
    };

    /// Swap rows of a vector
    template<class PM, class MV>
    void
    swap_rows (const PM &pm, MV &mv, ublas::vector_tag);

    /// Swap rows of a matrix
    template<class PM, class MV>
    void
    swap_rows (const PM &pm, MV &mv, ublas::matrix_tag);

    /// Swap rows of a matrix or a vector: dispatcher
    template<class PM, class MV>
    void
    swap_rows (const PM &pm, MV &mv);

    /// LU factorize: return LU in place of matrix
    template<class M, class PM>
    typename M::size_type
    lu_factorize (M &m, PM &pm);

    /// LU solve: linear solve using LU and permutation
    template<class M, class PM, class MV>
    void
    lu_substitute (M &m, const PM &pm, MV &mv);
  }
} }
#endif  // _GLUCAT_HAVE_UBLAS_LU_H
#endif  // _GLUCAT_UBLAS_H
