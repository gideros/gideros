/* work in progress */


# Path Rendering
This is a high quality vector graphics rendering library for OpenGL and OpenGL ES. It's mostly based on 'GPU Accelerated Path Rendering' paper (stencil and cover method) from NVIDIA.

[![Video](http://img.youtube.com/vi/Ysg2L2CR4AM/0.jpg)](http://www.youtube.com/watch?v=Ysg2L2CR4AM)

# Usage

The API is based on [NV_path_rendering extension](https://www.opengl.org/registry/specs/NV/path_rendering.txt).

# API Reference

See the header file pr.h for API reference.

# Status

### Finished

* All geometric primitives (line, quadratic bezier, cubic bezier, arc)
* Filling and Stroking
* End cap (flat, square, round, triangulate)
* Join style (miter, bevel, round)
* Attribute interpolation
* SVG path description syntax
* Glyphs paths using Freetype 2

### In progress

* Dashing

### License 

MIT License
