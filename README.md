## CopperSpice

### Introduction

CopperSpice consists of a set of libraries used to develop cross-platform software applications. It is an
open source project released under the LGPL V2.1 license.

###### Major enhancements

Meta-Object Compiler (moc)
 * The Meta-Object Compiler is no longer required for generating meta data
 * Since moc is no longer required it was removed
 * The functionality provided by moc was replaced with compile time templates
 * CopperSpice automatically generates meta data for processing Signals/ Slots and Introspection
 * A template class can now inherit from QObject with no restrictions on types
 * Complex data types can be used without restrictions
   * For example the following is valid in CopperSpice: QMap&lt;QString, int&gt;

New Functionality
 * CopperSpice includes several of the Qt 5 classes
 * CopperSpice makes extensive use of modern C++ features like variadic templates, constexpr,
   SFINAE, lambdas, atomics, tuple, etc
 * Reimplement container classes in CsCore leveraging the C++ standard library containers, iterators, and algorithms
 * Integration of CsSignal
   * Improved thread aware Signal/Slot delivery
   * Increased efficiency while maintaining the full Signal/Slot API
 * Integration of CsString
   * Improved storage of Unicode strings
   * In the process of adding QString8 (UTF-8) and QString16 (UTF-16)

Building
 * The CopperSpice libraries can be built using CMake or GNU Autotools
 * Any application using CopperSpice can be built with CMake, GNU Autotools, or any other build system
 * CopperSpice can be linked directly into any standard C++ application


CopperSpice is a C++ library derived from the Qt framework. Our motivation for developing CopperSpice
was to change the core design of the libraries leveraging modern C++ functionality.

CopperSpice consists of the following libraries:

<div style="margin-top:-1em;margin-left:3em">
      CsCore, CsGui, CsMultimedia, CsNetwork, CsOpenGL, CsPhonon,
      CsScript, CsSql, CsSvg, CsWebKit, CsXml, and CsXmlPatterns
</div>



### System Requirements

To use the CopperSpice libraries you will need a C++11 compiler and a C++11 standard library.

Uses CMake or Autotools for building binary files. Your project can be built with either CMake
or Autotools. Refer to our CopperSpice Overview documentation, DoxyPress application, or our
KitchenSink demo for sample build files.


### Documentation

Full class documentation for CopperSpice is available on the CopperSpice website:

www.copperspice.com/docs/cs_api/index.html


### Overview and Building

The CopperSpice Overview documentation includes information on how to build CopperSpice,
how to set up a CopperSpice project, and how to migrate from Qt to CopperSpice.

www.copperspice.com/docs/cs_overview/index.html


### Presentations

YouTube channel videos about CopperSpice, DoxyPress, C++, and the other topics related to our work.

https://www.youtube.com/channel/UC-lNlWEq0kpMcThO-I81ZdQ


Links to technical presentations recorded at CppNow and CppCon:

www.copperspice.com/presentations.html


### Authors

* **Ansel Sermersheim**
* **Barbara Geller**
* **Robin Mills**
* **Ivailo Monev**
* **Charley Bey**
* **Adam Mensel**
* **Daniel Pfeifer**


### License

This library is released under the LGPL V2.1 license. For more information refer to the
LICENSE files provided with this project.


### References

* Website: www.copperspice.com
* Email:   info@copperspice.com
