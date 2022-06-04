//: fresa by jose pazos perez, licensed under GPLv3
#pragma once

#include "r_types.h"

namespace Fresa::Graphics
{
    //---------------------------------------------------
    //: Data
    //---------------------------------------------------
    
    //---Linear Algebra---
    /*
     : Coordinate systems
            Local/object space -(model)-> World space -(view)-> View/camera space -(projection)-> Clip space -(viewport)-> Screen space
            Model matrix - scale, rotate and translate (in that order) an object to place it in world space
            View matrix - move the entire scene to be referenced from the camera's point of view
            Projection matrix - normalizes coordinates to be in the range [-1.0, 1.0]
     : Projections
            Orthographic
            Perspective
     */
    
    //: Camera projections
    //      Can be either orthographic or perspective
    //      Optionally it can also be scaled, which means that it will be scaled pixel perfectly on the screen
    //      None returns an identity matrix for the projection
    enum CameraProjection {
        PROJECTION_ORTHOGRAPHIC = 1 << 0,
        PROJECTION_PERSPECTIVE = 1 << 1,
        PROJECTION_SCALED = 1 << 2,
        PROJECTION_NONE = 1 << 3,
    };
    
    //: Camera transform
    //      Holds the projection and view matrices for the current camera, calculated via getTransform()
    struct CameraTransform {
        Members(CameraTransform, view, proj)
        glm::mat4 view;
        glm::mat4 proj;
    };
    
    //: Camera data
    //      Holds the camera position and rotations, as well as important information to calculate the transform matrices
    //      It is preferred that it only is modified by your camera system implementation
    inline struct CameraData {
        glm::vec3 pos;
        float phi;      //: 0 to 2*pi
        float theta;    //: 0 to pi
        CameraProjection projection;
        Vec2<ui16> resolution;
    } camera;
    
    //---------------------------------------------------
    //: Systems
    //---------------------------------------------------
    namespace Camera {
        //: Create a new CameraData object
        CameraData create(CameraProjection proj = PROJECTION_NONE, glm::vec3 i_pos = glm::vec3(0.f), float i_phi = 0.f, float i_theta = 0.f);
        
        //: Get the camera direction from the polar components phi, theta
        glm::vec3 getDirection();
        
        //: Create a view matrix
        glm::mat4 getView();
        
        //: Create a projection matrix
        glm::mat4 getProjection();
        
        //: Get a CameraTransform object from updated view and projection matrices
        CameraTransform getTransform();
    }
}
