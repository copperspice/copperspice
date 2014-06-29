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
class Employee
{
public:
    Employee() : sn( 0 ) { }
    Employee( const QString& forename, const QString& surname, int salary )
	: fn( forename ), sn( surname ), sal( salary )
    { }

    void setSalary( int salary ) { sal = salary; }

    QString forename() const { return fn; }
    QString surname() const { return sn; }
    int salary() const { return sal; }

private:
    QString fn;
    QString sn;
    int sal;
};

Q3PtrList<Employee> list;
list.setAutoDelete( TRUE ); // the list owns the objects

list.append( new Employee("John", "Doe", 50000) );
list.append( new Employee("Jane", "Williams", 80000) );
list.append( new Employee("Tom", "Jones", 60000) );

Employee *employee;
for ( employee = list.first(); employee; employee = list.next() )
    cout << employee->surname().latin1() << ", " <<
	    employee->forename().latin1() << " earns " <<
	    employee->salary() << endl;
cout << endl;

// very inefficient for big lists
for ( uint i = 0; i < list.count(); ++i )
    if ( list.at(i) )
	cout << list.at( i )->surname().latin1() << endl;
//! [0]


//! [1]
Doe, John earns 50000
Williams, Jane earns 80000
Jones, Tom earns 60000

Doe
Williams
Jones
//! [1]


//! [2]
if ( list.findRef( item ) != -1 )
    list.remove();
//! [2]


//! [3]
Q3PtrList<Employee> list;

list.append( new Employee("John", "Doe", 50000) );
list.append( new Employee("Jane", "Williams", 80000) );
list.append( new Employee("Tom", "Jones", 60000) );

Q3PtrListIterator<Employee> it( list );
Employee *employee;
while ( (employee = it.current()) != 0 ) {
    ++it;
    cout << employee->surname().latin1() << ", " <<
	    employee->forename().latin1() << " earns " <<
	    employee->salary() << endl;
}
//! [3]


//! [4]
Doe, John earns 50000
Williams, Jane earns 80000
Jones, Tom earns 60000
//! [4]
