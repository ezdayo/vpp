# TODO

- [ ] Generate a top-level configuration-specific vpp.hpp file;
- [ ] Create a full comprehensive vpp.pc file generator including Requires,
  Requires.Private, CFlags, Libs and Libs.private;
- [ ] Add the vpp.pc file to the list of installed files;
- [X] Add specific OCV/Font and VPP/Font classes for managing the
  presence or absence of OpenCV\_freetype library and allowing
  runtime font update;
- [X] Make the CMakeList modular so that it manages configurations
  where some of the input libraries (except mandatory ones) are
  missing;
- [ ] Enable the generation of a static VPP library;
- [ ] Enable support for code quality and documentation generation
  tools;
- [ ] Add a specific stage for object tracking (with Kalman filter);
- [ ] Create a string-indexed map to store filters;
- [ ] Add capabilities to define composed filters;
- [ ] Enable Android, iOS, MacOS and MS Windows versions;
- [ ] Improve the code documentation.

# OPEN POINTS

- [ ] Add a specific stage for visualisation, or let the visualisation
  be an observer in the application?
