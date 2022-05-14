ofxBezierWarp
=============

Performs a warp using openGL bezier surfaces on anything you care to distort/undistort.

Syntax is much like an FBO, with an allocate, begin, end, draw style interface.

It's pretty fast.

Code was adapted from the method described here: http://forum.openframeworks.cc/index.php/topic,4002.0.html

If you're using this software for something cool consider sending me an email to let me know about your project: m@gingold.com.au


##Fork by dasoe:

Added new (kind of hidden) functionality:
When hitting SHIFT while grabbing a corner point of the warp grid all points turn red instead of green. When realeasing the mouse while they are red, all points are rearranged equally - subject to the position of the 4 corners. This is especially useful for (perspective) warping before doing any fine-tuning.