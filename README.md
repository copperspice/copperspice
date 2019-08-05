## CopperSpice

### Introduction

CopperSpice is a set of C++ libraries used to develop cross-platform software applications. This is an open source project
released under the LGPL V2.1 license. CopperSpice was derived from the Qt framework and has diverged as the libraries
are enhanced to work seamlessly with the C++ standard library. Our motivation for developing CopperSpice was to change the
core design to provide a consistent and feature rich API.

The libraries provided in CopperSpice include:

 * CsCore
 * CsGui
 * CsMultimedia
 * CsNetwork
 * CsOpenGL
 * CsScript
 * CsSql
 * CsSvg
 * CsWebKit
 * CsXml
 * CsXmlPatterns

There are also three open source BSD licensed libraries which are used by CopperSpice and available as stand alone
libraries for anyone developing a C++ application.

 * CsSignal
 * CsString
 * libGuarded


###### Major Enhancements

Reflection
 * No Meta-Object Compiler is required for generating meta data, all references were removed
 * The functionality provided by moc was replaced with compile time templates
 * CopperSpice automatically generates meta data for processing Signals/ Slots and Introspection
 * A template class can now inherit from QObject with no restrictions on types
 * Complex data types such as **QMap&lt;QString, int&gt;** can be used without restriction

Enhanced Functionality
 * CopperSpice makes extensive use of modern C++ features
   * constexpr, variadic templates, SFINAE, lambda expressions, tuple, move semantics, and type traits
 * Redesigned all container classes to use the C++ standard library containers, iterators, and algorithms
 * New platform independent plugin system based on C++
 * High DPI Support
 * CopperSpice includes a large number of classes from Qt5
 * Integration of CsSignal
   * Improved thread aware Signal/Slot delivery
   * Increased efficiency while maintaining the full Signal/Slot API
 * Integration of CsString
   * Improved storage of Unicode strings
   * QString8 (UTF-8) and QString16 (UTF-16)
   * New classes for QStringView and QStringParser

Using the Libraries
 * Any application using CopperSpice can be built with CMake, GNU Autotools, or any other build system
 * CopperSpice can be linked directly into any standard C++ application


### System Requirements

To use the CopperSpice libraries you will need a C++14 compiler and a C++14 standard library.

Uses CMake or Autotools for building binary files. Your project can be built with either CMake
or Autotools. Refer to our CopperSpice Overview documentation, DoxyPress application, or our
KitchenSink demo for sample build files.


### Building

The CopperSpice libraries can be built using CMake or GNU Autotools.


### Documentation

###### Overview

The CopperSpice Overview documentation includes information on building CopperSpice, downloading prebuilt binary
files, package requirements, setting up an an application which links with CopperSpice, migrating to CopperSpice, and
other general information.

www.copperspice.com/docs/cs_overview/index.html


###### API

The API contains full class documentation and multiple tutorials for CopperSpice. It is available on the website or
from our download page.


|URL      |Description|
|---------|-----------|
|www.copperspice.com/docs/cs_api_1.5/index.html|CopperSpice 1.5  (stable)|
|www.copperspice.com/docs/cs_api_1.6/index.html|CopperSpice 1.6|
|         |           |
|https://download.copperspice.com/copperspice/documentation|Overview and API (tar and zip formats)|



### Presentations

Our YouTube channel contains videos about modern C++, graphics, build systems, CopperSpice, DoxyPress, and other
topics related to software development.

https://www.youtube.com/copperspice


Links to technical presentations recorded at CppNow and CppCon:

www.copperspice.com/presentations.html


### Authors / Contributors

* **Ansel Sermersheim**
* **Barbara Geller**
* **Tim van Deurzen**
* **Jan Wilmans**
* **Peter Bindels**
* **Mortaro Marcello**
* **Adam Mensel**
* **Robin Mills**
* **Ivailo Monev**
* **Adam Mensel**
* **Matan Nassaw**
* **Daniel Pfeifer**
* **Zbigniew Skowron**



### License

This library is released under the LGPL V2.1 license. For more information refer to the LICENSE files provided with
this project.


### References

* Website: www.copperspice.com
* Email:   info@copperspice.com
