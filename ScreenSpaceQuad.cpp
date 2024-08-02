#include "ScreenSpaceQuad.h"

const unsigned int STRIDE = 7 * sizeof(GL_FLOAT);

ScreenSpaceQuad::ScreenSpaceQuad()
{
    const unsigned int ARRSIZE = 28;
    const unsigned int SIZE = ARRSIZE*sizeof(GLfloat);
    const GLfloat data[ARRSIZE] =
    {
        //lower left
        0.0f, 0.0f,	  //tex coords
        0.0f,0.0f,1.0f,	//normal
        -1.0f, -1.0f, //verts

        //lower right
        1.0f, 0.0f,//tex coord
        0.0f,0.0f,1.0f,	//normal
        1.0f, -1.0f,//vert

        //upper right
        1.0f, 1.0f,//tex coord
        0.0f,0.0f,1.0f,	//normal
        1.0f, 1.0f,//vert

        //upper left
        0.0f, 1.0f,//tex coord
        0.0f,0.0f,1.0f,//normal
        -1.0f, 1.0f//vert
    };

    glGenBuffers(1, &quadVBO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

    glBufferData(GL_ARRAY_BUFFER, SIZE, data, GL_STATIC_DRAW);
    glTexCoordPointer(2, GL_FLOAT, STRIDE,0);
    glNormalPointer  (   GL_FLOAT, STRIDE,(char*)(2*sizeof(GL_FLOAT)));
    glVertexPointer  (2, GL_FLOAT, STRIDE,(char*)(5*sizeof(GL_FLOAT)));

    //unbind the buffer object
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}



void ScreenSpaceQuad::draw() const
{
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

    glTexCoordPointer(2, GL_FLOAT, STRIDE,0);
    glNormalPointer  (   GL_FLOAT, STRIDE,(char*)(2*sizeof(GL_FLOAT)));
    glVertexPointer  (2, GL_FLOAT, STRIDE,(char*)(5*sizeof(GL_FLOAT)));

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glDrawArrays(GL_QUADS, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);//unbind the vbo when we're done rendering
}
