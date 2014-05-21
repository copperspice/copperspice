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
Q3AsciiDict<QLineEdit> fields; // char* keys, QLineEdit* values
fields.insert( "forename", new QLineEdit( this ) );
fields.insert( "surname", new QLineEdit( this ) );

fields["forename"]->setText( "Homer" );
fields["surname"]->setText( "Simpson" );

Q3AsciiDictIterator<QLineEdit> it( fields ); // See Q3AsciiDictIterator
for( ; it.current(); ++it )
    cout << it.currentKey() << ": " << it.current()->text() << endl;
cout << endl;

if ( fields["forename"] && fields["surname"] )
    cout << fields["forename"]->text() << " " 
	<< fields["surname"]->text() << endl;  // Prints "Homer Simpson"

fields.remove( "forename" ); // Does not delete the line edit
if ( ! fields["forename"] )
    cout << "forename is not in the dictionary" << endl;
//! [0]


//! [1]
Q3AsciiDict<char> dict;
    ...
if ( dict.find(key) )
    dict.remove( key );
dict.insert( key, item );
//! [1]


//! [2]
Q3AsciiDict<QLineEdit> fields;
fields.insert( "forename", new QLineEdit( this ) );
fields.insert( "surname", new QLineEdit( this ) );
fields.insert( "age", new QLineEdit( this ) );

fields["forename"]->setText( "Homer" );
fields["surname"]->setText( "Simpson" );
fields["age"]->setText( "45" );

Q3AsciiDictIterator<QLineEdit> it( fields );
for( ; it.current(); ++it )
    cout << it.currentKey() << ": " << it.current()->text() << endl;
cout << endl;

// Output (random order):
//	age: 45
//	surname: Simpson
//	forename: Homer
//! [2]
