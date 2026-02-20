# Provided project files
Contained files are extracted from a real-world project, though with some
alterations to reduce dependencies.
It uses two 3-party libraries: fmt, and Apache Xerces, both downloaded, and built
by the build system (CMake).
Files in the `tabfiles` directory are the ones to be parsed. Their content
cannot be changed.

# The task
Using modern C++ facilities and techniques (standard library, ranges, concepts, etc)
(in no particular order) :

* refactor the code to achieve better safety & readability
  while doing so, ensure the behaviour of the code does not change by writing unit tests
* imagine we have many more readers; try to come up with some generic template
  that could replace them, or at least vastly reduce the amount of code
  that needs to be written for each of them
* Document your steps by committing "at the right time" with meaningful commit messages.
* The end result should be a git repository.
* The HEAD of the main branch must be buildable with CMake and GCC 12 or 13.

You may use 3-party libraries (e.g. for writing tests),
provided they can be used with CMake's FetchContent.

# Some hints:
* The task is pretty open-ended. The goal is not to transform the code into
  a work of art, but to learn more about your ability to adapt and make meaningful
  changes in a pre-existing code base.
* The Xerces API requires initialization by calling `xercesc::XMLPlatformUtils::Initialize();`