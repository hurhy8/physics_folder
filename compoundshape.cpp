#include <math.h>
#include <cmath>
#include <numbers>
#include <vector>
#include "polygon.h"
#include <algorithm>
#include <functional>
void CompoundShape::boundary_wall(Vec2 segv1, Vec2 segv2, std::function<bool(double, double)> comparator, BoundaryType side){
    // These boundaries are only used for axis aligned, world boundaries. Do NOT use them for
    // diagonal walls or walls that objects could hit from either side, use the SAT based one for that
    // The only (major) benefit of using this is that effectively disables the effect of tunnelling.
    std::pair<Vec2, Vec2> segment_vert = {segv1, segv2};
    Vec2 segment_edge = Vec2(segv2.x - segv1.x, segv2.y - segv1.y);
    std::vector<std::tuple<bool, Vec2, std::vector<Vec2>>> results;
    bool collided_poly = false;
    bool collided = false;
    Vec2 MTV;

    std::vector<Vec2> manifold;
    for (auto& child:child_polygons){
        //std::cout << child->local_center_x << "\n";
        double EPSILON = 1e-6;
        auto poly_ptr = std::dynamic_pointer_cast<Polygon>(child);
        if (std::fabs(segment_edge.y) < EPSILON){
            
            results.push_back(poly_ptr->y_boundaries(segment_edge, segment_vert, segv1.y, comparator, side));
        }
        else if (std::fabs(segment_edge.x) < EPSILON){
            results.push_back(poly_ptr->x_boundaries(segment_edge, segment_vert, segv1.x, comparator, side));
        }

    }
    std::vector<Vec2> manifolds;
    Vec2 deepest_MTV;
    std::vector<Vec2> combined_manifold;
    double max_penetration = 0.0;

    for (auto result : results){
        std::tie(collided_poly, MTV, manifold) = result;
        if (collided_poly){
            double depth = std::sqrt(MTV.x * MTV.x + MTV.y * MTV.y);
            if (depth > max_penetration){
                max_penetration = depth;
                deepest_MTV = MTV;
            }
            collided = true;
            combined_manifold.insert(combined_manifold.end(), manifold.begin(), manifold.end());
        }
    }

    if (collided){
        
        impulse_resolution(deepest_MTV, segment_edge, segment_vert, combined_manifold);
    }
    

    
}

void CompoundShape::clamp_motion(){
    
    if (std::abs(vx) < 1e-6) vx = 0;
    if (std::abs(vy) < 1e-6) vy = 0;
    //omega *= std::exp(-0.65 * dt);
    
    if (!std::isfinite(omega)) omega = 0.0;
    //omega = std::max(std::min(omega, 3.5), -3.5);
}

void CompoundShape::put_to_sleep(){
    
    double velocity_magnitude = std::sqrt(vx * vx + vy * vy + (length/2 * omega) * (length/2 * omega));
    
    if (velocity_magnitude < 5 && !sleep) {
       // ++num_times_sleepy;
    }
    else{
        num_times_sleepy = 0;
    }
    if ((num_times_sleepy > 720 || sleep) && allow_motion){
        
        double new_angle = (PI * (vertices - 2))/(2*vertices); 
        //std::cout << std::abs(new_angle - angle) << "\n";
        
        if (!std::abs(new_angle - angle) < 0.174533){ 
            //angle = new_angle;
            sleep = true;
            gravity = 0;
            vy = 0;
            vx = 0;
            omega = 0; }
        

    }
}

void CompoundShape::apply_rotation(Vec2 lever_arm_r, double Jx, double Jy){
    double torque_impulse = lever_arm_r.x * Jy - lever_arm_r.y * Jx;
    double effective_inertia = inertia;
    if (torque_impulse < 0){
        effective_inertia /= 1.0;
    }
    double omega_change = (torque_impulse/effective_inertia);
    omega += omega_change;
}

void CompoundShape::update_aabb() {
    if (world_vertices.empty()) return;
    aabb.min_x = std::numeric_limits<double>::infinity();
    aabb.max_x = -std::numeric_limits<double>::infinity();
    aabb.min_y = std::numeric_limits<double>::infinity();
    aabb.max_y = -std::numeric_limits<double>::infinity();

    for (auto& child: child_polygons){
        for (auto &v : child->world_vertices) {
            if (v.x < aabb.min_x) aabb.min_x = v.x;
            if (v.x > aabb.max_x) aabb.max_x = v.x;
            if (v.y < aabb.min_y) aabb.min_y = v.y;
            if (v.y > aabb.max_y) aabb.max_y = v.y;
        }
    }

}

bool CompoundShape::aabb_overlap(const AABB& a, const AABB& b) {
    return (a.min_x <= b.max_x && a.max_x >= b.min_x &&
            a.min_y <= b.max_y && a.max_y >= b.min_y);
}


void CompoundShape::impulse_resolution(Vec2 MTV, Vec2 segment_edge,
    std::pair<Vec2, Vec2> segment_vertices, std::vector<Vec2> manifold){

        double correction_fraction = 1;
        center_x += MTV.x * correction_fraction;
        center_y += MTV.y * correction_fraction;
        double seg_mag = std::hypot(segment_edge.x, segment_edge.y);
        double seg_nx = -segment_edge.y / seg_mag;
        double seg_ny = segment_edge.x / seg_mag;
        double to_body_x = center_x - segment_vertices.first.x;
        double to_body_y = center_y - segment_vertices.first.y;
        if (to_body_x * seg_nx + to_body_y * seg_ny < 0){
            seg_nx *= -1;
            seg_ny *= -1;
        }
        double lever_arm_x = 0;
        double lever_arm_y = 0;
        for (auto& pt : manifold){
            lever_arm_x += pt.x - center_x;
            lever_arm_y += pt.y - center_y;
        }
        lever_arm_x /= manifold.size();
        lever_arm_y /= manifold.size();
    
        

            double v_rot_x = -omega * lever_arm_y;
            double v_rot_y = omega * lever_arm_x;
            double v_contact_x = vx + v_rot_x;
            double v_contact_y = vy + v_rot_y;
            double vel_along_normal = v_contact_x * seg_nx + v_contact_y * seg_ny;

            if (vel_along_normal > 0){
                
                return;
            }
            double epsilon = 0.001;
            double impulse_scalar;


            double local_e= collision_elasticity;
            if (vel_along_normal >-0.5){
                local_e = 0;
            }
            double r_cross_n = lever_arm_x * seg_ny - lever_arm_y * seg_nx;
            double denom = (1.0/mass) + (r_cross_n*r_cross_n)/inertia;
            impulse_scalar = (-(1.0 + local_e) * vel_along_normal)/denom;
        
            double Jx = (impulse_scalar) * seg_nx;
            double Jy = impulse_scalar * seg_ny;
            
            vy += Jy/mass;
            vx += Jx/mass;
            apply_rotation(Vec2(lever_arm_x, lever_arm_y), Jx, Jy);


            double mu_k = 0.3;
            double mu_s = 0.35;
            
            v_rot_x = -omega * lever_arm_y;
            v_rot_y = omega * lever_arm_x;
            v_contact_x = vx + v_rot_x;
            v_contact_y = vy + v_rot_y;
            vel_along_normal = v_contact_x * seg_nx + v_contact_y * seg_ny;
            Vec2 vn = Vec2(vel_along_normal * seg_nx, vel_along_normal * seg_ny);
            //tangentional velocity
            Vec2 t = Vec2(-seg_ny, seg_nx);
            double vt_dot = v_contact_x * t.x + v_contact_y *t.y;
            Vec2 vt = Vec2(vt_dot * t.x,  vt_dot *t.y);

            
            double j_mag = (std::sqrt(Jx * Jx + Jy *Jy));
            double r_cross_t = lever_arm_x * t.y - lever_arm_y * t.x;
            
            double denom_t = (1.0/mass) + (r_cross_t * r_cross_t)/inertia;
            
            double full_denom = denom_t;
            double static_jt_unclamped = -vt_dot/full_denom;
            double limit = mu_s * j_mag;
            double jt;
            jt = std::clamp( + static_jt_unclamped, -limit, limit);
            if (std::abs(static_jt_unclamped) <= limit){
                jt = static_jt_unclamped;
            }
            else{
                jt = mu_k * j_mag;
                jt *= static_jt_unclamped > 0 ? (1) : (-1);
            }
            double jtx = jt * t.x;
            double jty = jt * t.y;
            vx += jtx/ mass;
            vy += jty/ mass;
            apply_rotation(Vec2(lever_arm_x, lever_arm_y), jtx, jty);



        
        

        }


    void CompoundShape::update_world_vertices(){
        angle = std::fmod(angle, (2 * PI));
        double cos_a = std::cos(angle);
        double sin_a = std::sin(angle);

        world_vertices.clear();
        
        

        for (auto& child:child_polygons){
            child->world_vertices.clear();
            for (auto lv : child->local_vertices){
                double dx = (child->local_center_x - local_center_x) + lv.x;
                double dy = (child->local_center_y - local_center_y) + lv.y;
                double rx = dx*cos_a-dy*sin_a;
                double ry = dx*sin_a+dy*cos_a;
                double wx = rx + center_x;
                double wy = ry + center_y;
                Vec2 actual_vert = Vec2(wx, wy);
                world_vertices.push_back(actual_vert);
                child->world_vertices.push_back(actual_vert);
                double cx = child->local_center_x;
                double cy = child->local_center_y;

                double crx = cx * cos_a - cy * sin_a;
                double cry = cx * sin_a + cy * cos_a;

                child->center_x = crx + center_x;
                child->center_y = cry + center_y;
        
            }
        }
        if (world_vertices.size() != vertices) {
            std::cout << "World vertices size: " << world_vertices.size() << "\n";
            std::cout << "vertices size: " << vertices << "\n";
            throw std::runtime_error("vertex count mismatch in CompoundShape::update_world_vertices()");
        }

            // Build edges safely
        edge_coordinates.clear();
        size_t start_index = 0;

        for (auto& child: child_polygons){
            size_t n = child->local_vertices.size();
            for (size_t i = 0; i < n; i++){
                edge_coordinates.push_back({
                    world_vertices[start_index + i],
                    world_vertices[start_index + (i + 1) % n]
                });
            }
            start_index += n;
        }
    }


    void CompoundShape::apply_force(double dt){
        vy += (gravity) * dt;
    }
    void CompoundShape::apply_motion(double dt){
        center_x += vx * dt;
        angle += omega * dt;
       
        center_y += vy * dt;
        update_world_vertices();
        update_aabb();
    }