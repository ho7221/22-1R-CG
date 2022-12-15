#include "scene.h"

#include "obj_teapot.h"
#include "tex_flower.h"

Shader* Scene::vertexShader = nullptr;
Shader* Scene::fragmentShader = nullptr;
Program* Scene::program = nullptr;
Camera* Scene::camera = nullptr;
Light* Scene::light = nullptr;
Object* Scene::teapot = nullptr;
Material* Scene::flower = nullptr;

void Scene::setup(AAssetManager* aAssetManager) {

    // set asset manager
    Asset::setManager(aAssetManager);

    // create shaders
    vertexShader = new Shader(GL_VERTEX_SHADER, "vertex.glsl");
    fragmentShader = new Shader(GL_FRAGMENT_SHADER, "fragment.glsl");

    // create program
    program = new Program(vertexShader, fragmentShader);

    // create camera
    camera = new Camera(program);
    camera->eye = vec3(60.0f, 00.0f, 0.0f);
    camera->updateCameraUVN();

    // create light
    light = new Light(program);
    light->position = vec3(100.0f, 0.0f, 0.0f);

    // create floral texture
    flower = new Material(program, texFlowerData, texFlowerSize);

    // create teapot object
    teapot = new Object(program, flower, objTeapotVertices, objTeapotIndices,
                        objTeapotVerticesSize, objTeapotIndicesSize);

    //////////////////////////////
    /* TODO: Problem 2.
     *  Scale the teapot by 2.0 along the y-axis.
     *  Rotate the teapot by 90° CW about the rotation axis defined by two points
     *  (0, 0, 10) → (10, 0, 20).
     */
    mat4 scaleM, rotMat;
    scaleM = transpose(mat4(1.0f, 0.0f, 0.0f, 0.0f,  // In OpenGL, the matrix must be transposed
                            0.0f, 2.0f, 0.0f, 0.0f,
                            0.0f, 0.0f, 1.0f, 0.0f,
                            0.0f, 0.0f, 0.0f, 1.0f));

    // 1. translate
    rotMat = transpose(mat4(1.0f,0.0f,0.0f,-10.0f,
                            0.0f,1.0f,0.0f,0.0f,
                            0.0f,0.0f,1.0f,0.0f,
                            0.0f,0.0f,0.0f,1.0f));

    // 2. axis rotation
    rotMat *= mat4(1.0f/sqrt(2),0.0f,1.0f/sqrt(2),0.0f,
                             0.0f,1.0f,0.0f,0.0f,
                             -1.0f/sqrt(2),-1.0f,1.0f/sqrt(2),0.0f,
                             0.0f,0.0f,0.0f,1.0f);
    // 3. r_x(270)
    rotMat *= transpose(mat4(1.0f,0.0f,0.0f,0.0f,
                             0.0f,0.0f,1.0f,0.0f,
                             0.0f,-1.0f,0.0f,0.0f,
                             0.0f,0.0f,0.0f,1.0f));

    // 4. axis rotation inverse
    rotMat *= transpose(mat4(1.0f/sqrt(2),0.0f,1.0f/sqrt(2),0.0f,
                   0.0f,1.0f,0.0f,0.0f,
                   -1.0f/sqrt(2),-1.0f,1.0f/sqrt(2),0.0f,
                   0.0f,0.0f,0.0f,1.0f));

    // 5. translate inverse
    rotMat *= transpose(mat4(1.0f,0.0f,0.0f,10.0f,
                             0.0f,1.0f,0.0f,0.0f,
                             0.0f,0.0f,1.0f,0.0f,
                             0.0f,0.0f,0.0f,1.0f));
    teapot->worldMatrix =  rotMat * scaleM;
    //////////////////////////////
}

void Scene::screen(int width, int height) {

    // set camera aspect ratio
    camera->aspect = (float) width / height;
}

void Scene::update(float deltaTime) {
    // use program
    program->use();

    //////////////////////////////
    /* TODO: Problem 3.
     *  Rotate the teapot about the z-axis.
     */
    mat4 rotMat;
    rotMat=glm::rotate(deltaTime,vec3(0,0,1));
    teapot->worldMatrix = rotMat * teapot->worldMatrix;
    //////////////////////////////


    camera->updateViewMatrix();
    camera->updateProjectionMatrix();
    light->setup();


    // draw teapot
    teapot->draw();
}

void Scene::rotateCamera(float dx,float dy) {
    float rotationSensitivity = 0.03;

    float thetaYaw=glm::radians(rotationSensitivity*dx);
    float thetaPinch=glm::radians(rotationSensitivity*dy);

    rotateCameraYaw(thetaYaw);
    rotateCameraPitch(thetaPinch);
}

void Scene::rotateCameraYaw(float theta) {

    //////////////////////////////
    /* TODO: Problem 4.
     *  calculate the rotated u,n vector about v axis.
     *  Argument theta is amount of rotation in radians. theta is positive when CCW.
     *  Note that u,v,n should always be orthonormal.
     *  The u vector can be accessed via camera->cameraU.
     *  The v vector can be accessed via camera->cameraV.
     *  The n vector can be accessed via camera->cameraN.
     */
    vec3 u=camera->cameraU;
    vec3 v=camera->cameraV;
    vec3 n=camera->cameraN;
    mat4 rotMat,R,mat;

    R=transpose(mat4(u[0],u[1],u[2],0.0f,
            v[0],v[1],v[2],0.0f,
            n[0],n[1],n[2],0.0f,
            0.0f,0.0f,0.0f,1.0f));
    rotMat=glm::rotate(-theta,vec3(0,1,0));
    mat=R*rotMat;
    camera->cameraU=vec3(mat[0][0],mat[0][1],mat[0][2]);
    camera->cameraV=vec3(mat[1][0],mat[1][1],mat[1][2]);
    camera->cameraN=vec3(mat[2][0],mat[2][1],mat[2][2]);
    camera->updateViewMatrix();
    //////////////////////////////
}

void Scene::rotateCameraPitch(float theta) {

    //////////////////////////////
    /* TODO: Problem 4.
     *  calculate the rotated v,n vector about u axis.
     *  Argument theta is amount of rotation in radians. Theta is positive when CCW.
     *  Note that u,v,n should always be orthonormal.
     *  The u vector can be accessed via camera->cameraU.
     *  The v vector can be accessed via camera->cameraV.
     *  The n vector can be accessed via camera->cameraN.
     */
    vec3 u=camera->cameraU;
    vec3 v=camera->cameraV;
    vec3 n=camera->cameraN;
    mat4 rotMat,R,mat;

    R=transpose(mat4(u[0],u[1],u[2],0.0f,
                     v[0],v[1],v[2],0.0f,
                     n[0],n[1],n[2],0.0f,
                     0.0f,0.0f,0.0f,1.0f));
    rotMat=glm::rotate(theta,vec3(1,0,0));
    mat=R*rotMat;
    camera->cameraU=vec3(mat[0][0],mat[0][1],mat[0][2]);
    camera->cameraV=vec3(mat[1][0],mat[1][1],mat[1][2]);
    camera->cameraN=vec3(mat[2][0],mat[2][1],mat[2][2]);
    camera->updateViewMatrix();
    //////////////////////////////
}

void Scene::translateLeft(float amount) {

    //////////////////////////////
    /* TODO: Problem 4.
     *  Calculate the camera position(eye) when translated left.
     */
    camera->eye+=vec3(0,0,amount);
    camera->updateViewMatrix();
    //////////////////////////////
}

void Scene::translateFront(float amount) {

    //////////////////////////////
    /* TODO: Problem 4.
     *  Calculate the camera position(eye) when translated front.
     */
    camera->eye+=vec3(-amount,0,0);
    camera->updateViewMatrix();
    //////////////////////////////
}

void Scene::translateRight(float amount) {

    //////////////////////////////
    /* TODO: Problem 4.
     *  Calculate the camera position(eye) when translated right.
     */
    camera->eye+=vec3(0,0,-amount);
    camera->updateViewMatrix();
    //////////////////////////////
}

void Scene::translateBack(float amount) {

    //////////////////////////////
    /* TODO: Problem 4.
     *  Calculate the camera position(eye) when translated back.
     */
    camera->eye+=vec3(amount,0,0);
    camera->updateViewMatrix();
    //////////////////////////////
}
