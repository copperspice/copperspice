## CopperSpice

### Introduction
CopperSpice is of a set of libraries used to develop cross-platform software applications in C++. This is an open source project
released under the LGPL V2.1 license.

###### Major enhancements

Reflection
 * No Meta-Object Compiler is required for generating meta data, all references were removed
 * The functionality provided by moc was replaced with compile time templates
 * CopperSpice automatically generates meta data for processing Signals/ Slots and Introspection
 * A template class can now inherit from QObject with no restrictions on types
 * Complex data types such as **QMap&lt;QString, int&gt;** can be used without restriction

Enhanced Functionality
 * CopperSpice makes extensive use of modern C++ features like variadic templates, constexpr, SFINAE, lambdas, atomics,
   tuple, etc
 * Reimplemented container classes in CsCore using the C++ standard library containers, iterators, and algorithms
 * CopperSpice includes several of the Qt 5 classes
 * Integration of CsSignal
   * Improved thread aware Signal/Slot delivery
   * Increased efficiency while maintaining the full Signal/Slot API
 * Integration of CsString
   * Improved storage of Unicode strings
   * QString8 (UTF-8) and QString16 (UTF-16)

Building
 * CopperSpice libraries can be built using CMake or GNU Autotools
 * Any application using CopperSpice can be built with CMake, GNU Autotools, or any other build system
 * CopperSpice can be linked directly into any standard C++ application


CopperSpice is a set of C++ libraries derived from the Qt framework. Our motivation for developing CopperSpice was to
change the core design and leverage modern C++ functionality.

CopperSpice consists of the following libraries:

<div style="margin-top:-1em;margin-left:3em">
      CsCore, CsGui, CsMultimedia, CsNetwork, CsOpenGL, CsPhonon,
      CsScript, CsSql, CsSvg, CsWebKit, CsXml, and CsXmlPatterns
</div>



### System Requirements

To use the CopperSpice libraries you will need a C++14 compiler and a C++14 standard library.

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

https://www.youtube.com/copperspice


Links to technical presentations recorded at CppNow and CppCon:

www.copperspice.com/presentations.html


### Authors / Contributors

* **Ansel Sermersheim**
* **Barbara Geller**
* **Robin Mills**
* **Ivailo Monev**
* **Adam Mensel**
* **Daniel Pfeifer**
* **Zbigniew Skowron**
* **Matan Nassaw**


### License

This library is released under the LGPL V2.1 license. For more information refer to the LICENSE files provided with
this project.


### References

* Website: www.copperspice.com
* Email:   info@copperspice.com
