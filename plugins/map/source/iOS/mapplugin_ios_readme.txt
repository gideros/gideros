Map Plugin

Installation on iOS:
Export your Gideros project for iOS.
In Finder, copy the mapplugin folder into Plugins folder.
Drag mapplugin folder from Finder into Xcode Plugins folder to add those files to the project.

Notes:

The AnnotatedMap class implements all the functionality for the Lua map plugin for iOS, extending the native MKMapView class.  It uses the CustomAnnotation class to implement markers on the map.

Note that the mapPressed method is actually detecting a long press of at least 1 second.  The anticipated use of this method is to allow the user to pick out a point on the map, so the on-screen user instructions should ask for a long press in that situtation.  If a 1 second press is not suitable, one could change the implementation of that method.  Search for "longpress" in AnnotatedMap.m for more information.

Note that marker20x30.png can be replaced with any custom marker, but it will be rendered as a solid color tinted as specified in the setMarkerHue() method.  As this marker points to a location at the bottom of the image in the horizontal center, a line of code in AnnotatedMap.m specifies the offset.  By default markers are centered on their specified locations on the map.  This line of code offsets the markers so they are drawn 15 pixels higher, so the tip of the marker in the bottom center correctly aligns with the map coordinate:
[annotationView setCenterOffset: CGPointMake(0,-15)];
If you use a different marker image, adjust the line of code accordingly.

