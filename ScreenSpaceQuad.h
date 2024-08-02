#ifndef SCREENSPACEQUAD_H_INCLUDED
#define SCREENSPACEQUAD_H_INCLUDED


#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>


//todo: make renders take in openglrenderconfig (Steve version) to validate that gl calls are valid
//todo: bindable 3d model class
class ScreenSpaceQuad{

	private:

		GLuint quadVBO;
		ScreenSpaceQuad();
		void draw() const;

		ScreenSpaceQuad& operator=(const ScreenSpaceQuad&);
		ScreenSpaceQuad(const ScreenSpaceQuad&);

	public:
		static const ScreenSpaceQuad& getInstance();
		static void render();
		~ScreenSpaceQuad();
		const GLuint& getID()const;

};


#include "ScreenSpaceQuad.tcc"

#endif // SCREENSPACEQUAD_H_INCLUDED
