#include <stdio.h>
#include <stdlib.h>
#include <iostream>

std::string VertexShaderCode = R"(\
    #version 400

    // Input vertex data, different for all executions of this shader.
    layout(location = 0) in vec3 pos_in;
    layout(location = 1) in vec3 normal_in;

    // Output data : will be passed to TCS.
    out vec3 pos_worldspace;
    out vec3 normal_worldspace;
    out vec3 lightDir;
    out vec3 eyeDir;
    out vec2 uv;

    // Uniform values that stay constant for the whole mesh.
    uniform float time;
    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;
    uniform vec3 lightPos;
    uniform vec2 texOffset = vec2(1.0, 2.0);
    uniform float texScale = 6.0;


    void main(){
        pos_worldspace = pos_in;
        normal_worldspace = ( V * M * vec4(pos_in,0)).xyz;

        eyeDir = vec3(0,0,0) - pos_worldspace;
        lightDir = lightPos-pos_worldspace;

        uv = (pos_worldspace.xz + texOffset + (time * 0.08)) / texScale;

        gl_Position = vec4(pos_worldspace, 1.0);
    }
)";

std::string TesselationControlShaderCode = R"(\
    #version 400

    layout (vertices = 4) out;

    // Input vertex data, aggregated into patches
    in vec3 pos_worldspace[];
    in vec3 normal_worldspace[];
    in vec3 lightDir[];
    in vec3 eyeDir[];
    in vec2 uv[];

    // Output data : will be passed to TES.
    out vec3 position_tcs[];
    out vec3 normal_tcs[];
    out vec3 light_tcs[];
    out vec3 eye_tcs[];
    out vec2 uv_tcs[];

    // Uniform values that stay constant for the whole mesh.
    uniform float outerTess = 16.0;
    uniform float innerTess = 16.0;

    void main() {
        //use gl_InvocationID
        gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
        position_tcs[gl_InvocationID] = pos_worldspace[gl_InvocationID];
        normal_tcs[gl_InvocationID] = normal_worldspace[gl_InvocationID];
        light_tcs[gl_InvocationID] = lightDir[gl_InvocationID];
        eye_tcs[gl_InvocationID] = eyeDir[gl_InvocationID];
        uv_tcs[gl_InvocationID] = uv[gl_InvocationID];


        gl_TessLevelOuter[0] = outerTess;
        gl_TessLevelOuter[1] = outerTess;
        gl_TessLevelOuter[2] = outerTess;
        gl_TessLevelOuter[3] = outerTess;
        gl_TessLevelInner[0] = innerTess;
        gl_TessLevelInner[1] = innerTess;

    }
)";

std::string TesselationEvaluationShaderCode = R"(\
    #version 400

    layout (quads, equal_spacing) in;

    uniform mat4 MVP;

    in vec3 position_tcs[];
    in vec3 normal_tcs[];
    in vec3 light_tcs[];
    in vec3 eye_tcs[];
    in vec2 uv_tcs[];

    out vec3 position_tes;
    out vec3 normal_tes;
    out vec3 light_tes;
    out vec3 eye_tes;
    out vec2 uv_tes;

    void main() {

        vec4 p0= gl_in[0].gl_Position;
        vec4 p1= gl_in[1].gl_Position;
        vec4 p2= gl_in[2].gl_Position;
        vec4 p3= gl_in[3].gl_Position;

        vec4 m1 = mix(p0, p1, gl_TessCoord.x);//may have to rearrange these numbers depending on your implementation
        vec4 m2 = mix(p3, p2, gl_TessCoord.x);//may have to rearrange these numbers depending on your implementation
        gl_Position = mix(m1, m2, gl_TessCoord.y);

        vec3 c1 = mix(normal_tcs[0], normal_tcs[1], gl_TessCoord.x);//may have to rearrange these numbers depending on your implementation
        vec3 c2 = mix(normal_tcs[3], normal_tcs[2], gl_TessCoord.x);//may have to rearrange these numbers depending on your implementation
        normal_tes = mix(c1, c2, gl_TessCoord.y);

        c1 = mix(position_tcs[0], position_tcs[1], gl_TessCoord.x);//may have to rearrange these numbers depending on your implementation
        c2 = mix(position_tcs[3], position_tcs[2], gl_TessCoord.x);//may have to rearrange these numbers depending on your implementation
        position_tes = mix(c1, c2, gl_TessCoord.y);



        c1 = mix(eye_tcs[0], eye_tcs[1], gl_TessCoord.x);//may have to rearrange these numbers depending on your implementation
        c2 = mix(eye_tcs[3], eye_tcs[2], gl_TessCoord.x);//may have to rearrange these numbers depending on your implementation
        eye_tes = mix(c1, c2, gl_TessCoord.y);

        c1 = mix(light_tcs[0], light_tcs[1], gl_TessCoord.x);//may have to rearrange these numbers depending on your implementation
        c2 = mix(light_tcs[3], light_tcs[2], gl_TessCoord.x);//may have to rearrange these numbers depending on your implementation
        light_tes = mix(c1, c2, gl_TessCoord.y);

        vec2 u1 = mix(uv_tcs[0], uv_tcs[1], gl_TessCoord.x);//may have to rearrange these numbers depending on your implementation
        vec2 u2 = mix(uv_tcs[3], uv_tcs[2], gl_TessCoord.x);//may have to rearrange these numbers depending on your implementation
        uv_tes = mix(u1, u2, gl_TessCoord.y);
    }
)";

std::string GeometryShaderCode = R"(\
    #version 400

    layout (triangles) in;
    layout (triangle_strip, max_vertices=3) out;

    // Input vertex data, aggregated into triangles
    in vec3 position_tes[];
    in vec3 normal_tes[];
    in vec3 light_tes[];
    in vec3 eye_tes[];
    in vec2 uv_tes[];

    // Output data per vertex, passed to primitive assembly and rasterization
    out vec3 pos_geo;
    out vec3 normal_geo;
    out vec3 lightDir_geo;
    out vec3 eyeDir_geo;
    out vec2 uv_geo;

    // Uniform values that stay constant for the whole mesh.
    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;

    uniform sampler2D displacementTexture;
    uniform float time;

    vec3 GetNormal(vec3 a, vec3 b, vec3 c)
    {
        vec3 x = b - a;
        vec3 y = c - b;
        return normalize(cross(x, y));
    }


    vec3 Gerstner(vec3 worldpos, float w, float A, float phi, float Q, vec2 D, int N) {
        float cos1 = cos(w * dot(D, worldpos.xz) + phi * time);
        float sin1 = sin(w * dot(D, worldpos.xz) + phi * time);
        float Qi = Q / (w * A * float(N));

        return vec3(Qi * A * D.x * cos1, A * sin1, Qi * A * D.y * cos1);
    }


    void main() {
        vec4 pos[gl_in.length()];
        for(int i = 0; i < gl_in.length(); ++i) {
            pos[i] = vec4(position_tes[i], 1.0);

            pos[i] += vec4(Gerstner(position_tes[i], 4, 0.08, 1.1, 0.75, vec2(0.3, 0.6), 4), 0.0);
            pos[i] += vec4(Gerstner(position_tes[i], 2, 0.05, 1.1, 0.75, vec2(0.2, .866), 4), 0.0);
            pos[i] += vec4(Gerstner(position_tes[i], 0.6, 0.2, 0.4, 0.1, vec2(0.3, 0.7), 4), 0.0);
            pos[i] += vec4(Gerstner(position_tes[i], 0.9, 0.15, 0.4, 0.1, vec2(0.8, 0.1), 4), 0.0);

            vec2 uv = uv_tes[i].xy * 0.03;
            float disp = texture(displacementTexture, uv).y;
            pos[i].y += disp;
        }

        vec3 mynorm = GetNormal(pos[0].xyz, pos[1].xyz, pos[2].xyz);

        for(int i = 0; i < gl_in.length(); ++i) {
            normal_geo = vec3(M * vec4(mynorm, 0)).xyz;
            lightDir_geo = light_tes[i];
            eyeDir_geo = eye_tes[i];
            uv_geo = uv_tes[i];
            gl_Position = P*V*M*pos[i];
            pos_geo = vec3(gl_Position);
            EmitVertex();
        }
        EndPrimitive();

    }
)";

std::string FragmentShaderCode = R"(\
    #version 400

    // Interpolated values from the vertex shaders
    in vec3 pos_geo;
    in vec3 normal_geo;
    in vec3 lightDir_geo;
    in vec3 eyeDir_geo;
    in vec2 uv_geo;

    // Ouput data
    out vec4 color_out;

    uniform sampler2D waterTexture;

    void phongColor() {
        // Light emission properties
        vec4 LightColor = vec4(1,1,1,1);

        // Material properties
        vec4 MaterialDiffuseColor = texture(waterTexture, uv_geo);
        vec4 MaterialAmbientColor = vec4(0.2,0.2,0.2,MaterialDiffuseColor.a) * MaterialDiffuseColor;
        vec4 MaterialSpecularColor = vec4(0.7, 0.7, 0.7,MaterialDiffuseColor.a);

        vec3 normNormal = normalize(normal_geo);
        vec3 lightNormal = normalize(lightDir_geo);
        vec3 eyeNormal = normalize(eyeDir_geo);

        float cosTheta = clamp(dot(normNormal, lightNormal), 0, 1);
        vec3 R = reflect(-lightNormal, normNormal);
        float cosAlpha = clamp(dot(eyeNormal, R), 0, 1);

        color_out =
            // Ambient : simulates indirect lighting
            MaterialAmbientColor +
            // Diffuse : "color" of the object
            MaterialDiffuseColor * LightColor * cosTheta +
            // Specular : reflective highlight, like a mirror
            MaterialSpecularColor * LightColor * pow(cosAlpha,8);
    }

    void main(){
        phongColor();
    }

)";