#version 330 core
 
in vec3 vertex_position;
 
uniform mat4 mvp;
uniform mat4 model_matrix; // Add model matrix for proper transformations
 
// Pass by variables
in vec3 color_input;
out vec3 fragment_color;
in vec3 vertex_normal_in;
out vec3 vertex_normal;
out vec3 fragment_position;

// Texture coordinates
in vec2 texture_coord_in;
out vec2 texture_coord;

//in float ambient_value;
//out float ambient_out;
 
void main(){    
    gl_Position = mvp * vec4(vertex_position, 1);
    
    // Transform position to world space for lighting calculations
    fragment_position = vec3(model_matrix * vec4(vertex_position, 1.0));
    
    // Transform normal to world space - note: should use normal matrix for non-uniform scaling
    vertex_normal = mat3(transpose(inverse(model_matrix))) * vertex_normal_in;
    
    fragment_color = color_input;
    texture_coord = texture_coord_in;

    //ambient_out= ambient_value;
}