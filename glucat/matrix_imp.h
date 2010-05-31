#ifndef _GLUCAT_MATRIX_IMP_H
#define _GLUCAT_MATRIX_IMP_H
/***************************************************************************
    GluCat : Generic library of universal Clifford algebra templates
    matrix_imp.h : Implement common matrix functions
                             -------------------
    begin                : Sun 2001-12-09
    copyright            : (C) 2001-2009 by Paul C. Leopardi
                         : uBLAS interface contributed by Joerg Walter
 ***************************************************************************

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.

 ***************************************************************************
 This library is based on a prototype written by Arvind Raja and was
 licensed under the LGPL with permission of the author. See Arvind Raja,
 "Object-oriented implementations of Clifford algebras in C++: a prototype",
 in Ablamowicz, Lounesto and Parra (eds.)
 "Clifford algebras with numeric and symbolic computations", Birkhauser, 1996.
 ***************************************************************************
     See also Arvind Raja's original header comments in glucat.h
 ***************************************************************************/

#include <set>
#if defined(_GLUCAT_USE_ALGLIB)
#include <alglib/evd.h>
#endif
#if defined(_GLUCAT_USE_BINDINGS_V1)
#include <boost/numeric/bindings/lapack/workspace.hpp>
#include <boost/numeric/bindings/lapack/gees.hpp>
#include <boost/numeric/bindings/traits/ublas_matrix.hpp>
#endif
#if defined(_GLUCAT_USE_BINDINGS)
#include <boost/numeric/bindings/lapack/driver/gees.hpp>
#include <boost/numeric/bindings/ublas.hpp>
#endif

namespace glucat { namespace matrix
{
  /// Kronecker tensor product of matrices - as per Matlab kron
  template< typename LHS_T, typename RHS_T >
  const
  RHS_T
  kron(const LHS_T& lhs, const RHS_T& rhs)
  {
    typedef RHS_T matrix_t;
    typedef typename matrix_t::size_type matrix_index_t;
    const matrix_index_t rhs_s1 = rhs.size1();
    const matrix_index_t rhs_s2 = rhs.size2();
    matrix_t result(lhs.size1()*rhs_s1, lhs.size2()*rhs_s2);
    result.clear();

    typedef typename matrix_t::value_type Scalar_T;
    typedef typename LHS_T::const_iterator1 lhs_const_iterator1;
    typedef typename LHS_T::const_iterator2 lhs_const_iterator2;
    typedef typename RHS_T::const_iterator1 rhs_const_iterator1;
    typedef typename RHS_T::const_iterator2 rhs_const_iterator2;
    for (lhs_const_iterator1
        lhs_it1 = lhs.begin1();
        lhs_it1 != lhs.end1();
        ++lhs_it1)
      for (lhs_const_iterator2
          lhs_it2 = lhs_it1.begin();
          lhs_it2 != lhs_it1.end();
          ++lhs_it2)
      {
        const matrix_index_t start1 = rhs_s1 * lhs_it2.index1();
        const matrix_index_t start2 = rhs_s2 * lhs_it2.index2();
        const Scalar_T& lhs_val = *lhs_it2;
        for (rhs_const_iterator1
            rhs_it1 = rhs.begin1();
            rhs_it1 != rhs.end1();
            ++rhs_it1)
          for (rhs_const_iterator2
              rhs_it2 = rhs_it1.begin();
              rhs_it2 != rhs_it1.end();
              ++rhs_it2)
            result(start1 + rhs_it2.index1(), start2 + rhs_it2.index2()) = lhs_val * *rhs_it2;
      }
    return result;
  }

  /// Sparse Kronecker tensor product of monomial matrices
  template< typename LHS_T, typename RHS_T >
  const
  RHS_T
  mono_kron(const LHS_T& lhs, const RHS_T& rhs)
  {
    typedef RHS_T matrix_t;
    typedef typename matrix_t::size_type matrix_index_t;
    const matrix_index_t rhs_s1 = rhs.size1();
    const matrix_index_t rhs_s2 = rhs.size2();
    const matrix_index_t dim = lhs.size1()*rhs_s1;
    matrix_t result(dim, dim, dim);
    result.clear();

    typedef typename matrix_t::value_type Scalar_T;
    typedef typename LHS_T::const_iterator1 lhs_const_iterator1;
    typedef typename LHS_T::const_iterator2 lhs_const_iterator2;
    typedef typename RHS_T::const_iterator1 rhs_const_iterator1;
    typedef typename RHS_T::const_iterator2 rhs_const_iterator2;
    for (lhs_const_iterator1
        lhs_it1 = lhs.begin1();
        lhs_it1 != lhs.end1();
        ++lhs_it1)
    {
      const lhs_const_iterator2 lhs_it2 = lhs_it1.begin();
      const matrix_index_t start1 = rhs_s1 * lhs_it2.index1();
      const matrix_index_t start2 = rhs_s2 * lhs_it2.index2();
      const Scalar_T& lhs_val = *lhs_it2;
      for (rhs_const_iterator1
          rhs_it1 = rhs.begin1();
          rhs_it1 != rhs.end1();
          ++rhs_it1)
      {
        const rhs_const_iterator2 rhs_it2 = rhs_it1.begin();
        result(start1 + rhs_it2.index1(), start2 + rhs_it2.index2()) = lhs_val * *rhs_it2;
      }
    }
    return result;
  }

  /// Left inverse of Kronecker product
  template< typename LHS_T, typename RHS_T >
  const
  RHS_T
  nork(const LHS_T& lhs, const RHS_T& rhs, const bool mono)
  {
    // nork(A, kron(A, B)) is close to B
    typedef RHS_T matrix_t;
    typedef typename LHS_T::size_type lhs_index_t;
    typedef typename matrix_t::size_type matrix_index_t;
    const lhs_index_t lhs_s1 = lhs.size1();
    const lhs_index_t lhs_s2 = lhs.size2();
    const matrix_index_t rhs_s1 = rhs.size1();
    const matrix_index_t rhs_s2 = rhs.size2();
    const matrix_index_t res_s1 = rhs_s1 / lhs_s1;
    const matrix_index_t res_s2 = rhs_s2 / lhs_s2;
    typedef typename matrix_t::value_type Scalar_T;
    const Scalar_T& nnz_lhs = Scalar_T( double(mono ? lhs_s1 : nnz(lhs)) );
    if (!mono)
    {
      typedef error<matrix_t> error_t;
      if (rhs_s1 == 0)
        throw error_t("matrix", "nork: number of rows must not be 0");
      if (rhs_s2 == 0)
        throw error_t("matrix", "nork: number of cols must not be 0");
      if (res_s1 * lhs_s1 != rhs_s1)
        throw error_t("matrix", "nork: incompatible numbers of rows");
      if (res_s2 * lhs_s2 != rhs_s2)
        throw error_t("matrix", "nork: incompatible numbers of cols");
      if (nnz_lhs == Scalar_T(0))
        throw error_t("matrix", "nork: LHS must not be 0");
    }
    matrix_t result(res_s1, res_s2);
    result.clear();
    for (typename LHS_T::const_iterator1
        lhs_it1 = lhs.begin1();
        lhs_it1 != lhs.end1();
        ++lhs_it1)
      for (typename LHS_T::const_iterator2
          lhs_it2 = lhs_it1.begin();
          lhs_it2 != lhs_it1.end();
          ++lhs_it2)
        if (*lhs_it2 != Scalar_T(0))
        {
          typedef ublas::matrix_range<const matrix_t> matrix_range_t;
          const Scalar_T& lhs_scale = *lhs_it2 * nnz_lhs;
          const matrix_index_t start1 = res_s1 * lhs_it2.index1();
          const matrix_index_t start2 = res_s2 * lhs_it2.index2();
          using ublas::range;
          const range& range1 = range(start1, start1 + res_s1);
          const range& range2 = range(start2, start2 + res_s2);
          const matrix_range_t& rhs_range = matrix_range_t(rhs, range1, range2);
          for (typename matrix_range_t::const_iterator1
              rhs_it1 = rhs_range.begin1();
              rhs_it1 != rhs_range.end1();
              ++rhs_it1)
            for (typename matrix_range_t::const_iterator2
                rhs_it2 = rhs_it1.begin();
                rhs_it2 != rhs_it1.end();
                ++rhs_it2)
                result(rhs_it2.index1(), rhs_it2.index2()) += *rhs_it2 / lhs_scale;
        }
    return result;
  }

  /// Number of non-zeros
  template< typename Matrix_T >
  typename Matrix_T::size_type
  nnz(const Matrix_T& m)
  {
    typedef Matrix_T matrix_t;
    typedef typename matrix_t::size_type matrix_index_t;
    typedef typename matrix_t::const_iterator1 const_iterator1;
    typedef typename matrix_t::const_iterator2 const_iterator2;
    matrix_index_t result = 0;
    for (const_iterator1
        it1 = m.begin1();
        it1 != m.end1();
        ++it1)
      for (const_iterator2
          it2 = it1.begin();
          it2 != it1.end();
          ++it2)
        if (*it2 != 0)
          ++result;
    return result;
  }

  /// Not a Number
  template< typename Matrix_T >
  bool
  isnan(const Matrix_T& m)
  {
    typedef Matrix_T matrix_t;
    typedef typename matrix_t::value_type Scalar_T;
    typedef typename matrix_t::size_type matrix_index_t;
    typedef typename matrix_t::const_iterator1 const_iterator1;
    typedef typename matrix_t::const_iterator2 const_iterator2;
    for (const_iterator1
        it1 = m.begin1();
        it1 != m.end1();
        ++it1)
      for (const_iterator2
          it2 = it1.begin();
          it2 != it1.end();
          ++it2)
        if (numeric_traits<Scalar_T>::isNaN(*it2))
          return true;

    return false;
  }

  /// Unit matrix - as per Matlab eye
  template< typename Matrix_T >
  inline
  const
  Matrix_T
  unit(const typename Matrix_T::size_type dim)
  {
    typedef typename Matrix_T::value_type Scalar_T;
    return ublas::identity_matrix<Scalar_T>(dim);
  }

  /// Product of monomial matrices
  template< typename LHS_T, typename RHS_T >
  const typename RHS_T::expression_type
  mono_prod(const ublas::matrix_expression<LHS_T>& lhs,
            const ublas::matrix_expression<RHS_T>& rhs)
  {
    typedef const LHS_T lhs_expression_t;
    typedef const RHS_T rhs_expression_t;
    typedef typename RHS_T::expression_type matrix_t;
    typedef typename matrix_t::size_type  matrix_index_t;
    typedef typename matrix_t::value_type Scalar_T;
    typedef typename lhs_expression_t::const_iterator1   lhs_const_iterator1;
    typedef typename lhs_expression_t::const_iterator2   lhs_const_iterator2;
    typedef typename ublas::matrix_row<rhs_expression_t> matrix_row_t;
    typedef typename matrix_row_t::const_iterator        row_const_iterator;

    const matrix_index_t dim = lhs().size1();
    // The following assumes that matrix_t is a sparse matrix type.
    matrix_t result = matrix_t(dim, dim, dim);
    for (lhs_const_iterator1
        lhs_row = lhs().begin1();
        lhs_row != lhs().end1();
        ++lhs_row)
    {
      const lhs_const_iterator2& lhs_it = lhs_row.begin();
      if (lhs_it != lhs_row.end())
      {
        const matrix_row_t& rhs_row = matrix_row_t(rhs(), lhs_it.index2());
        const row_const_iterator& rhs_it = rhs_row.begin();
        if (rhs_it != rhs_row.end())
          result(lhs_it.index1(), rhs_it.index()) = (*lhs_it) * (*rhs_it);
      }
    }
    return result;
  }

  /// Product of sparse matrices
  template< typename LHS_T, typename RHS_T >
  inline
  const typename RHS_T::expression_type
  sparse_prod(const ublas::matrix_expression<LHS_T>& lhs,
              const ublas::matrix_expression<RHS_T>& rhs)
  {
    typedef typename RHS_T::expression_type expression_t;
    return ublas::sparse_prod<expression_t>(lhs(), rhs());
  }

  /// Product of matrices
  template< typename LHS_T, typename RHS_T >
  inline
  const typename RHS_T::expression_type
  prod(const ublas::matrix_expression<LHS_T>& lhs,
       const ublas::matrix_expression<RHS_T>& rhs)
  {
#ifdef _GLUCAT_USE_DENSE_MATRICES
    typedef typename RHS_T::size_type matrix_index_t;
    const matrix_index_t dim = lhs().size1();
    RHS_T result(dim, dim);
    ublas::axpy_prod(lhs, rhs, result, true);
    return result;
#else
    typedef typename RHS_T::expression_type expression_t;
    return ublas::sparse_prod<expression_t>(lhs(), rhs());
#endif
  }

  /// Inner product: sum(lhs(i,j)*rhs(i,j))/lhs.nrows()
  template< typename Scalar_T, typename LHS_T, typename RHS_T >
  Scalar_T
  inner(const LHS_T& lhs, const RHS_T& rhs)
  {
    Scalar_T result = Scalar_T(0);
    for (typename LHS_T::const_iterator1
        lhs_it1 = lhs.begin1();
        lhs_it1 != lhs.end1();
        ++lhs_it1)
      for (typename LHS_T::const_iterator2
          lhs_it2 = lhs_it1.begin();
          lhs_it2 != lhs_it1.end();
          ++lhs_it2)
      {
        const Scalar_T& rhs_val = rhs(lhs_it2.index1(),lhs_it2.index2());
        if (rhs_val != Scalar_T(0))
          result += (*lhs_it2) * rhs_val;
      }
    return result / lhs.size1();
  }

  /// Square of Frobenius norm
  template< typename Matrix_T >
  typename Matrix_T::value_type
  norm_frob2(const Matrix_T& val)
  {
    typedef typename Matrix_T::value_type Scalar_T;
    typedef typename Matrix_T::size_type matrix_index_t;

    Scalar_T result = Scalar_T(0);
    for (typename Matrix_T::const_iterator1
        val_it1 = val.begin1();
        val_it1 != val.end1();
        ++val_it1)
      for (typename Matrix_T::const_iterator2
          val_it2 = val_it1.begin();
          val_it2 != val_it1.end();
          ++val_it2)
      {
        if (numeric_traits<Scalar_T>::isNaN(*val_it2))
          return numeric_traits<Scalar_T>::NaN();
        result += (*val_it2) * (*val_it2);
      }
    return result;
  }

  /// Matrix trace
  template< typename Matrix_T >
  typename Matrix_T::value_type
  trace(const Matrix_T& val)
  {
    typedef typename Matrix_T::value_type Scalar_T;
    typedef typename Matrix_T::size_type matrix_index_t;

    Scalar_T result = Scalar_T(0);
    matrix_index_t dim = val.size1();
    for (matrix_index_t ndx=0;
        ndx != dim;
        ++ndx)
    {
      const Scalar_T crd = val(ndx, ndx);
      if (numeric_traits<Scalar_T>::isNaN(crd))
        return numeric_traits<Scalar_T>::NaN();
      result += crd;
    }
    return result;
  }

#if defined(_GLUCAT_USE_ALGLIB)
  /// Convert matrix to ALGLIB format
  template< typename Matrix_T >
  static
  ap::real_2d_array
  to_alglib(const Matrix_T& val)
  {
    typedef typename Matrix_T::size_type matrix_index_t;
    const matrix_index_t s1 = val.size1();
    const matrix_index_t s2 = val.size2();

    ap::real_2d_array result;
    result.setlength(int(s1), int(s2));

    typedef typename Matrix_T::value_type Scalar_T;
    typedef numeric_traits<Scalar_T> traits_t;

    matrix_index_t j;
    matrix_index_t k;
    for (j = 0;
        j != s1;
        ++j)
      for(k = 0;
          k != s2;
          ++k)
         result(int(j), int(k)) = 0.0;

    typedef typename Matrix_T::const_iterator1 const_iterator1;
    typedef typename Matrix_T::const_iterator2 const_iterator2;
    for (const_iterator1
        val_it1 = val.begin1();
        val_it1 != val.end1();
        ++val_it1)
      for (const_iterator2
          val_it2 = val_it1.begin();
          val_it2 != val_it1.end();
          ++val_it2)
         result(int(val_it2.index1()), int(val_it2.index2())) = traits_t::to_double(*val_it2);

    return result;
  }
#endif

#if defined(_GLUCAT_USE_BINDINGS_V1) || defined(_GLUCAT_USE_BINDINGS)
  /// Convert matrix to LAPACK format
  template< typename Matrix_T >
  static
  ublas::matrix<double, ublas::column_major>
  to_lapack(const Matrix_T& val)
  {
    typedef typename Matrix_T::size_type matrix_index_t;
    const matrix_index_t s1 = val.size1();
    const matrix_index_t s2 = val.size2();

    typedef typename ublas::matrix<double, ublas::column_major> lapack_matrix_t;
    lapack_matrix_t result(s1, s2);
    result.clear();

    typedef typename Matrix_T::value_type Scalar_T;
    typedef numeric_traits<Scalar_T> traits_t;

    typedef typename Matrix_T::const_iterator1 const_iterator1;
    typedef typename Matrix_T::const_iterator2 const_iterator2;
    for (const_iterator1
        val_it1 = val.begin1();
        val_it1 != val.end1();
        ++val_it1)
      for (const_iterator2
          val_it2 = val_it1.begin();
          val_it2 != val_it1.end();
          ++val_it2)
         result(val_it2.index1(), val_it2.index2()) = traits_t::to_double(*val_it2);

      return result;
  }
#endif

  /// Eigenvalues of a matrix
  template< typename Matrix_T >
  ublas::vector< std::complex<double> >
  eigenvalues(const Matrix_T& val)
  {
    typedef std::complex<double> complex_t;
    typedef typename ublas::vector<complex_t> complex_vector_t;
    typedef typename Matrix_T::size_type matrix_index_t;

    const matrix_index_t dim = val.size1();
    complex_vector_t lambda = complex_vector_t(dim);
    lambda.clear();

#if defined(_GLUCAT_USE_ALGLIB)
    ap::real_2d_array T = to_alglib(val);
    const int n = dim;
    ap::real_1d_array real_lambda;
    real_lambda.setlength(n);
    ap::real_1d_array imag_lambda;
    imag_lambda.setlength(n);
    ap::real_2d_array vl;
    ap::real_2d_array vr;

    rmatrixevd(T, n, 0, real_lambda, imag_lambda, vl, vr);

    lambda.clear();
    for (int  k=0; k!= n; ++k)
      lambda[k] = complex_t(real_lambda(k), imag_lambda(k));
#endif
#if defined(_GLUCAT_USE_BINDINGS_V1) || defined(_GLUCAT_USE_BINDINGS)
    namespace lapack = boost::numeric::bindings::lapack;
    typedef typename ublas::matrix<double, ublas::column_major> lapack_matrix_t;

    lapack_matrix_t T = to_lapack(val);
    lapack_matrix_t V = T;
#if defined(_GLUCAT_USE_BINDINGS_V1)

    lapack::gees (T, lambda, V, lapack::optimal_workspace() );

#elif defined(_GLUCAT_USE_BINDINGS)
    typedef typename ublas::vector<double> vector_t;
    vector_t real_lambda = vector_t(dim);
    vector_t imag_lambda = vector_t(dim);
    fortran_int_t sdim = 0;

    lapack::gees ('N', 'N', (logical_t*)0, T, sdim, real_lambda, imag_lambda, V );

    lambda.clear();
    for (vector_t::size_type  k=0; k!= dim; ++k)
      lambda[k] = complex_t(real_lambda[k], imag_lambda[k]);
#endif
#endif
    return lambda;
  }

  /// Classify the eigenvalues of a matrix
  template< typename Matrix_T >
  eig_genus<Matrix_T>
  classify_eigenvalues(const Matrix_T& val)
  {
    typedef typename Matrix_T::value_type Scalar_T;
    eig_genus<Matrix_T> result;
    result.m_eig_case = safe_eig_case;
    result.m_safe_arg = Scalar_T(0);

    typedef std::complex<double> complex_t;
    typedef typename ublas::vector<complex_t> complex_vector_t;
    complex_vector_t lambda = eigenvalues(val);

    std::set<double> arg_set;

    typedef typename complex_vector_t::size_type vector_index_t;
    const vector_index_t dim = lambda.size();
    static const double epsilon = std::numeric_limits<float>::epsilon();

    bool negative_eig_found = false;
    bool imaginary_eig_found = false;
    for (vector_index_t k=0; k != dim; ++k)
    {
      const complex_t lambda_k = lambda[k];
      arg_set.insert(std::arg(lambda_k));

      const double real_lambda_k = std::real(lambda_k);
      const double imag_lambda_k = std::imag(lambda_k);
      const double norm_tol = 1024.0*epsilon*epsilon*std::norm(lambda_k);

      if (!negative_eig_found &&
          real_lambda_k < -epsilon &&
         (imag_lambda_k == 0.0 ||
          imag_lambda_k * imag_lambda_k < norm_tol))
        negative_eig_found = true;
      if (!imaginary_eig_found &&
          std::abs(imag_lambda_k) > epsilon &&
         (real_lambda_k == 0.0 ||
          real_lambda_k * real_lambda_k < norm_tol))
        imaginary_eig_found = true;
    }

    static const double pi = numeric_traits<double>::pi();
    if (negative_eig_found)
    {
      if (imaginary_eig_found)
        result.m_eig_case = both_eig_case;
      else
      {
        result.m_eig_case = negative_eig_case;
        result.m_safe_arg = -pi/2.0;
      }
    }

    if (result.m_eig_case == both_eig_case)
    {
      typename std::set<double>::const_iterator arg_it = arg_set.begin();
      double first_arg = *arg_it;
      double best_arg = first_arg;
      double best_diff = 0.0;
      double previous_arg = first_arg;
      for (++arg_it;
          arg_it != arg_set.end();
          ++arg_it)
      {
        const double arg_diff = *arg_it - previous_arg;
        if (arg_diff > best_diff)
        {
          best_diff = arg_diff;
          best_arg = previous_arg;
        }
        previous_arg = *arg_it;
      }
      const double arg_diff = first_arg + 2.0*pi - previous_arg;
      if (arg_diff > best_diff)
      {
        best_diff = arg_diff;
        best_arg = previous_arg;
      }
      result.m_safe_arg = pi - (best_arg + best_diff / 2.0);
    }
    return result;
  }
} }

#endif  // _GLUCAT_MATRIX_IMP_H
