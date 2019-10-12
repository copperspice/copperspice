## CopperSpice

### Introduction

CopperSpice is a set of individual libraries which can be used to develop cross platform software applications in C++.
It is a totally open source project released under the LGPL V2.1 license and was initially derived from the Qt
framework. Over the last several years CopperSpice has completely diverged, with a goal of providing a first class GUI
library to unite the C++ community.

Our motivation for developing CopperSpice was to change the fundamental design and turn the existing framework into a
set of libraries for C++ developers. We are accomplishing this by leveraging modern C++ functionality, new technology,
and modern tooling. CopperSpice currently requires C++17 or newer.

The libraries avaiable in CopperSpice include:

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

There are also open source BSD licensed libraries which are used by CopperSpice and available as stand alone libraries
for anyone developing a C++ application.

 * CsSignal
 * CsString
 * libGuarded


### System Requirements

To use the CopperSpice libraries a C++17 compiler and a C++17 standard library are required.

CopperSpice CMake build files are provided with the source distribution. We recommend your project should also use
CMake for the build system.

For additional information about building from source, refer to our CopperSpice Overview Documentation or our
KitchenSink demo application for sample CMake project files.


### Building

The CopperSpice libraries are built using the CMake build system.


### Documentation

###### Overview

The CopperSpice Overview documentation includes information on building CopperSpice, downloading prebuilt binary
files, package requirements, setting up an application which links with CopperSpice, migrating to CopperSpice, and
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


###### Major Enhancements

Reflection
 * No Meta-Object Compiler is required for generating meta data, all references were removed
 * The functionality provided by moc was replaced with compile time templates
 * CopperSpice automatically generates meta data for processing Signals/ Slots and Introspection
 * A template class can now inherit from QObject with no restrictions on types
 * Complex data types such as **QMap&lt;QString, int&gt;** can be used for signal or slot arguments

Enhanced Functionality
 * CopperSpice makes extensive use of modern C++ features
   * constexpr, variadic templates, SFINAE, lambda expressions, tuple, move semantics, and type traits
 * Redesigned all container classes to use the C++ standard library containers, iterators, and algorithms
 * CopperSpice includes a majority of the Qt 5 classes
 * New platform independent plugin system based on C++
 * High DPI Support

 * Integration of CsSignal
   * Improved thread aware Signal/Slot delivery
   * Increased efficiency while maintaining the full Signal/Slot API
   * Deadlocks in Signal/Slot processing have been eliminated
 * Integration of CsString
   * Improved storage to properly represent Unicode strings
   * QString8 (UTF-8) and QString16 (UTF-16) classes
   * Added QStringView and QStringParser

Using the Libraries
 * Any application using CopperSpice can be built with CMake or any build system which imports CMake files
 * CopperSpice can be linked directly into any standard C++ application


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
