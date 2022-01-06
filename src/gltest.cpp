#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <GL/glu.h>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <string>

using namespace std;
using namespace boost::asio::ip;

sf::RenderWindow setupGraphics()
{
    return sf::RenderWindow(sf::VideoMode(640, 480), "OpenGL Test", sf::Style::Close | sf::Style::Titlebar);
}

int main()
{
    glewExperimental = GL_TRUE;
    glewInit();

    sf::RenderWindow window = setupGraphics();
    sf::Event event;

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    // glBindVertexArray(VertexArrayID);

    // // An array of 3 vectors which represents 3 vertices
    // static const GLfloat g_vertex_buffer_data[] = {
    //     -1.0f, -1.0f, 0.0f,
    //     1.0f, -1.0f, 0.0f,
    //     0.0f,  1.0f, 0.0f,
    // };

    // // This will identify our vertex buffer
    // GLuint vertexbuffer;
    // // Generate 1 buffer, put the resulting identifier in vertexbuffer
    // glGenBuffers(1, &vertexbuffer);
    // // The following commands will talk about our 'vertexbuffer' buffer
    // glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // // Give our vertices to OpenGL.
    // glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    while (window.isOpen())
    {
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            }
        }

        // display
        // 1st attribute buffer : vertices
        // glEnableVertexAttribArray(0);
        // glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        // glVertexAttribPointer(
        //     0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        //     3,                  // size
        //     GL_FLOAT,           // type
        //     GL_FALSE,           // normalized?
        //     0,                  // stride
        //     (void*)0            // array buffer offset
        // );
        // // Draw the triangle !
        // glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
        // glDisableVertexAttribArray(0);
    }

    return 0;
}