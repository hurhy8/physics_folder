#include <math.h>
#include <cmath>
#include <numbers>
#include <vector>
#include "polygon.h"
#include <algorithm>
#include <functional>

Vec2 PendulumJoint::update_local_anchor() const {
    double angle = std::fmod(rigid_body->angle, (2 * PI));
    double cos_a = std::cos(angle);
    double sin_a = std::sin(angle);
    

    double rx = body_anchor.x*cos_a-body_anchor.y*sin_a;
    double ry = body_anchor.x*sin_a+body_anchor.y*cos_a;
    double wx = rx + rigid_body->center_x;
    double wy = ry + rigid_body->center_y;
    Vec2 actual_vert = Vec2(wx, wy);
    return actual_vert;
        
}

Vec2 DynamicJoint::update_local_anchor(Vec2 anchor, std::shared_ptr<Shape> shape) const {
    double angle = std::fmod(shape->angle, (2 * PI));
    double cos_a = std::cos(angle);
    double sin_a = std::sin(angle);
    

    double rx = anchor.x*cos_a-anchor.y*sin_a;
    double ry = anchor.x*sin_a+anchor.y*cos_a;
    double wx = rx + shape->center_x;
    double wy = ry + shape->center_y;
    Vec2 actual_vert = Vec2(wx, wy);
    return actual_vert;
        
}


void PendulumJoint::impulse_resolution(){
    double lever_arm_x = 0;
    double lever_arm_y = 0;
    Vec2 body_to_world = update_local_anchor();
    lever_arm_x += body_to_world.x - rigid_body->center_x;
    lever_arm_y += body_to_world.y - rigid_body->center_y;
    

    Vec2 lever_arm_r = Vec2(lever_arm_x, lever_arm_y);
    double v_rot_x = -rigid_body->omega * lever_arm_r.y;
    double v_rot_y = rigid_body->omega * lever_arm_r.x;
    double v_contact_x =rigid_body->vx + v_rot_x;
    double v_contact_y =rigid_body->vy + v_rot_y;
    //std::tuple<double, double, Vec2> result = {v_contact_x, v_contact_y, lever_arm_r};
    Vec2 direction_vector = Vec2(body_to_world.x - world_anchor.x, body_to_world.y - world_anchor.y);
    double direction_mag = std::sqrt(direction_vector.x * direction_vector.x + direction_vector.y * direction_vector.y);
    Vec2 anchor_direction = Vec2(direction_vector.x/direction_mag, direction_vector.y/direction_mag);
    double relative_velocity_along_anchor = anchor_direction.x * v_contact_x + anchor_direction.y * v_contact_y;
    if (direction_mag > rope_length && relative_velocity_along_anchor > 0){
        double r_cross_n = lever_arm_x * anchor_direction.y - lever_arm_y * anchor_direction.x;
        double denom = (1.0/rigid_body->mass) + (r_cross_n*r_cross_n)/rigid_body->inertia;

        double error = direction_mag -rope_length;
        double bias = ((0.3 * error)/dt);

        double impulse_scalar = -(relative_velocity_along_anchor + bias)/denom;
        double Jx = impulse_scalar * anchor_direction.x;
        double Jy = impulse_scalar * anchor_direction.y;



        rigid_body->vx += Jx/rigid_body->mass;
        rigid_body->vy += Jy/rigid_body->mass;
        rigid_body->apply_rotation(lever_arm_r, Jx, Jy);
        
    }

}


std::tuple<double, double, Vec2> DynamicJoint::relative_velocity(Shape& polygon, Vec2 contact){
    double lever_arm_x = 0;
    double lever_arm_y = 0;

    lever_arm_x += contact.x - polygon.center_x;
    lever_arm_y += contact.y - polygon.center_y;
    

    Vec2 lever_arm_r = Vec2(lever_arm_x, lever_arm_y);
    double v_rot_x = -polygon.omega * lever_arm_r.y;
    double v_rot_y = polygon.omega * lever_arm_r.x;
    double v_contact_x = polygon.vx + v_rot_x;
    double v_contact_y = polygon.vy + v_rot_y;
    std::tuple<double, double, Vec2> result = {v_contact_x, v_contact_y, lever_arm_r};
    return result;
}
void DynamicJoint::impulse_resolution(){

    Vec2 body_to_world1 = update_local_anchor(body_anchor1, rigid_body1);
    Vec2 body_to_world2 = update_local_anchor(body_anchor2, rigid_body2);
    Vec2 radius1 = Vec2(body_to_world1.x, body_to_world1.y);

    Vec2 radius2 = Vec2(body_to_world2.x, body_to_world2.y);

    auto [v_contact_x, v_contact_y, lever_arm] = relative_velocity(*rigid_body1, radius1);
    auto [v_contact_x_other, v_contact_y_other, lever_arm_other] = relative_velocity(*rigid_body2, radius2);

    //std::tuple<double, double, Vec2> result = {v_contact_x, v_contact_y, lever_arm_r};
    Vec2 direction_vector = Vec2(body_to_world2.x - body_to_world1.x, body_to_world2.y - body_to_world1.y);
    double direction_mag = std::sqrt(direction_vector.x * direction_vector.x + direction_vector.y * direction_vector.y);
    
    Vec2 anchor_direction = Vec2(direction_vector.x/direction_mag, direction_vector.y/direction_mag);

    double rvx = v_contact_x_other - v_contact_x;
    double rvy = v_contact_y_other - v_contact_y;
    double relative_velocity_along_anchor = (rvx * anchor_direction.x + rvy * anchor_direction.y);
    double r_cross_n = lever_arm.x * anchor_direction.y - lever_arm.y * anchor_direction.x;
    double denom = (1.0/rigid_body1->mass) + (r_cross_n*r_cross_n)/rigid_body1->inertia;


    double r_cross_n_other = lever_arm_other.x * anchor_direction.y - lever_arm_other.y * anchor_direction.x;
    double denom_other = (1.0/rigid_body2->mass) + (r_cross_n_other*r_cross_n_other)/rigid_body2->inertia;

    if (direction_mag > rope_length && relative_velocity_along_anchor > 0){
        //std::cout << "COLLISION RESOLUTION\n" << "ROPE LENGTH: " << rope_length << "\n";
        double error = direction_mag -rope_length;
        double bias = ((0.4 * error)/dt);
        
        double impulse_scalar = -(relative_velocity_along_anchor+bias)/(denom+denom_other);
        double Jx = impulse_scalar * anchor_direction.x;
        double Jy = impulse_scalar * anchor_direction.y;

       // std::cout
       // << "DEBUG: dist=" << direction_mag
       // << ", rope_length=" << rope_length
      //  << ", error=" << (direction_mag - rope_length)
      //  << ", anchor_dir=(" << anchor_direction.x << "," << anchor_direction.y << ")"
      //  << ", denom=" << (denom + denom_other)
      //  << ", rv_along_anchor=" << relative_velocity_along_anchor
       // << ", impulse_scalar=" << impulse_scalar
      //  << ", rb1_mass=" << rigid_body1->mass
     //   << ", rb2_mass=" << rigid_body2->mass
     //   << ", rb1_inertia=" << rigid_body1->inertia
     //   << ", rb2_inertia=" << rigid_body2->inertia
     //   << "\n";

        rigid_body1->vx -= Jx/rigid_body1->mass;
        rigid_body1->vy -= Jy/rigid_body1->mass;
        rigid_body1->apply_rotation(lever_arm, -Jx, -Jy);


        
        rigid_body2->vx += Jx/rigid_body2->mass;
        rigid_body2->vy += Jy/rigid_body2->mass;
        rigid_body2->apply_rotation(lever_arm_other, Jx, Jy);

    }
    
   

}

Vec2 SoftJoint::update_local_anchor(Vec2 anchor, std::shared_ptr<Shape> shape) const {
    double angle = std::fmod(shape->angle, (2 * PI));
    double cos_a = std::cos(angle);
    double sin_a = std::sin(angle);
    

    double rx = anchor.x*cos_a-anchor.y*sin_a;
    double ry = anchor.x*sin_a+anchor.y*cos_a;
    double wx = rx + shape->center_x;
    double wy = ry + shape->center_y;
    Vec2 actual_vert = Vec2(wx, wy);
    return actual_vert;
        
}

std::tuple<double, double, Vec2> SoftJoint::relative_velocity(Shape& polygon, Vec2 contact){
    double lever_arm_x = 0;
    double lever_arm_y = 0;

    lever_arm_x += contact.x - polygon.center_x;
    lever_arm_y += contact.y - polygon.center_y;
    

    Vec2 lever_arm_r = Vec2(lever_arm_x, lever_arm_y);
    double v_rot_x = -polygon.omega * lever_arm_r.y;
    double v_rot_y = polygon.omega * lever_arm_r.x;
    double v_contact_x = polygon.vx + v_rot_x;
    double v_contact_y = polygon.vy + v_rot_y;
    std::tuple<double, double, Vec2> result = {v_contact_x, v_contact_y, lever_arm_r};
    return result;
}
void SoftJoint::impulse_resolution(){

    Vec2 body_to_world1 = update_local_anchor(body_anchor1, rigid_body1);
    Vec2 body_to_world2 = update_local_anchor(body_anchor2, rigid_body2);
    Vec2 radius1 = Vec2(body_to_world1.x, body_to_world1.y);

    Vec2 radius2 = Vec2(body_to_world2.x, body_to_world2.y);

    auto [v_contact_x, v_contact_y, lever_arm] = relative_velocity(*rigid_body1, radius1);
    auto [v_contact_x_other, v_contact_y_other, lever_arm_other] = relative_velocity(*rigid_body2, radius2);

    //std::tuple<double, double, Vec2> result = {v_contact_x, v_contact_y, lever_arm_r};
    Vec2 direction_vector = Vec2(body_to_world2.x - body_to_world1.x, body_to_world2.y - body_to_world1.y);
    double direction_mag = std::sqrt(direction_vector.x * direction_vector.x + direction_vector.y * direction_vector.y);
    
    Vec2 anchor_direction = Vec2(direction_vector.x/direction_mag, direction_vector.y/direction_mag);

    double rvx = v_contact_x_other - v_contact_x;
    double rvy = v_contact_y_other - v_contact_y;
    double relative_velocity_along_anchor = (rvx * anchor_direction.x + rvy * anchor_direction.y);
    
    if (direction_mag > rope_length){
        //std::cout << "COLLISION RESOLUTION\n" << "ROPE LENGTH: " << rope_length << "\n";
        double r_cross_n = lever_arm.x * anchor_direction.y - lever_arm.y * anchor_direction.x;
        double denom = (1.0/rigid_body1->mass) + (r_cross_n*r_cross_n)/rigid_body1->inertia;


        double r_cross_n_other = lever_arm_other.x * anchor_direction.y - lever_arm_other.y * anchor_direction.x;
        double denom_other = (1.0/rigid_body2->mass) + (r_cross_n_other*r_cross_n_other)/rigid_body2->inertia;

        
        double error = rope_length - direction_mag;
        
        double damping_coefficient = 0.1;
        double force_spring = (error*k) - (damping_coefficient  * relative_velocity_along_anchor);

        double Jx = force_spring * anchor_direction.x;
        double Jy = force_spring * anchor_direction.y;

        rigid_body1->vx -= (Jx*dt)/rigid_body1->mass;
        rigid_body1->vy -= (Jy*dt)/rigid_body1->mass;
        rigid_body1->apply_rotation(lever_arm, -Jx, -Jy);


        
        rigid_body2->vx += (Jx*dt)/rigid_body2->mass;
        rigid_body2->vy += (Jy*dt)/rigid_body2->mass;
        rigid_body2->apply_rotation(lever_arm_other, Jx, Jy);
        

    }

}




void PendulumSolver::iterative_solver(){
    for (auto joint:joints){
        for (int i = 0; i <150; ++i){

            joint->impulse_resolution();
        }
    }
}