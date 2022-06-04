//: fresa by jose pazos perez, licensed under GPLv3
#include "r_camera.h"
#include "r_window.h"
#include "config.h"

using namespace Fresa;
using namespace Graphics;

#if defined USE_VULKAN
constexpr float viewport_y = -1.0f;
#elif defined USE_OPENGL
constexpr float viewport_y = 1.0f;
#endif

constexpr float small_bound = 0.1f;
constexpr float large_bound = 1e6f;

constexpr glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

//---------------------------------------------------
//: Create camera
//---------------------------------------------------
CameraData Camera::create(CameraProjection proj, glm::vec3 i_pos, float i_phi, float i_theta) {
    CameraData cam;
    
    //: Position
    cam.pos = i_pos;
    
    //: Direction
    cam.phi = i_phi;
    cam.theta = i_theta;
    
    //: Projection
    cam.projection = proj;
    
    //: Resolution (varies with projection)
    cam.resolution = (proj & PROJECTION_SCALED) ? Config::resolution : window.size;
    
    return cam;
}

//---------------------------------------------------
//: Get the camera direction using phi and theta (polar coordinates, physics convention)
//---------------------------------------------------
glm::vec3 Camera::getDirection() {
    return glm::normalize(glm::vec3(std::cos(camera.phi) * std::cos(camera.theta),
                                    std::sin(camera.theta),
                                    std::sin(camera.phi) * std::cos(camera.theta)));
}

//---------------------------------------------------
//: Get the view transformation matrix
//---------------------------------------------------
glm::mat4 Camera::getView() {
    glm::mat4 view;
    
    //: Polar coordinates
    glm::vec3 direction = getDirection();
    
    //: Gram–Schmidt ortonormalization
    glm::vec3 camera_right = glm::normalize(glm::cross(up, direction));
    glm::vec3 camera_up = glm::cross(direction, camera_right);
    
    //: View matrix
    view = glm::lookAt(camera.pos, camera.pos + direction, camera_up);
    
    return view;
}

//---------------------------------------------------
//: Get the projection transformation matrix
//---------------------------------------------------
glm::mat4 Camera::getProjection() {
    glm::mat4 proj;
    
    //: Orthographic (2D)
    if (camera.projection & PROJECTION_ORTHOGRAPHIC)
        proj = glm::ortho(0.0f, (float)camera.resolution.x, 0.0f, (float)camera.resolution.y, -large_bound, large_bound);
    
    //: Perspective (3D)
    if (camera.projection & PROJECTION_PERSPECTIVE)
        proj = glm::perspective(glm::radians(45.0f), (float)camera.resolution.x / (float)camera.resolution.y, small_bound, large_bound);
    
    //: Flip viewport where appropiate
    proj[1][1] *= -viewport_y;
    
    //: None (Vertex passthrough)
    if (camera.projection & PROJECTION_NONE)
        proj = glm::mat4(1.0f);
    
    return proj;
}

//---------------------------------------------------
//: Get the projection transformation matrix
//---------------------------------------------------
CameraTransform Camera::getTransform() {
    static CameraTransform transform{};
    static CameraData previous_camera{};
    
    //: Update projection
    if (camera.resolution != previous_camera.resolution or camera.projection != previous_camera.projection)
        transform.proj = getProjection();
    
    //: Update view
    if (camera.pos != previous_camera.pos or camera.phi != previous_camera.phi or camera.theta != previous_camera.theta)
        transform.view = getView();
    
    previous_camera = camera;
    return transform;
}
