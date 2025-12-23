#include <math.h>
#include <cmath>
#include <numbers>
#include <vector>
#include "polygon.h"
#include <algorithm>
#include <functional>
std::vector<Vec2> Polygon::calculate_vertices_and_inertia(){
    if (vertices < 3) throw std::runtime_error("Polygon must have at least 3 vertices");
    if (mass <= 0) throw std::runtime_error("Polygon mass must be positive");
    if (length <= 0) throw std::runtime_error("Polygon length must be positive");
    inertia = 0;
    std::vector<Vec2> vertex_points = {};
    double radius = length/(2*std::sin(PI/vertices));
    for (int i=0; i < vertices; i++){
        double x =  radius * std::cos((2*PI*i)/vertices + angle);
        double y =  radius * std::sin((2*PI*i)/vertices + angle);
        vertex_points.push_back(Vec2(x, y));
    }
    if (vertices > 4){
        inertia = (0.5 * mass * radius * radius * (std::sin(2*PI/vertices)/
        (2 * PI / vertices))) *100;
    }
    else if (vertices == 4){
        inertia = ((mass * length * length)/6) *100;
    }
    else if (vertices == 3){
        inertia = ((mass * length * length)/12) *100;
    }

    return vertex_points;
};

std::vector<Vec2> Rectangle::calculate_vertices_and_inertia()  {
    
    Vec2 v1 = Vec2( - width/2,  - length/2);
    Vec2 v2 = Vec2( + width/2,  - length/2);
    Vec2 v3 = Vec2( + width/2,  + length/2);
    Vec2 v4 = Vec2( - width/2,  + length/2);
    inertia= ((1.0/12.0 * mass * length * length) + (1.0/12.0 * mass * width * width)) * 100;
    
    return {v1, v2, v3, v4};
}
bool Rectangle::aabb_overlap(const AABB& a, const AABB& b) {
    return (a.min_x <= b.max_x && a.max_x >= b.min_x &&
            a.min_y <= b.max_y && a.max_y >= b.min_y);
}
void Rectangle::apply_rotation(Vec2 lever_arm_r, double Jx, double Jy){
    double torque_impulse = lever_arm_r.x * Jy - lever_arm_r.y * Jx;
    double effective_inertia = inertia;
    if (torque_impulse < 0){
        effective_inertia /= 1.225;
    }
    double omega_change = torque_impulse/effective_inertia*10;
    omega += omega_change;
}

std::vector<Vec2> IsoscelesTriangle::calculate_vertices_and_inertia()  {
    
    Vec2 v1 = Vec2(0,0);
    Vec2 v2 = Vec2(width, 0);
    Vec2 v3 = Vec2(width/2, std::sqrt(length*length - (width/2) * (width/2)));
    Vec2 v1_rot(
        v1.x * std::cos(angle) - v1.y * std::sin(angle),
        v1.x * std::sin(angle) + v1.y * std::cos(angle)
    );
    
    Vec2 v2_rot(
        v2.x * std::cos(angle) - v2.y * std::sin(angle),
        v2.x * std::sin(angle) + v2.y * std::cos(angle)
    );
    
    Vec2 v3_rot(
        v3.x * std::cos(angle) - v3.y * std::sin(angle),
        v3.x * std::sin(angle) + v3.y * std::cos(angle)
    );
    inertia= ((mass/36) * (width*width + length*length)) * 100;
    
    return {v1_rot, v2_rot, v3_rot};
}
bool IsoscelesTriangle::aabb_overlap(const AABB& a, const AABB& b) {
    return (a.min_x <= b.max_x && a.max_x >= b.min_x &&
            a.min_y <= b.max_y && a.max_y >= b.min_y);
}
void IsoscelesTriangle::apply_rotation(Vec2 lever_arm_r, double Jx, double Jy){
    double torque_impulse = lever_arm_r.x * Jy - lever_arm_r.y * Jx;
    double effective_inertia = inertia;
    if (torque_impulse < 0){
        effective_inertia /= 1.225;
    }
    double omega_change = torque_impulse/effective_inertia;
    omega += omega_change;
}

std::pair<double, double> Polygon::compute_line_segment_projection(std::pair<Vec2, Vec2> segment_vertices, Vec2 normal){
    std::vector<double> projections = {};
    for (auto vertex : {segment_vertices.first, segment_vertices.second}){
        double x = vertex.x;
        double y = vertex.y;
        double p = x * normal.x + y * normal.y;
        projections.push_back(p);
    }
    auto [min_projection, max_projection] = std::minmax_element(projections.begin(), projections.end());
    std::pair<double, double> projection = {*min_projection, *max_projection};
    return projection;

}
std::tuple<bool, Vec2, std::vector<Vec2>>Polygon::x_boundaries(Vec2 segment_edge, 
    std::pair<Vec2, Vec2> segment_vertices, double boundary,
    std::function<bool(double, double)> comparator, BoundaryType side){
        edge_coordinates.clear();
        for (size_t i = 0; i < vertices; i++) {
            std::pair<Vec2, Vec2> edge = {world_vertices[i], world_vertices[(i + 1) % vertices]};
            edge_coordinates.push_back(edge);
        } 
        


        
        double max_overlap = 0.0;
        for (auto vertex: world_vertices){
            if (comparator(vertex.x, boundary)){
               
                double overlap = std::abs(vertex.x - boundary);
                if (overlap > max_overlap){
                    max_overlap = overlap;
                }
                
                
                
            }
    }
    double seg_mag = std::hypot(segment_edge.x, segment_edge.y);
    std::vector<Vec2> manifold = build_contact_manifold(segment_vertices.first, segment_vertices.second, 
        Vec2(-segment_edge.y / seg_mag, -segment_edge.x / seg_mag), edge_coordinates, 2);
    if (max_overlap){
        
        double direction = (side == BoundaryType::Right) ? -1.0 : 1.0;
        return {true, Vec2(direction * max_overlap, 0), manifold};
    }
    return {false, Vec2(0, 0), {(0,0)}};
    }



std::tuple<bool, Vec2, std::vector<Vec2>> Polygon::y_boundaries(Vec2 segment_edge, 
    std::pair<Vec2, Vec2> segment_vertices,    double boundary,
    std::function<bool(double, double)> comparator, BoundaryType side){
        edge_coordinates.clear();
        for (size_t i = 0; i < vertices; i++) {
            std::pair<Vec2, Vec2> edge = {world_vertices[i], world_vertices[(i + 1) % vertices]};
            edge_coordinates.push_back(edge);
        } 
        


        
        double max_overlap = 0.0;
        for (auto vertex: world_vertices){
            
            if (comparator(vertex.y, boundary)){
                ;
                double overlap = std::abs(vertex.y - boundary);
                if (overlap > max_overlap){
                    max_overlap = overlap;
                }
                
                
                
            }
    }
    double seg_mag = std::hypot(segment_edge.x, segment_edge.y);
    std::vector<Vec2> manifold = build_contact_manifold(segment_vertices.first, segment_vertices.second, 
        Vec2(-segment_edge.y / seg_mag, -segment_edge.x / seg_mag), edge_coordinates, 2);
    if (max_overlap){
        double direction = (side == BoundaryType::Right) ? -1.0 : 1.0;
        return {true, Vec2(0, direction * max_overlap), manifold};
    }
    return {false, Vec2(0, 0), {(0,0)}};
    }
std::tuple<bool, Vec2, std::vector<Vec2>> Polygon::sat_algorithm_boundary(Vec2 segment_edge, 
    std::pair<Vec2, Vec2> segment_vertices){
    edge_coordinates.clear();

    size_t n = world_vertices.size();
    if (n == 0) {
        std::cerr << "Warning: world_vertices is empty! Returning false.\n";
        return {false, Vec2(0, 0), {(0,0)}};
    }
    for (size_t i = 0; i < vertices; i++) {
        std::pair<Vec2, Vec2> edge = {world_vertices[i], world_vertices[(i + 1) % vertices]};
        edge_coordinates.push_back(edge);
    }


    // Compute normals
    std::vector<Vec2> normals;
    for (const auto& edge : edge_coordinates) {
        Vec2 vec(edge.first.x - edge.second.x, edge.first.y - edge.second.y);
        double mag = std::hypot(vec.x, vec.y);
        if (mag == 0.0) continue;  // skip degenerate edges
        normals.push_back(Vec2(-vec.y / mag, -vec.x / mag));
    }

    // Add segment normal
    double seg_mag = std::hypot(segment_edge.x, segment_edge.y);
    if (seg_mag != 0.0) {
        normals.push_back(Vec2(-segment_edge.y / seg_mag, -segment_edge.x / seg_mag));
    }

    // SAT loop
    double min_overlap = std::numeric_limits<double>::infinity();
    Vec2 mtv_axis(0, 0);

    for (const auto& normal : normals) {
        // Project polygon
        double poly_min = std::numeric_limits<double>::infinity();
        double poly_max = -std::numeric_limits<double>::infinity();
        for (const auto& vertex : world_vertices) {
            double p = vertex.x * normal.x + vertex.y * normal.y;
            poly_min = std::min(poly_min, p);
            poly_max = std::max(poly_max, p);
        }

        // Project segment
        auto [seg_min, seg_max] = compute_line_segment_projection(segment_vertices, normal);

        // Check overlap
        if (poly_max < seg_min || seg_max < poly_min) {
            return {false, Vec2(0, 0), {(0,0)}};
        }

        double overlap = std::min(poly_max, seg_max) - std::max(poly_min, seg_min);
        if (seg_min == seg_max){
            if (poly_min <= seg_min && seg_min <= seg_max){
                overlap = std::min(poly_max-seg_min, seg_min - poly_min);
            }
            else{
                return {false, Vec2(0, 0), {(0,0)}};
            }
        }
        if (overlap < min_overlap) {
            min_overlap = overlap;
            mtv_axis = normal;
        }
    }

    // MTV direction
    Vec2 seg_center((segment_vertices.first.x + segment_vertices.second.x) / 2,
                    (segment_vertices.first.y + segment_vertices.second.y) / 2);
    Vec2 direction(seg_center.x - center_x, seg_center.y - center_y);
    if (direction.x * mtv_axis.x + direction.y * mtv_axis.y > 0) {
        mtv_axis = Vec2(-mtv_axis.x, -mtv_axis.y);
    }

    Vec2 MTV(mtv_axis.x * min_overlap, mtv_axis.y * min_overlap);
    std::vector<Vec2> manifold = build_contact_manifold(segment_vertices.first, segment_vertices.second, 
        Vec2(-segment_edge.y / seg_mag, -segment_edge.x / seg_mag), edge_coordinates, 2);
    return {true, MTV, manifold};
}

Vec2 Polygon::closest_point_on_segment(Vec2 seg_vert_1, Vec2 seg_vert_2, Vec2 P){
    Vec2 AB = Vec2(seg_vert_2.x - seg_vert_1.x, seg_vert_2.y-seg_vert_1.y);
    Vec2 AP = Vec2(P.x - seg_vert_1.x, P.y - seg_vert_1.y);
    double ab2 = AB.x * AB.x + AB.y*AB.y;
    double t = 0.0;
    if (ab2 == 0){
        t = 0.0;
    }
    else{
        t = std::max(0.0, std::min(1.0, (AP.x * AB.x + AP.y * AB.y)/ab2));
    }
    return Vec2(seg_vert_1.x + AB.x*t, seg_vert_1.y + AB.y * t);
}

void Polygon::clamp_motion(){
    
    if (std::abs(vx) < 1e-6) vx = 0;
    if (std::abs(vy) < 1e-6) vy = 0;
    omega *= std::exp(-0.65 * dt);
    
    if (!std::isfinite(omega)) omega = 0.0;
    omega = std::max(std::min(omega, 3.5), -3.5);
}

bool Polygon::is_flat(double angle, std::vector<double> flat_angles, double tolerance){
    for (auto fa:flat_angles){
        if (std::fmod(std::abs(angle - fa + PI), (2*PI) - PI) < tolerance){
            return true;
        }
        
    }
    return  false;
}

void Polygon::put_to_sleep(){
    
    double velocity_magnitude = std::sqrt(vx * vx + vy * vy + (length/2 * omega) * (length/2 * omega));
    
    if (velocity_magnitude < 20 && !sleep) {
        ++num_times_sleepy;
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

std::vector<Vec2> Polygon::build_contact_manifold(Vec2 seg_a, Vec2 seg_b,
    Vec2 seg_normal, std::vector<std::pair<Vec2, Vec2>> polygon_edges, int max_contacts, double epsilon){
        double mag = std::sqrt(seg_normal.x * seg_normal.x + seg_normal.y * seg_normal.y);
        Vec2 normal = Vec2(seg_normal.x/mag, seg_normal.y/mag);
        std::vector<Vec2> raw_contacts = {};
        for (auto vertices : polygon_edges){
            Vec2 vi = vertices.first;
            Vec2 vj = vertices.second;
            Vec2 di = closest_point_on_segment(seg_a, seg_b, vi);
            Vec2 dj = closest_point_on_segment(seg_a, seg_b, vj);
            double dx_i = vi.x - di.x;
            double dy_i = vi.y - di.y;
            double dist_i = dx_i * normal.x + dy_i * normal.y;

            double dx_j = vj.x - dj.x;
            double dy_j = vj.y - dj.y;
            double dist_j = dx_j * normal.x + dy_j * normal.y;
            if (dist_i < 0){
                raw_contacts.push_back(vi);
            }
            if (dist_j < 0){
                raw_contacts.push_back(vj);
            }
    }
    std::vector<Vec2> unique = {};
    auto close = [epsilon] (Vec2 a, Vec2 b){
        return (std::abs(a.x - b.x) <= epsilon && std::abs(a.y - b.y) <= epsilon);
    };
    for (auto contact : raw_contacts){
        bool merged = false;
        for (auto& uc : unique){
            if (close(contact, uc)){
                uc = Vec2((uc.x  + contact.x) * 0.5, (uc.y + contact.y) * 0.5);
                merged = true;
                break;
            }

        }
        if (!merged){
            unique.push_back(contact);
        }

    }
    std::vector<Vec2> chosen;
    if (static_cast<size_t>( max_contacts) < unique.size()){
        chosen.assign(unique.begin(), unique.begin() + max_contacts);
    }
    else{
        chosen = unique;
    }
    
    return chosen;




}
void Polygon::apply_rotation(Vec2 lever_arm_r, double Jx, double Jy){
    double torque_impulse = lever_arm_r.x * Jy - lever_arm_r.y * Jx;
    double effective_inertia = inertia;
    if (torque_impulse < 0){
        effective_inertia /= 1.225;
    }
    double omega_change = torque_impulse/effective_inertia * 10;
    omega += omega_change;
}
void Polygon::update_aabb() {
    if (world_vertices.empty()) return;

    aabb.min_x = aabb.max_x = world_vertices[0].x;
    aabb.min_y = aabb.max_y = world_vertices[0].y;

    for (auto &v : world_vertices) {
        if (v.x < aabb.min_x) aabb.min_x = v.x;
        if (v.x > aabb.max_x) aabb.max_x = v.x;
        if (v.y < aabb.min_y) aabb.min_y = v.y;
        if (v.y > aabb.max_y) aabb.max_y = v.y;
    }
}
bool Polygon::aabb_overlap(const AABB& a, const AABB& b) {
    return (a.min_x <= b.max_x && a.max_x >= b.min_x &&
            a.min_y <= b.max_y && a.max_y >= b.min_y);
}
void Polygon::impulse_resolution(Vec2 MTV, Vec2 segment_edge,
    std::pair<Vec2, Vec2> segment_vertices, std::vector<Vec2> manifold){
        vx *= 0.99;
        vy *= 0.99;
        center_x += MTV.x;
        center_y += MTV.y;
        double seg_mag = std::hypot(segment_edge.x, segment_edge.y);
        double seg_nx = -segment_edge.y / seg_mag;
        double seg_ny = segment_edge.x / seg_mag;
        double to_body_x = center_x - segment_vertices.first.x;
        double to_body_y = center_y - segment_vertices.first.y;
        if (to_body_x * seg_nx + to_body_y * seg_ny < 0){
            seg_nx *= -1;
            seg_ny *= -1;
        }

        for (auto closest_point : manifold){
            double lever_arm_x = closest_point.x - center_x;
            double lever_arm_y = closest_point.y - center_y;
            double v_rot_x = -omega * lever_arm_y;
            double v_rot_y = omega * lever_arm_x;
            double v_contact_x = vx + v_rot_x;
            double v_contact_y = vy + v_rot_y;
            double vel_along_normal = v_contact_x * seg_nx + v_contact_y * seg_ny;

            if (vel_along_normal > 0){
                continue;
            }
            double epsilon = 0.001;
            double impulse_scalar;

            double v0 = 0.0; double v1 = 1.0;
            double t_ = std::max(0.0, std::min(1.0, -vel_along_normal) - ( v0/v1 - v0));

            double local_e = collision_elasticity * t_;
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
            double dot = vx * seg_nx + vy * seg_ny;
            Vec2 vn = Vec2(dot * seg_nx, dot * seg_ny);
            //tangentional velocity
            Vec2 vt = Vec2(vx - vn.x, vy - vn.y);
            double vt_mag = std::sqrt(vt.x * vt.x + vt.y * vt.y);
            if (vt_mag == 0){
                vt_mag = 1e-6;
            }
            Vec2 t = Vec2(vt.x/vt_mag, vt.y/vt_mag);
            double j_mag = (std::sqrt(Jx * Jx + Jy *Jy));
            double r_cross_t = lever_arm_x * t.y - lever_arm_y * t.x;
            
            double denom_t = (1.0/mass) + (r_cross_t * r_cross_t)/inertia ;
            
            double full_denom = denom_t;
            double static_jt_unclamped = -vt_mag/full_denom;
            double limit = mu_s * j_mag;
            double jt;
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
        

    }
void Polygon::update_world_vertices(){
    angle = std::fmod(angle, (2 * PI));
    double cos_a = std::cos(angle);
    double sin_a = std::sin(angle);
    world_vertices = {};
    for (auto lv : local_vertices){
        double dx = lv.x - local_center_x;
        double dy = lv.y - local_center_y;
        double rx = dx*cos_a-dy*sin_a;
        double ry = dx*sin_a+dy*cos_a;
        double wx = rx + center_x;
        double wy = ry + center_y;
        Vec2 actual_vert = Vec2(wx, wy);
        world_vertices.push_back(actual_vert);

    }
        // Build edges safely
    for (size_t i = 0; i < vertices; i++) {
        std::pair<Vec2, Vec2> edge = {world_vertices[i], world_vertices[(i + 1) % vertices]};
        edge_coordinates.push_back(edge);
    }

}
void Polygon::apply_force(double dt){
    vy += (gravity * 100) * dt;
}
void Polygon::apply_motion(double dt){
    center_x += vx * dt;
    angle += omega * dt;
   
    center_y += vy * dt;
    update_world_vertices();
    update_aabb();
}
void Polygon::boundary_wall(Vec2 segv1, Vec2 segv2, std::function<bool(double, double)> comparator, BoundaryType side){
    // These boundaries are only used for axis aligned, world boundaries. Do NOT use them for
    // diagonal walls or walls that objects could hit from either side, use the SAT based one for that
    // The only (major) benefit of using this is that effectively disables the effect of tunnelling.
    std::pair<Vec2, Vec2> segment_vert = {segv1, segv2};
    Vec2 segment_edge = Vec2(segv2.x - segv1.x, segv2.y - segv1.y);
    std::tuple<bool, Vec2, std::vector<Vec2>> result;

    double EPSILON = 1e-6;
    bool collided;
    Vec2 MTV;
    std::vector<Vec2> manifold;
    if (std::fabs(segment_edge.y) < EPSILON){
        
        result = y_boundaries(segment_edge, segment_vert, segv1.y, comparator, side);
    }
    else if (std::fabs(segment_edge.x) < EPSILON){
        result = x_boundaries(segment_edge, segment_vert, segv1.x, comparator, side);
    }
    std::tie(collided, MTV, manifold) = result;
    if (collided){
        
        impulse_resolution(MTV, segment_edge, segment_vert, manifold);
    }
}
void Polygon::wall(Vec2 segv1, Vec2 segv2){
    
    std::pair<Vec2, Vec2> segment_vert = {segv1, segv2};
    Vec2 segment_edge = Vec2(segv2.x - segv1.x, segv2.y - segv1.y);
    std::tuple<bool, Vec2, std::vector<Vec2>> result;
    bool collided;
    Vec2 MTV;
    std::vector<Vec2> manifold;
    double EPSILON = 1e-6;


        
        
    result = sat_algorithm_boundary(segment_edge, segment_vert);
        
    
    
    std::tie(collided, MTV, manifold) = result;
    if (collided){
        
        impulse_resolution(MTV, segment_edge, segment_vert, manifold);
        
        
        
    }

}




std::optional<std::pair<Rectangle, Rectangle>> Rectangle::fracture(double impulse, Vec2 contact_point, double fracture_threshold){
    edge_coordinates.clear();
    for (size_t i = 0; i < vertices; i++) {
        std::pair<Vec2, Vec2> edge = {world_vertices[i], world_vertices[(i + 1) % vertices]};
        edge_coordinates.push_back(edge);
    }
    double MPa = (impulse/(length* 1.0* width ));
    if (length < 12){
        return std::nullopt;
    }
    Vec2 center_bottom;
    Vec2 center_top;
    Vec2 vector_top;
    Vec2 vector_bottom;
    if (MPa > fracture_threshold) {
        std::pair<Vec2, Vec2> edge_coord;
        
        bool found_edge = false;
        for (auto edge : edge_coordinates) {
            // compute cross with the current edge
            Vec2 edge_length = Vec2(edge.first.x - edge.second.x, edge.first.y - edge.second.y);
            double edge_distance = std::sqrt(edge_length.x * edge_length.x + edge_length.y *edge_length.y);
            double tolerance = 1e-3; // or some small fraction of width
            if (std::abs(edge_distance - width) < tolerance) {
                
                continue;
            }
                Vec2 AB = Vec2(edge.second.x - edge.first.x, edge.second.y-edge.first.y);
                Vec2 AP = Vec2(contact_point.x - edge.first.x, contact_point.y - edge.first.y);
                double ab2 = AB.x * AB.x + AB.y*AB.y;
                double t = 0.0;
                if (ab2 == 0){
                    t = 0.0;
                }
                else{
                    t = std::max(0.0, std::min(1.0, (AP.x * AB.x + AP.y * AB.y)/ab2));
                }
                Vec2 closest = Vec2(edge.first.x + AB.x*t, edge.first.y + AB.y * t);

                double dx = contact_point.x - closest.x;
                double dy = contact_point.y - closest.y;
                double dist = std::sqrt(dx*dx + dy*dy);
            
            if (std::abs(dist) < 1e-1) {
                edge_coord = edge; // assign the current edge
                vector_top = Vec2(contact_point.x - edge_coord.first.x, contact_point.y - edge_coord.first.y);
                vector_bottom = Vec2(contact_point.x - edge_coord.second.x, contact_point.y - edge_coord.second.y);
                found_edge = true;
        
                center_top = Vec2(
                    (contact_point.x + edge_coord.first.x) / 2.0,
                    (contact_point.y + edge_coord.first.y) / 2.0
                );
        
                center_bottom = Vec2(
                    (contact_point.x + edge_coord.second.x) / 2.0,
                    (contact_point.y + edge_coord.second.y) / 2.0
                );
                break;
            }
        }
        if (!found_edge || length < 17) {
            
            return std::nullopt; // nothing to fracture
        }
          
        
       
        double distance_top = std::sqrt(vector_top.x*vector_top.x + vector_top.y*vector_top.y);

        
        
        double distance_bottom = std::sqrt(vector_bottom.x*vector_bottom.x + vector_bottom.y*vector_bottom.y);
        if (distance_bottom + distance_top > length + 1 || distance_bottom + distance_top < length -1){
            return std::nullopt;
        }
        if (distance_bottom < 0 || distance_top < 0){
            return std::nullopt;
        }
        Rectangle rect1 = Rectangle(4, vx, center_top.x, center_top.y, distance_top,width, omega, angle);
        
        Rectangle rect2 = Rectangle(4, vx, center_bottom.x, center_bottom.y, distance_bottom, width, -omega, angle);
        std::pair<Rectangle, Rectangle> new_rects = {rect1, rect2};
        return new_rects;
    }
    return std::nullopt;
}