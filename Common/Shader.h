#pragma once
#include <GL/glew.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <memory>

namespace Shader
{
    inline GLint CompileShaders( const std::vector<std::string>& shader_source, const std::vector<GLuint>& shader )
    {
        /* Function to compiler shaders given shader file path */

        // index n of shader_source should correspond to index i of shader
        GLint result = GL_FALSE;
        int info_log;

        for(uint i = 0; i < shader_source.size(); i++)
        {
            char const* shader_source_ptr = shader_source[i].c_str();
            glShaderSource( shader[i], 1, &shader_source_ptr, NULL );
            glCompileShader( shader[i] );

            // next section can be removed after debugging has finished
            glGetShaderiv(shader[i], GL_COMPILE_STATUS, &result);
            glGetShaderiv(shader[i], GL_INFO_LOG_LENGTH, &info_log);

            if( info_log > 0 )
            {
                std::vector<char> shader_err_msg(info_log+1);
                glGetShaderInfoLog(shader[i], info_log, NULL, &shader_err_msg[0]);
                std::cerr << &shader_err_msg[0];
                exit(EXIT_FAILURE);
            }
        }
        return result;
    }

    inline GLuint LoadShaders( const std::vector<std::string>& file_paths )
    {
        /* Load multiple shaders into OpenGL given their file paths */
        std::vector<GLuint> shaders;
        std::vector<std::string> shader_source;

        for( uint i = 0; i < file_paths.size(); i++ )
        {
            // shaders currently MUST open else the program fails
            std::ifstream shader_stream( file_paths[i] );
            if( !shader_stream.is_open() )
            {
                std::cerr << "Could not open " << file_paths[i] << std::endl;
                exit(EXIT_FAILURE);
            }

            std::stringstream stream;
            stream << shader_stream.rdbuf();
            shader_source.push_back(stream.str());
            std::string file_extension = file_paths[i].substr(file_paths[i].find("."));
            
            // must be either vertex or fragment shader
            if( file_extension == ".vert" )
            {
                shaders.push_back( glCreateShader(GL_VERTEX_SHADER) );
            }
            else if( file_extension == ".frag" )
            {
                shaders.push_back( glCreateShader(GL_FRAGMENT_SHADER) );
            }
            else
            {
                std::cerr << "Shader file type not recognised - exiting program";
                exit(EXIT_FAILURE);
            }
            shader_stream.close();
        }
        // create shader program and check if its successful.
        GLint result = CompileShaders( shader_source, shaders );
        GLuint program = glCreateProgram();
        for(uint i = 0; i < shaders.size(); i++)
        {
            glAttachShader(program, shaders[i]);
        }
        glLinkProgram(program);

        int info_log;
        glGetProgramiv(program, GL_LINK_STATUS, &result);
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log);

        if ( info_log > 0 )
        {
            std::vector<char> program_err_msg(info_log+1);
            glGetProgramInfoLog(program, info_log, NULL, &program_err_msg[0]);
            std::cerr << &program_err_msg[0];
            exit(EXIT_FAILURE);
        }
        
        for(uint i = 0; i < shaders.size(); i++)
        {
            glDetachShader(program, shaders[i]);
            glDeleteShader( shaders[i] );
        }

        return program;
    }
}