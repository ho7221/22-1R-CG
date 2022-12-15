#include "scene.h"
#include "binary/animation.h"
#include "binary/skeleton.h"
#include "binary/player.h"

Shader* Scene::vertexShader = nullptr;
Shader* Scene::fragmentShader = nullptr;
Program* Scene::program = nullptr;
Camera* Scene::camera = nullptr;
Object* Scene::player = nullptr;
Texture* Scene::diffuse = nullptr;
Material* Scene::material = nullptr;
Object* Scene::lineDraw = nullptr;
Texture* Scene::lineColor = nullptr;
Material* Scene::lineMaterial = nullptr;
int Scene::width=0;
int Scene::height=0;
float elapsedTime=0;
float P0x,P0y,P1x,P1y;
int motion=0;

void Scene::setup(AAssetManager* aAssetManager) {
    Asset::setManager(aAssetManager);

    Scene::vertexShader = new Shader(GL_VERTEX_SHADER, "vertex.glsl");
    Scene::fragmentShader = new Shader(GL_FRAGMENT_SHADER, "fragment.glsl");

    Scene::program = new Program(Scene::vertexShader, Scene::fragmentShader);

    Scene::camera = new Camera(Scene::program);
    Scene::camera->eye = vec3(0.0f, 0.0f, 80.0f); // 0,0,80

    Scene::diffuse = new Texture(Scene::program, 0, "textureDiff", playerTexels, playerSize);
    Scene::material = new Material(Scene::program, diffuse);
    Scene::player = new Object(program, material, playerVertices, playerIndices);
    player->worldMat = scale(vec3(1.0f / 3.0f));

    Scene::lineColor = new Texture(Scene::program, 0, "textureDiff", {{0xFF, 0x00, 0x00}}, 1);
    Scene::lineMaterial = new Material(Scene::program, lineColor);
    Scene::lineDraw = new Object(program, lineMaterial, {{}}, {{}}, GL_LINES);
}

void Scene::screen(int width, int height) {
    Scene::camera->aspect = (float) width/height;
    Scene::width = width;
    Scene::height = height;
}

vec3 getArcVec(float x, float y){ // calculate arcball vector
    int w = Scene::width;
    int h = Scene::height;
    x = x*2/float(w)-1;
    y = -(y*2/float(h)-1);

    if(x*x+y*y>1) return normalize(vec3(x,y,0));
    else return vec3(x,y,sqrt(1-(x*x+y*y)));
}

void Scene::update(float deltaTime) {
    Scene::program->use();

    Scene::camera->update();

    //////////////////////////////
    /* TODO */
    int idx;
    elapsedTime += deltaTime;
    if(elapsedTime>4) elapsedTime-=4;
    if(elapsedTime>=3) idx=3;
    else if(elapsedTime>=2) idx=2;
    else if(elapsedTime>=1) idx=1;
    else idx=0;
    float slerpweight = elapsedTime-idx;

    vector<mat4> Mp, Md, Ma;
    for(int i=0;i<28;i++){
        Mp.push_back((translate(jOffsets[i])));

        quat q0 = quat(cos(radians(motions[idx][6+(i-1)*3+2])/2),0,0,sin(radians(motions[idx][6+(i-1)*3+2])/2))
                  *quat(cos(radians(motions[idx][6+(i-1)*3+0])/2),sin(radians(motions[idx][6+(i-1)*3+0])/2),0,0)
                  *quat(cos(radians(motions[idx][6+(i-1)*3+1])/2),0,sin(radians(motions[idx][6+(i-1)*3+1])/2),0);

        quat q1 = quat(cos(radians(motions[(idx+1)%4][6+(i-1)*3+2])/2),0,0,sin(radians(motions[(idx+1)%4][6+(i-1)*3+2])/2))
                  *quat(cos(radians(motions[(idx+1)%4][6+(i-1)*3+0])/2),sin(radians(motions[(idx+1)%4][6+(i-1)*3+0])/2),0,0)
                  *quat(cos(radians(motions[(idx+1)%4][6+(i-1)*3+1])/2),0,sin(radians(motions[(idx+1)%4][6+(i-1)*3+1])/2),0);
        quat q2 = slerp(q0,q1,slerpweight);

        if(!i){
            Md.push_back(mat4(1.0f));
            mat4 m0 = mix(translate(vec3(motions[idx][0],motions[idx][1],motions[idx][2])),
                          translate(vec3(motions[(idx+1)%4][0],motions[(idx+1)%4][1],motions[(idx+1)%4][2])),
                          slerpweight);

            Ma.push_back(m0*mat4_cast(q2));
        }
        else{
            int Pi = jParents[i];
            Md.push_back(Md[Pi]*Mp[i]);
            Ma.push_back(Ma[Pi]*Mp[i]*mat4_cast(q2));
        }
    }

    vector<Vertex> VBO = playerVertices;
    for(auto & iVBO : VBO){
        tvec4<int> bone = iVBO.bone;
        tvec4<float> weight = iVBO.weight;
        vec4 normal = vec4(iVBO.nor,1.0);
        vec4 vertex = vec4(iVBO.pos,1.0);
        mat4 animation = mat4(0.0f);
        for(int j=0;j<4;j++){
            if(bone[j]<0) continue;
            animation += weight[j]*Ma[bone[j]]*inverse(Md[bone[j]]);
        }
        iVBO.pos = vec3(animation*vertex);
        iVBO.nor = normalize(vec3(animation*normal));
    }
    if(motion==1){
        vec3 P0v = getArcVec(P0x,P0y);
        vec3 P1v = getArcVec(P1x,P1y);
        float theta = acos(dot(P0v,P1v));
        mat4 rotMat = rotate(theta,cross(P0v,P1v));
        player->worldMat = player->worldMat * rotMat;
        P0x = P1x;
        P0y = P1y;
        motion = 0;
    }
    //////////////////////////////
    Scene::player->load(VBO, playerIndices);
    Scene::player->draw();
}



void Scene::mouseDownEvents(float x, float y) {
    //////////////////////////////
    /* TODO: Optional problem
     * object rotating
     * Automatically called when mouse down
     */
    P0x = x;
    P0y = y;
    //////////////////////////////
}

void Scene::mouseMoveEvents(float x, float y) {
    //////////////////////////////
    /* TODO: Optional problem
     * object rotating
     * Automatically called when mouse move
     */
    P1x = x;
    P1y = y;
    motion = 1;
    //////////////////////////////
}

