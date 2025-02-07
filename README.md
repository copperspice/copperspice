## CopperSpice

### Introduction

CopperSpice is a set of individual libraries which can be used to develop cross platform software applications in C++.
It is a true open source project released under the LGPL V2.1 license and was initially derived from the Qt framework.

CopperSpice has diverged to become a modern representation of what can be accomplished with C++. Our goal and intention
is to provide a first class GUI library to unite the C++ community.

Our motivation for developing CopperSpice was to change the fundamental design and turn the existing framework into a
set of libraries for C++ developers. We are accomplishing this by leveraging modern C++ functionality, new technology,
and modern tooling.

The libraries available in CopperSpice include:

 * CsCore
 * CsGui
 * CsMultimedia
 * CsNetwork
 * CsOpenGL
 * CsSql
 * CsSvg
 * CsVulkan
 * CsWebKit
 * CsXml
 * CsXmlPatterns

There are also several open source BSD licensed libraries which are used by CopperSpice and available as stand
alone libraries for anyone developing C++ applications.

 * CsCrypto
 * CsLibGuarded
 * CsPaint
 * CsPointer
 * CsSignal
 * CsString


### System Requirements

Building CopperSpice requires a C++20 compiler and a C++20 standard library.

CMake build files are provided with the source distribution to build this library. The unit test binary executable is
an optional part of the build process.

This library has been tested with clang sanitizer and an extensive industry code review.


### Using the CopperSpice Libraries

 * Any C++ application using CopperSpice can be built with CMake or any build system which imports CMake files

 * We recommend your software application use CMake and Ninja for the build system

 * CopperSpice can be linked directly into any standard C++ application

 * KitchenSink is a demo application which contains over 30 examples of the basic functionality available in CopperSpice


### Documentation

###### Overview

The CopperSpice Overview documentation includes information on building CopperSpice, downloading prebuilt binary
files, package requirements, setting up an application which links with CopperSpice, migrating to CopperSpice, and
general configuration information.

https://www.copperspice.com/docs/cs_overview/index.html


###### API

The API contains full class documentation and multiple tutorials for CopperSpice and is available directly on our
website and from our download page.

https://www.copperspice.com/docs/cs_api/index.html


###### Offline

Both the CS Overview and API documentation can be downloaded for offline use. They are available in a compressed tar file
or zip format.

https://download.copperspice.com/copperspice/documentation


### Major Enhancements

* Reflection (run time and compile time)
  * No Meta-Object Compiler is required for generating meta data, all references were removed
  * The functionality provided by moc was replaced with compile time templates
  * CopperSpice automatically generates meta data for processing Signals/ Slots and Introspection
  * Template classes can inherit from QObject with no restrictions on types
  * Complex data types such as **QMap&lt;QString, int&gt;** can be used for signal or slot arguments
<!-- -->
* Enhanced Functionality
  * CopperSpice makes extensive use of modern C++ features
    * constexpr, lambda expressions, templates, variadic templates, template variables
    * move semantics, structured bindings, tuple,  decltype, SFINAE, and type traits
  * Redesigned all container classes to use the C++ standard library containers, iterators, and algorithms
  * Refactored all Mutex and Lock classes
  * CopperSpice includes a majority of the Qt 5 classes
  * Platform independent plugin system based on standard C++
  * High DPI Rendering Support
  * Full support for the Vulkan Graphics API
<!-- -->
* Integration of CsLibGuarded
  * Used to manage shared data
  * Provides functionality to associate a mutex with the data it protects
<!-- -->
* Integration of CsSignal
  * Improved thread aware Signal/Slot delivery
  * Increased efficiency while maintaining the full Signal/Slot API
  * Deadlocks in Signal/Slot processing have been eliminated
<!-- -->
* Integration of CsString
  * Improved storage to properly represent Unicode strings
  * QString8 (UTF-8) and QString16 (UTF-16) classes
  * Added QStringView, QStringParser, and QRegularExpression
<!-- -->
* Integration of CsPointer
  * Leverages the C++ pointer classes, adds additional functionality


### Presentations

Our YouTube channel contains over 75 videos about C++, programming fundamentals, Unicode/Strings, multithreading,
graphics, CopperSpice, DoxyPress, and other software development topics.

https://www.youtube.com/copperspice

Links to additional videos can be found on our website.

https://www.copperspice.com/presentations.html


### Authors and Key Contributors

The CS team welcomes contributors of all skill levels. When submitting a pull request please observe our
Coding Style Guidelines.

https://www.copperspice.com/style_guide/source_code_style.html


* **Ansel Sermersheim**
* **Barbara Geller**
* **Jan Wilmans**
* **Tim van Deurzen**
* **Paul Bendixen**
* **Peter Bindels**
* **Mortaro Marcello**
* **Adam Mensel**
* **Robin Mills**
* **Ivailo Monev**
* **Adam Mensel**
* **Matan Nassaw**
* **Jeff Cohen**
* **Daniel Pfeifer**
* **Zbigniew Skowron**
* **Johan Förberg**
* **Dennis Menschel**


### License

This library is released under the LGPL V2.1 license. For more information refer to the LICENSE file provided with
this project.


### References

 * Website:  https://www.copperspice.com
 * Twitter:  https://twitter.com/copperspice_cpp
 * Email:    info@copperspice.com

<!-- -->
 * Github:   https://github.com/copperspice

<!-- -->
 * Forum:    https://forum.copperspice.com
 * Journal:  https://journal.copperspice.com
