#ifndef OPENGLQUADRIC_H
#define OPENGLQUADRIC_H

#include <GL/glew.h>
#include <string>

class OpenGLQuadricObject
{
private:

     GLUquadricObj *quadricObject;

     OpenGLQuadricObject& operator=(const OpenGLQuadricObject&){throw new std::string("Error, calling invalid operator on quadric");}
     OpenGLQuadricObject(const OpenGLQuadricObject&){throw std::string("Error, calling invalid operator on quadric");}

    OpenGLQuadricObject():quadricObject(gluNewQuadric())
    {
        gluQuadricDrawStyle(quadricObject, GLU_FILL);
        gluQuadricNormals(quadricObject, GLU_SMOOTH);
        gluQuadricOrientation(quadricObject, GLU_OUTSIDE);//dsfsdfdf
        gluQuadricTexture(quadricObject, GL_TRUE);
    }

public:

    static const OpenGLQuadricObject& getInstance()
    {
        static OpenGLQuadricObject instance;
        return instance;
    }

    GLUquadricObj* getObject() const{return quadricObject;}

    ~OpenGLQuadricObject()
    {
        gluDeleteQuadric(quadricObject);
    }
};

#endif
