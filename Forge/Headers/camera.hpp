//
//  camera.hpp
//  Glitter
//
//  Created by Aaron Ruel on 3/28/19.
//

#ifndef camera_h
#define camera_h

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include "glitter.hpp"
#include "input.hpp"

class Camera {
public:
    // Public methods
    Camera(Input * _input);
    void render(GLint shader);
    void translate(glm::vec3 amount);
    void update();
    
    // Getters
    glm::vec3 getPosition() { return position; }
    glm::vec3 getEye() { return eye; }
    
    static Camera * getInstance();
    
private:
    void reloadView();
    void processMouse();
    void processKeys();
    
    // Input driver
    Input * input;
    
    /// SINGLETON INSTANCE
    static Camera * instance;
    
    // Vector components
    double yaw = -90.0f; // counteract the eye direction on first mouse calculation
    double pitch = 0.0f;
    float speed = 0.1f;
    
    // LookAt components
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.f);
    glm::vec3 eye = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.f);

    // Initializes the camera to be at (0,0,10)
    // looking at (0,0,0)
    // roll vector is (0,1,0) (flat)
    glm::mat4 view;
    
    // 45 degree camera projection angle
    // aspect ratio is screen size
    // distance 1 near plane clipping
    // distance 100 far plane clipping
    glm::mat4 proj = glm::perspective(45.0f, (float)mWidth / (float)mHeight, 0.1f, 1000.0f);
};

#endif /* camera_h */
