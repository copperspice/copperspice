/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

//! [0]
#include <qvaluevector.h>
#include <qstring.h>
#include <stdio.h>

class Employee
{
public:
    Employee(): s(0) {}
    Employee( const QString& name, int salary )
	: n( name ), s( salary )
    { }

    QString name()   const	 	{ return n; }
    int	salary() const	 	{ return s; }
    void	setSalary( int salary )	{ s = salary; }
private:
    QString n;
    int     s;
};

int main()
{
    typedef Q3ValueVector<Employee> EmployeeVector;
    EmployeeVector vec( 3 );  // vector of 3 Employees

    vec[0] = Employee( "Bill", 50000 );
    vec[1] = Employee( "Steve", 80000 );
    vec[2] = Employee( "Ron", 60000 );

    Employee joe( "Joe", 50000 );
    vec.push_back( joe );  // vector expands to accommodate 4 Employees
    joe.setSalary( 70000 );

    EmployeeVector::iterator it;
    for( it = vec.begin(); it != vec.end(); ++it )
	printf( "%s earns %d\n", (*it).name().latin1(), (*it).salary() );

    return 0;
}
//! [0]


//! [1]
Bill earns 50000
Steve earns 80000
Ron earns 60000
Joe earns 50000
//! [1]


//! [2]
Q3ValueVector<int> vec1;  // an empty vector
vec1[10] = 4;  // WARNING: undefined, probably a crash

Q3ValueVector<QString> vec2(25); // initialize with 25 elements
vec2[10] = "Dave";  // OK
//! [2]


//! [3]
void func( Q3ValueVector<int>& vec )
{
    if ( vec.size() > 10 ) {
	vec[9] = 99; // OK
    }
};
//! [3]


//! [4]
Q3ValueVector<int> vec( 3 );
vec.push_back( 1 );
vec.push_back( 2 );
vec.push_back( 3 );
...
if ( !vec.empty() ) {
    // OK: modify the first element
    int& i = vec.front();
    i = 18;
}
...
Q3ValueVector<double> dvec;
double d = dvec.back(); // undefined behavior
//! [4]
