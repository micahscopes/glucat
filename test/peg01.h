#ifndef GLUCAT_TEST_PEG01_H
#define GLUCAT_TEST_PEG01_H
/***************************************************************************
    GluCat : Generic library of universal Clifford algebra templates
    peg01.cpp : programming example 1 : 2D lengths and areas
                             -------------------
    begin                : Sun 2001-12-09
    copyright            : (C) 2001 by Paul C. Leopardi
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

namespace peg01
{
  using namespace glucat;
  using namespace std;
  template< class Multivector_T >
  void do_test1()
  {
    typedef Multivector_T number;

    const number OP("4.0{1}+3.0{2}"),
                 OQ("2.0{1}+3.0{2}");
    cout << endl
         << "OP = " << OP << endl
         << "OQ = " << OQ << endl
         << "length of side OP = "
         << abs(OP) << endl
         << "area of triangle OPQ = "
         << abs((OP ^ OQ) / 2.0) << endl;
  }
}

int test1()
{
  using namespace peg01;
  cout << "Programming example 1 : 2D lengths and areas" << endl;
  cout << endl;
  cout << "Let OPQ be a triangle, where P=(4,3), Q=(2,3) and O is the origin (0,0)." << endl;
  cout << "Calculate the length r of side OP and the area A of the triangle." << endl;
  cout << "framed_multi<double>" << endl;
  cout << endl;
  do_test1< framed_multi<double> >();
  cout << "matrix_multi<double>" << endl;
  do_test1< matrix_multi<double> >();
  return 0;
}
#endif
