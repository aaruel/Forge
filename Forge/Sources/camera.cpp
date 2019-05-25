//
//  camera.cpp
//  Glitter
//
//  Created by Aaron Ruel on 3/28/19.
//

#include "camera.hpp"
#include "glitter.hpp"
#include <iostream>

// Static

Camera * Camera::instance = 0;

Camera * Camera::getInstance() {
    if (instance == 0) {
        instance = new Camera(Input::getInstance());
    }
    return instance;
}

// Public

Camera::Camera(Input * _input) : input(_input) {}

Camera::Camera() { controllable = false; }

void Camera::render(GLint shader) {
    // Projection indicates the far and near frutsum planes, and warping
    // View is the position of the camera, the view direction, and the Z angle
    GLint uniformMVP0 = glGetUniformLocation(shader, "camPos");
    glUniform3fv(uniformMVP0, 1, glm::value_ptr(position));
    GLint uniformMVP1 = glGetUniformLocation(shader, "view");
    glUniformMatrix4fv(uniformMVP1, 1, GL_FALSE, glm::value_ptr(view));
    GLint uniformMVP2 = glGetUniformLocation(shader, "projection");
    glUniformMatrix4fv(uniformMVP2, 1, GL_FALSE, glm::value_ptr(proj));
    GLint uniformMVP3 = glGetUniformLocation(shader, "eye");
    glUniform3fv(uniformMVP3, 1, glm::value_ptr(eye));
}

void Camera::translate(glm::vec3 amount) {
    view = glm::translate(view, amount);
}

void Camera::update() {
    if (controllable) {
        processMouse();
        processKeys();
    }
    
    reloadView();
}

// Private

void Camera::processMouse() {
    if (!input->isNewPos()) return;
    
        Input::MousePos mousepos = input->getMousePos();
    
        double sensitivity = 0.175;
        mousepos.offsX *= sensitivity;
        mousepos.offsY *= sensitivity;

        yaw   += mousepos.offsX;
        pitch += mousepos.offsY;

        if(pitch > 89.0f) pitch = 89.0f;
        if(pitch < -89.0f) pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        eye = glm::normalize(front);
    
    // to deal with the callback
    input->resetPosBool();
}

void Camera::processKeys() {
    GLfloat multiplier = 1.0f;
    if (input->isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        multiplier *= 4.0f;
    }
    if (input->isKeyPressed(GLFW_KEY_W)) {
        position += speed * eye * multiplier;
    }
    if (input->isKeyPressed(GLFW_KEY_S)) {
        position -= speed * eye * multiplier;
    }
    if (input->isKeyPressed(GLFW_KEY_A)) {
        position -= glm::normalize(glm::cross(eye, up)) * speed;
    }
    if (input->isKeyPressed(GLFW_KEY_D)) {
        position += glm::normalize(glm::cross(eye, up)) * speed;
    }
    if (input->isKeyPressed(GLFW_KEY_SPACE)) {
        position += speed * up;
    }
    if (input->isKeyPressed(GLFW_KEY_C)) {
        position -= speed * up;
    }
}

void Camera::reloadView() {
    view = glm::lookAt(
        position,
        // In order to translate based on the eye
        position + eye,
        up
    );
}
