#pragma once
/**
 *  Copyright (C) 2016 Rafal Janicki
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty.  In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 *
 *  Rafal Janicki
 *  ravarcade@gmail.com
 */


/**
 * Resources:
 * http://www.orthostereo.com/geometryopengl.html 
 * http://stackoverflow.com/questions/16071984/opengl-asymmetric-frustum-for-desktop-vr
 * http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/perspective-projections-in-lh-and-rh-systems-r3598
 * DX Stuffs: Microsoft
 * https://msdn.microsoft.com/en-us/library/windows/desktop/bb205353(v=vs.85).aspx
 * https://msdn.microsoft.com/en-us/library/windows/desktop/bb205354(v=vs.85).aspx
 *
 * TPA uses Right Hand. If you need Left Hand projection matrix just replace final matrix build.
 *
 * Params:
 * l - minimum X 
 * r - maximum X
 * b - minimum Y
 * t - maximum Y
 * zn - minimum Z
 * zf - maximum Z
 * 
 * DX:RH:
 * 2*zn/(r-l)   0            0                0
 * 0            2*zn/(t-b)   0                0
 * (l+r)/(r-l)  (t+b)/(t-b)  zf/(zn-zf)      -1
 * 0            0            zn*zf/(zn-zf)    0
 */

/******************************************************************************
 *
 * Some description what is used and why so many params.
 * -----------------------------------------------------
 *
 * DisplaySize, DisplayNativeResolutionWidth, DizplayNativeResolutionHeight:
 * That 3 params are used to calc physical dimensions of screen:
 * Width & Height in millimeters.
 * Please note, that Width & Height will also define screen aspect.
 * All is done with assumptiona, that screen pixels are square.
 *
 * DisplayResolutionWidth, DisplayResolutionHeight:
 * May not be NATIVE screen res. May have different aspect. 
 * In that case pixel will not be square but rectangle!
 *
 * WindowPositionX, WindowPositionY, WindowWidth, WindowHeight:
 * Whole thing may work in window.
 * All that 3 blocks of params allow me to know, 
 * where on physical screen is every pixel drawed in app.
 * 
 * ViewerPositionX, ViewerPositionY, ViewerPositionZ:
 * Head tracking data (from BAM Tracker) or values entered by user. 
 * Units: millimeters.
 * Centere of coord system is screen center.
 * Right Hand coord system.
 *
 * Z_near, Z_far:
 * Almost what you expect. Almost.
 * 1. Units are millimeters.
 * 2. Distance is measured not from camera but from screen surface.
 *    Z_near = -100.0 (MINUS!) means: 100 millimeters above screen surface
 *    Z_far = 2000.0 means: 2 meters inside screen
 *
 * All params above are used to calc r,l,t,b,zn,zf. 
 * Params used to build assymetric frustum matrix.
 *
 * Rotation:
 * In FP user can rotate playfield without rotating screen in windows.
 * It is used same way.
 *
 * Last thing: 
 * Don't forget to put
 *    Translate(-ViewerPositionX, -ViewerPositionY, -ViewerPositionZ)
 * in View matrix!
 *
 * ----------------------------------------------------------------------------
 * Have a nice BAM.
 *****************************************************************************/

/// <summary>
/// Builds the Asymetrix Frustum Projection Matrix.
/// </summary>
/// <param name="ProjectionMatrix">The projection matrix (output).</param>
/// <param name="DisplaySize">The display size. [inches]</param>
/// <param name="DisplayNativeResolutionWidth">Display NATIVE width. [pixels]</param>
/// <param name="DisplayNativeResolutionHeight">Display NATIVE height. [pixels]</param>
/// <param name="DisplayResolutionWidth">Display width of the resolution. [pixels]</param>
/// <param name="DisplayResolutionHeight">Display height of the resolution. [pixels]</param>
/// <param name="WindowPositionX">The window position x. [pixels]</param>
/// <param name="WindowPositionY">The window position y. [pixels]</param>
/// <param name="WindowWidth">Width of the window. [pixels]</param>
/// <param name="WindowHeight">Height of the window. [pixels]</param>
/// <param name="ViewerPositionX">The viewer position x. [mm]</param>
/// <param name="ViewerPositionY">The viewer position y. [mm]</param>
/// <param name="ViewerPositionZ">The viewer position z. [mm]</param>
/// <param name="Z_near">The z_near. [mm] Suprise :)</param>
/// <param name="Z_far">The z_far. [mm] Suprise :) </param>
/// <param name="Rotation">The rotation of image. CCW. 0 - none, 1 - 90 deg, 3 - 180 deg, 4 - 270 deg.</param>
void BuildProjectionMatrix(
	float *ProjectionMatrix /* output */,
	float DisplaySize,
	float DisplayNativeResolutionWidth, float DisplayNativeResolutionHeight,
	float DisplayResolutionWidth, float DisplayResolutionHeight,
	float WindowPositionX, float WindowPositionY,
	float WindowWidth, float WindowHeight,
	float ViewerPositionX, float ViewerPositionY, float ViewerPositionZ,
	float Z_near, float Z_far,
	int Rotation
	)
{
	// Calc sin and cos for rotation (well, read it from table)
	float _sin, _cos;
	static float _sin_cos[4][2] = { { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 0 } };
	Rotation = Rotation < 0 || Rotation > 3 ? 0 : Rotation;
	_sin = _sin_cos[Rotation][0];
	_cos = _sin_cos[Rotation][1];

	// Apply rotation to ViewerPosition
	float tmpPosX = ViewerPositionX;
	float tmpPosY = ViewerPositionY;
	ViewerPositionX = tmpPosX*_cos - tmpPosY*_sin;
	ViewerPositionY = tmpPosX*_sin + tmpPosY*_cos;

	// Calc pixels to millimeters scale. Different for X & Y is possible if res with different aspect ratio than native res is used.
	float pixelsToMillimeters = (float)(25.4*DisplaySize / sqrt(DisplayNativeResolutionWidth*DisplayNativeResolutionWidth + DisplayNativeResolutionHeight*DisplayNativeResolutionHeight));
	float pixelsToMillimetersX = pixelsToMillimeters * DisplayNativeResolutionWidth / DisplayResolutionWidth;
	float pixelsToMillimetersY = pixelsToMillimeters * DisplayNativeResolutionHeight / DisplayResolutionHeight;

	// Windows edeges position in relation to screen center (in pixels)
	float l = WindowPositionX - DisplayResolutionWidth * 0.5f;
	float t = -(WindowPositionY - DisplayResolutionHeight * 0.5f);
	float r = l + WindowWidth;
	float b = t - WindowHeight;

	// Calc position of screen in relation to viewer
	l = pixelsToMillimetersX * l - ViewerPositionX;
	r = pixelsToMillimetersX * r - ViewerPositionX;
	t = pixelsToMillimetersY * t - ViewerPositionY;
	b = pixelsToMillimetersY * b - ViewerPositionY;


	// Viewer can't be less than 1 mm away from screen
	if (ViewerPositionZ < 1.0)
		ViewerPositionZ = 1.0;

	// Calc z near and z far
	float zn, zf;
	zn = ViewerPositionZ + Z_near;
	zf = ViewerPositionZ + Z_far;

	// zn can't be too close to viewer
	if (zn < 0.5) {
		zn = 0.5f;
	}

	// zs is screen position.
	float zs = ViewerPositionZ = ViewerPositionZ >= zn ? ViewerPositionZ : zn;

	// Move edges of frustrum from zs to zn
	float z_near_to_z_screen = zn / zs; // <=1.0
	r *= z_near_to_z_screen;
	l *= z_near_to_z_screen;
	t *= z_near_to_z_screen;
	b *= z_near_to_z_screen;

	//-------------------------------------------------------------------------
	// FINALLY. We have all params needed to build projection matrix.
	// If you need projection matrix for Left Hand or for OpenGL, replace this part.
	float P[16] = {
		2*zn/(r-l),   0,            0,                0,
		0,            2*zn/(t-b),   0,                0,
		(l+r)/(r-l),  (t+b)/(t-b),  zf/(zn-zf),       -1,
		0,            0,            zn*zf/(zn-zf),    0
	};
	//-------------------------------------------------------------------------

	// Copy projection matrix to output.
	memcpy_s(ProjectionMatrix, sizeof(float[16]), P, sizeof(P));

	// Last fix. Apply rotation.
	ProjectionMatrix[0] = P[0] * _cos + P[4] * _sin;
	ProjectionMatrix[1] = P[1] * _cos + P[5] * _sin;
	ProjectionMatrix[4] = P[0] * -_sin + P[4] * _cos;
	ProjectionMatrix[5] = P[1] * -_sin + P[5] * _cos;
}
