#include <math.h>
#include <typeinfo>
#include <numbers>
#include <vector>
#include "polygon.h"
#include <algorithm>
#include <cassert>


std::vector<Vec2> PolygonCollisions::normal_computation(Shape& polygon){
    std::vector<Vec2> normals;
    for (auto edge : polygon.edge_coordinates){
        Vec2 vec = Vec2(edge.second.x - edge.first.x, edge.second.y - edge.first.y);
        double mag = std::sqrt(vec.x * vec.x + vec.y * vec.y);
        double nx = -vec.y/mag;
        double ny = vec.x/mag;
        
        normals.push_back(Vec2(nx, ny));

    }
    return normals;
}
std::pair<double, double> PolygonCollisions::poly_min_max(Shape& polygon, const Vec2 normal){
    std::vector<double> projections;
    for (auto vertex : polygon.world_vertices){
        double p = vertex.x * normal.x + vertex.y * normal.y;
        projections.push_back(p);
    }
    auto [min_projection, max_projection] = std::minmax_element(projections.begin(), projections.end());
    std::pair<double, double> projection = {*min_projection, *max_projection};
    return projection;

}

std::tuple<bool, Vec2, Vec2, std::vector<ContactKey>> PolygonCollisions::sat_algorithm(Shape& polygon, Shape& polygon_other){
    std::vector<Vec2> all_normals;
    std::tuple<bool, Vec2, Vec2, std::vector<ContactKey>> result;
    std::vector<Vec2> normals = normal_computation(polygon);
    std::vector<Vec2> normals_other = normal_computation(polygon_other);
    all_normals.insert(all_normals.end(), normals.begin(), normals.end());
    all_normals.insert(all_normals.end(), normals_other.begin(), normals_other.end());
    double min_overlap = std::numeric_limits<double>::infinity();
    
    Vec2 mtv_axis;
    std::vector<ContactKey> contacts;
    if (polygon.world_vertices.size() < 16 || polygon_other.world_vertices.size() < 16){
        for (auto normal : all_normals) {
            auto [poly_min, poly_max] = poly_min_max(polygon, normal);
            auto [poly_min_other, poly_max_other] = poly_min_max(polygon_other, normal);
        
            // if no overlap → no collision
            if (poly_max < poly_min_other || poly_max_other < poly_min) {
                result = {false, Vec2(0,0), Vec2(0,0), {ContactKey{-1,-1,-1,-1, Vec2(0,0)}}};
                return result;
            }
        
            // compute magnitude
            double overlap = std::min(poly_max, poly_max_other) - std::max(poly_min, poly_min_other);
        
            // determine direction based on projection ordering
            double dir = (poly_min < poly_min_other) ? 1.0 : -1.0;
        
            // new best axis?
            if (overlap < min_overlap) {
                min_overlap = overlap;
                mtv_axis = Vec2(normal.x * dir, normal.y * dir); // SIGNED axis
                if (polygon.world_vertices.size() == 3 && polygon_other.world_vertices.size() == 3){
                    std::vector<Vec2> contact_triangle = polygon_contact_points(polygon, polygon_other, mtv_axis);
                    for (auto& contact:contact_triangle){
                        contacts.push_back(ContactKey{-1,-1,-1,-1, contact});
                    }
                   
                    
                }
                else {
                    contacts = polygon_contact_points_other(polygon, polygon_other);
                }
               
            }
        }
        
        // final MTV
        Vec2 MTV = Vec2(mtv_axis.x * min_overlap, mtv_axis.y * min_overlap);
        result = {true, mtv_axis, MTV, contacts};
        return result;
    }
    else {
        double distance = std::sqrt((polygon.center_x - polygon_other.center_x) * (polygon.center_x - polygon_other.center_x) + (polygon.center_y - polygon_other.center_y) * (polygon.center_y - polygon_other.center_y));
        if (distance==0) distance = 1e-6;
        double radius1 = polygon.length/(2*std::sin(PI/polygon.world_vertices.size()));
        double radius2 = polygon_other.length/(2*std::sin(PI/polygon_other.world_vertices.size()));
        if (distance < radius1 + radius2){
            double dx = (polygon_other.center_x - polygon.center_x);
            double dy = (polygon_other.center_y - polygon.center_y);
            double nx = dx/distance;
            double ny = dy/distance;
            double penetration_mag = -distance + (radius1 + radius2);
            Vec2 penetration_vec = Vec2((-distance + (radius1 + radius2)) * nx, (-distance + (radius1 + radius2))* ny);
            double contactx = polygon.center_x + nx * (radius1 - penetration_mag/2);
            double contacty = polygon.center_y + ny * (radius1 - penetration_mag/2);
            return {true, Vec2(nx, ny), penetration_vec, {ContactKey{-1,-1,-1,-1,{Vec2(contactx, contacty)}}}};

        }

    }
    return  {false, Vec2(0,0), Vec2(0,0), {ContactKey{-1,-1,-1,-1}}};
    
}

bool on_segment(Vec2 p, Vec2 q, Vec2 r) {
    return q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
           q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y);
}

int orientation(Vec2 p, Vec2 q, Vec2 r) {
    double val = (q.y - p.y)*(r.x - q.x) - (q.x - p.x)*(r.y - q.y);
    if (fabs(val) < 1e-8) return 0; // collinear
    return (val > 0) ? 1 : 2; // clock or counterclockwise
}

bool PolygonCollisions::segments_intersect(Vec2 p1, Vec2 p2, Vec2 q1, Vec2 q2) {
    int o1 = orientation(p1, p2, q1);
    int o2 = orientation(p1, p2, q2);
    int o3 = orientation(q1, q2, p1);
    int o4 = orientation(q1, q2, p2);

    if (o1 != o2 && o3 != o4) return true; // normal intersection
    if (o1 == 0 && on_segment(p1, q1, p2)) return true;
    if (o2 == 0 && on_segment(p1, q2, p2)) return true;
    if (o3 == 0 && on_segment(q1, p1, q2)) return true;
    if (o4 == 0 && on_segment(q1, p2, q2)) return true;

    return false;
}
std::pair<int, int> PolygonCollisions::line_intersects_segment(Vec2 begin_point, Vec2 end_point, Shape& polygon){
    int num_times = 0;
    int index = -1;
    for (int  i = 0; i < polygon.edge_coordinates.size(); ++i){
        std::pair<Vec2, Vec2> edge = polygon.edge_coordinates[i];
        bool intersected = segments_intersect(begin_point, end_point, edge.first, edge.second);
        if (intersected){
            index = i;
            ++num_times;
        }
    }
    return {num_times, index};
}
std::pair<int, int> PolygonCollisions::raycaster(Vec2 point, Shape& polygon, int ray_length){
    int num_times = 0;
    int index = -1;
    Vec2 current_point = Vec2(point.x + ray_length, point.y);
    std::pair<int, int> contact = line_intersects_segment(point, current_point, polygon);
    num_times += contact.first;
    index = contact.second;
    return {num_times, index};
}
bool PolygonCollisions::point_in_polygon(const Vec2& point, const Shape& polygon) {
    bool inside = true;
    size_t n = polygon.world_vertices.size();
    for (size_t i = 0; i < n; ++i) {
        Vec2 a = polygon.world_vertices[i];
        Vec2 b = polygon.world_vertices[(i + 1) % n];
        
        // Edge vector
        Vec2 edge = Vec2(b.x - a.x, b.y - a.y);
        // Vector from a to point
        Vec2 to_point = Vec2(point.x - a.x, point.y - a.y);
        // Perpendicular / cross product (z-component)
        double cross = edge.x * to_point.y - edge.y * to_point.x;
        if (cross < -1e-8) {  // tolerance for edge-on cases
            inside = false;
            break;
        }
    }
    return inside;
}

std::vector<ContactKey> PolygonCollisions::polygon_contact_points_other(Shape& polygon, Shape& polygon_other){
    std::vector<std::tuple<Vec2, int, int>> contacts;

    // Collect contacts from polyA vertices
    for (auto v : polygon.world_vertices) {
        auto [crosses, edge_index] = raycaster(v, polygon_other);
        if (crosses % 2 == 1) {
            contacts.push_back({v, polygon.id, edge_index});
        }
    }

    // Collect contacts from polyB vertices
    for (auto v : polygon_other.world_vertices) {
        auto [crosses, edge_index] = raycaster(v, polygon);
        if (crosses % 2 == 1) {
            contacts.push_back({v, polygon.id, edge_index});
        }
    }

    std::vector<ContactKey> contact_keys;
    for (auto& [pt, poly_id, edge_index] : contacts) {
        // Each contact gets its own key
        contact_keys.push_back(ContactKey{polygon.id, polygon_other.id, -1, edge_index, pt});
    }

    return contact_keys;
}

std::vector<Vec2> PolygonCollisions::polygon_contact_points(Shape& polygon, Shape& polygon_other, Vec2 mtv_axis) {
    std::vector<Vec2> contacts;

    // Helper lambda: clip a polygon against a single edge
    auto clip_polygon_edge = [](const std::vector<Vec2>& subject, const Vec2& a, const Vec2& b) {
        std::vector<Vec2> output;
        Vec2 edge = Vec2(b.x - a.x, b.y - a.y);

        auto inside = [&](const Vec2& p) {
            Vec2 ap(p.x - a.x, p.y - a.y);
            double cross = edge.x * ap.y - edge.y * ap.x;
            return cross >= -1e-8; // same as point_in_polygon logic
        };

        auto intersect = [&](const Vec2& p1, const Vec2& p2) {
            Vec2 dp(p2.x - p1.x, p2.y - p1.y);
            Vec2 ea(a.x - b.x, a.y - b.y);
            double denom = dp.x * ea.y - dp.y * ea.x;
            if (std::abs(denom) < 1e-8) return p2; // parallel edges
            double t = ((a.x - p1.x) * ea.y - (a.y - p1.y) * ea.x) / denom;
            return Vec2(p1.x + t * dp.x, p1.y + t * dp.y);
        };

        size_t n = subject.size();
        for (size_t i = 0; i < n; ++i) {
            Vec2 curr = subject[i];
            Vec2 prev = subject[(i + n - 1) % n];

            bool curr_inside = inside(curr);
            bool prev_inside = inside(prev);

            if (curr_inside) {
                if (!prev_inside) {
                    output.push_back(intersect(prev, curr));
                }
                output.push_back(curr);
            } else if (prev_inside) {
                output.push_back(intersect(prev, curr));
            }
        }

        return output;
    };

    // Start with all vertices of polygon_other
    std::vector<Vec2> clipped_points = polygon_other.world_vertices;

    // Clip against all edges of polygon
    for (auto& edge : polygon.edge_coordinates) {
        clipped_points = clip_polygon_edge(clipped_points, edge.first, edge.second);
        if (clipped_points.empty()) break; // no contact
    }

    // Add resulting clipped points
    contacts.insert(contacts.end(), clipped_points.begin(), clipped_points.end());

    // Remove duplicates
    std::sort(contacts.begin(), contacts.end(), [](const Vec2& a, const Vec2& b){
        if (a.x != b.x) return a.x < b.x;
        return a.y < b.y;
    });
    contacts.erase(std::unique(contacts.begin(), contacts.end(), [](const Vec2& a, const Vec2& b){
        return std::abs(a.x - b.x) < 1e-6 && std::abs(a.y - b.y) < 1e-6;
    }), contacts.end());

    return contacts;
}
std::tuple<double, double, Vec2> PolygonCollisions::relative_velocity(Shape& polygon, Vec2 contact){
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
std::tuple<double, double, double, double> PolygonCollisions::impulse_resolution(std::shared_ptr<Shape>& polygon, std::shared_ptr<Shape>& polygon_other, ContactKey contact_key,Vec2 MTV, int iterations, int count, double penn_corr){
    double fraction = 1.0/iterations;
    double Jx = 0;
    double Jy = 0;
    double jtx = 0;
    double jty = 0;

    Vec2 contact = contact_key.contact_point;



    double collision_normal_len = std::sqrt(MTV.x * MTV.x + MTV.y * MTV.y);
    Vec2 normal = Vec2(MTV.x/collision_normal_len, MTV.y/collision_normal_len);
    auto [v_contact_x, v_contact_y, lever_arm] = relative_velocity(*polygon, contact);
    auto [v_contact_x_other, v_contact_y_other, lever_arm_other] = relative_velocity(*polygon_other, contact);
    double rvx = v_contact_x_other - v_contact_x;
    double rvy = v_contact_y_other - v_contact_y;
    double rel_vel = (rvx * normal.x + rvy * normal.y);
    double r_cross_n = lever_arm.x * normal.y - lever_arm.y * normal.x;
    double r_cross_n_other = lever_arm_other.x * normal.y - lever_arm_other.y * normal.x;
    double denom = (1.0/polygon->mass) + (r_cross_n * r_cross_n)/polygon->inertia;
    double denom_other = (1.0/polygon_other->mass) + (r_cross_n_other * r_cross_n_other)/polygon_other->inertia;
    //std::cout << "rel_vel: " << rel_vel << "\n";
    //
    //std::cout << "mass2: " << polygon_other->mass << ", inertia2: " << polygon_other->inertia << "\n";
    //std::cout << "r_cross_n: " << r_cross_n << ", r_cross_n_other: " << r_cross_n_other << "\n";
    //std::cout << "denom+denom_other: " << (denom + denom_other) << "\n";
    double error = std::sqrt(MTV.x * MTV.x + MTV.y * MTV.y);
    bool high_speed_collision = false;
    double bias;    
    double max_bias = 10000;
    bias = ((0.3 * error)/dt);
    
    if (rel_vel < -0.0 || (error > 0.00)){

        double e = restitution;
        
        float threshold = -1.0;
        float range = 1.0; // the range over which e goes from its max to 0
        float max_e = restitution; // or whatever e normally is
        
        if (rel_vel > threshold) {
            float t = (rel_vel - threshold) / range; // normalize between 0 and 1
            if (t > 1.0) t = 1.0; // clamp
            e = max_e * (1.0 - t); // smooth decrease from max_e to 0
        }
        if (error < 0.00){
            bias = 0.0;
        }
        if (std::abs(bias) > max_bias){
            bias = std::clamp(bias, -max_bias, max_bias);
            

        }
        double impulse_scalar = -( (1 + e) * rel_vel - bias );
        impulse_scalar /= (denom + denom_other);
        
        double jn_delta = impulse_scalar;
        double jn_new = std::max(jn_delta, 0.0);
        double jn_to_apply;
        jn_to_apply = jn_new;
        Jx = jn_to_apply * normal.x;
        Jy = jn_to_apply * normal.y;
    
        polygon->vx -= Jx/ polygon->mass;
        polygon->vy -= Jy/ polygon->mass;
        polygon->apply_rotation(lever_arm, -Jx, -Jy);

        polygon_other->vx += Jx/ polygon_other->mass;
        polygon_other->vy += Jy/ polygon_other->mass;
        polygon_other->apply_rotation(lever_arm_other, Jx, Jy);


        auto [v_contact_x, v_contact_y, lever_arm] = relative_velocity(*polygon, contact);
        auto [v_contact_x_other, v_contact_y_other, lever_arm_other] = relative_velocity(*polygon_other, contact);
        rvx = v_contact_x_other - v_contact_x;
        rvy = v_contact_y_other - v_contact_y;

        Vec2 tangential_direction = Vec2(-normal.y, normal.x);
        

        double mu_s =0.55;
        double mu_k =0.4;
        double vt_rel = rvx * tangential_direction.x + rvy * tangential_direction.y;
        double j_mag = std::abs(jn_to_apply);

        double r_cross_t = lever_arm.x * tangential_direction.y - lever_arm.y * tangential_direction.x;
        double r_cross_t_other = lever_arm_other.x * tangential_direction.y - lever_arm_other.y * tangential_direction.x;
        double denom_t = (1.0/polygon->mass) + (r_cross_t * r_cross_t)/polygon->inertia;
        double denom_other_t = (1.0/polygon_other->mass) + (r_cross_t_other * r_cross_t_other)/polygon_other->inertia;
        double full_denom = denom_t + denom_other_t;
        double static_jt_unclamped = -vt_rel/full_denom;
        
        double limit = mu_s * j_mag;
        double jt;
        double jt_new;
        jt_new = static_jt_unclamped;
        if (abs(static_jt_unclamped) < limit)
            jt = static_jt_unclamped;
        else
            jt = std::clamp(static_jt_unclamped, -limit, limit);
        double jt_delta = jt ;
        
        if (count == 99){
            std::cout << "VREL: " << vt_rel << "\n";
        }
        assert(abs(std::sqrt(tangential_direction.x * tangential_direction.x + tangential_direction.y * tangential_direction.y) - 1.0) < 1e-6);
        jtx = jt_delta * tangential_direction.x;
        jty = jt_delta * tangential_direction.y;
        
        polygon->vx -= jtx/ polygon->mass;
        polygon->vy -= jty/ polygon->mass;
        polygon->apply_rotation(lever_arm, -jtx, -jty);
    
        polygon_other->vx += jtx/ polygon_other->mass;
        polygon_other->vy += jty/ polygon_other->mass;
        polygon_other->apply_rotation(lever_arm_other, jtx, jty);
    


       
        auto &total = impulse_cache[contact_key];

     
        
        
        

      
    
    }


   
    

        
    // polygon_other->angle += correction_mag * (r_cross_n_other / polygon_other->inertia) * (denom / inv_mass_sum);
        double J_mag = std::sqrt(Jx * Jx + Jy * Jy);

        double impulse_scalar = -(1 + restitution) * rel_vel-bias;
        if (bias > max_bias){{
            Jx = 0;
            Jy = 0;
        }}
        Jx = impulse_scalar * normal.x;
        Jy = impulse_scalar * normal.y;
    
        return {Jx, Jy, jtx, jty};
}
void PolygonCollisions::positional_correction(std::shared_ptr<Shape>& polygon, std::shared_ptr<Shape>& polygon_other, Vec2 MTV, double fraction, double penn_corr){
    if (std::sqrt(MTV.x * MTV.x + MTV.y * MTV.y) < 0.01){
        return;
    }
    double vx_corr = (MTV.x * (fraction / penn_corr));
    double vy_corr = (MTV.y * (fraction / penn_corr));

    polygon->vx -= vx_corr/2;
    polygon->vy -= vy_corr/2;

    polygon_other->vx += vx_corr/2;
    polygon_other->vy += vy_corr/2;
}
std::optional<std::tuple<std::pair<Rectangle, Rectangle>, int>> PolygonCollisions::fractured(std::shared_ptr<Shape> polygon, std::shared_ptr<Shape> polygon_other, double impulse_magnitide, Vec2 contact){
    auto p1 = polygon.get();
    auto p2 = polygon_other.get();
    // Check if p1 is a Rectangle
    if (auto rect = dynamic_cast<Rectangle*>(p1)) {
        // Now rect is a Rectangle*, safe to call Rectangle-specific methods
        std::optional<std::pair<Rectangle, Rectangle>> value = rect->fracture(impulse_magnitide, contact);
        
        

        if (value) {
            std::optional<std::tuple<std::pair<Rectangle, Rectangle>, int>> value_type = 
                std::make_optional(std::make_tuple(*value, 0));
                return value_type;
        }
    }

    // Same for p2
    if (auto rect = dynamic_cast<Rectangle*>(p2)) {
        std::optional<std::pair<Rectangle, Rectangle>> value_other = rect->fracture(impulse_magnitide, contact);
        if (value_other) {
            std::optional<std::tuple<std::pair<Rectangle, Rectangle>, int>> value_type = 
            std::make_optional(std::make_tuple(*value_other, 1));
            return value_type;
            
            
        }
    }
    return std::nullopt;
}

void PolygonCollisions::apply_impulse(std::shared_ptr<Shape>& polygon, std::shared_ptr<Shape>& polygon_other, Vec2 contact, std::pair<double, double> impulse, std::pair<double, double> friction){
    double Jx = impulse.first;
    double Jy = impulse.second;

    double Jtx = friction.first;
    double Jty = friction.second;
    Vec2 lever_arm = Vec2(contact.x - polygon->center_x, contact.y - polygon->center_y);
    Vec2 lever_arm_other = Vec2(contact.x - polygon_other->center_x, contact.y - polygon_other->center_y);
    polygon->vx -= Jx/polygon->mass;
    polygon->vy -= Jy/polygon->mass;
    polygon->apply_rotation(lever_arm, -Jx, -Jy);

    polygon_other->vx += Jx/polygon_other->mass;
    polygon_other->vy += Jy/polygon_other->mass;
    polygon_other->apply_rotation(lever_arm_other, Jx, Jy);

    polygon->vx -= Jtx/polygon->mass;
    polygon->vy -= Jty/polygon->mass;
    polygon->apply_rotation(lever_arm, -Jtx, -Jty);

    polygon_other->vx += Jtx/polygon_other->mass;
    polygon_other->vy += Jty/polygon_other->mass;
    polygon_other->apply_rotation(lever_arm_other, Jtx, Jty);
}





OptionalFractureVector PolygonCollisions::polygon_collisions(std::vector<std::shared_ptr<Shape>>& polygons){
    int iterations = 25;
    std::vector<std::pair<std::shared_ptr<Shape>, std::shared_ptr<Shape>>> broad_phase;
    broad_phase.reserve(polygons.size() * (polygons.size() - 1) / 2);
    
    for (size_t i = 0; i < polygons.size(); i++) {
        for (size_t j = i + 1; j < polygons.size(); j++) {
            auto poly1 = polygons[i];
            auto poly2 = polygons[j];
    
            if (poly1->aabb_overlap(poly1->aabb, poly2->aabb)) {
                broad_phase.push_back({poly1, poly2});
            }
        }
    }
    std::vector<std::shared_ptr<Shape>> to_remove;
    std::vector<std::shared_ptr<Shape>> to_add;
    FractureVector new_fractures;


    
    for (auto& [poly1, poly2]: broad_phase){
        // Always get polygons, whether a shape is simple or compound
        std::vector<std::tuple<bool, Vec2, Vec2, std::vector<ContactKey>>> results;
        bool collided_poly = false;
        bool collided = false;
        Vec2 MTV;
        
        std::vector<ContactKey> manifolds;
        std::vector<Vec2> MTVs;
        Vec2 mtv_axis;
        Vec2 deepest_axis;
        std::vector<ContactKey> combined_manifold;
        double max_penetration = 0.0;
        auto& polys1 = poly1->get_polygons();
        auto& polys2 = poly2->get_polygons();
        for (auto& p1_ptr : polys1) {
            for (auto& p2_ptr : polys2) {
        
                // Run SAT
                 
                    
        
                results.emplace_back(sat_algorithm(*p1_ptr, *p2_ptr));
            }
        }
                

                //if (is_rect_rect && (p1_is_rect && !poly1->allow_motion || p2_is_rect && !poly2->allow_motion)){
                //    continue;
                        //}
            
        

       
        double bestPenSq = -std::numeric_limits<double>::infinity();
        Vec2 bestMTV {0.0, 0.0};
        Vec2 deepest_MTV {0.0, 0.0};
        Vec2 bestAxis {0.0, 0.0};
        std::vector<ContactKey> contact_info_to_use;
        std::vector<ContactKey> bestManifolds;
        const double EPS = 1e-6;
        Vec2 finalMTV;
        for (auto& result : results) {
            bool collided_poly;
            Vec2 mtv_axis;
            Vec2 MTV;
            std::vector<ContactKey> contact_info;
            std::tie(collided_poly, mtv_axis, MTV, contact_info) = result;
            manifolds = contact_info;
            if (!collided_poly) continue;
            if (std::isnan(MTV.x) || std::isnan(MTV.y)) continue;
        
            double penSq = MTV.x*MTV.x + MTV.y*MTV.y;
            if (penSq > bestPenSq) {
                // This is the new winner
                bestPenSq = penSq;
                bestMTV = MTV;
                bestAxis = mtv_axis;       // NOTE: make sure sat returns the *corrected* axis
                bestManifolds = manifolds; // take the manifolds that came with the winner
                contact_info_to_use = contact_info;
            } 
            else{
                deepest_MTV = MTV;
            }
                // optional: if penetration approximately equal, you can append manifolds
                // to produce multi-contact manifold for contact clipping
            bestManifolds.insert(bestManifolds.end(), manifolds.begin(), manifolds.end());
            
        }
        
        // After loop: if collision found, orient and normalize properly
        if ((bestPenSq > 0 && bestPenSq != -std::numeric_limits<double>::infinity())) {
            double depth = std::sqrt(bestPenSq);
        
            // sanity: normalize axis consistently using parent centroids
            
            // if sat returned a zero/stale axis, restore from bestMTV:
            
                

        
            finalMTV = Vec2(bestAxis.x * depth, bestAxis.y * depth);
        
            // Use bestManifolds (optionally filter/transform them to parent space)
            combined_manifold = bestManifolds;
            // set whatever fields your collision response expects:
            deepest_axis = bestAxis;
            max_penetration = depth;
            MTVs.clear();
            MTVs.push_back(finalMTV);
            collided = true;
        }

        if (collided) {
            
            if (!poly1->collided || !poly2->collided){
                
                poly1->collided = poly2->collided = true;
                
                

            }   
        
        
        






            
            for (auto contact: combined_manifold){
                
                
                if (impulse_cache.find(contact)!= impulse_cache.end()){
                    
                    std::pair<std::pair<double, double>, std::pair<double, double>> cached = impulse_cache.at(contact);
                    double factor = 1;
                    //std::cout << "ContactKey { "
                    //<< "Jx: " << cached.first.first << ", "
                    //<< "Jy: " <<cached.first.second << ", "
                    //<< "Jtx: " << cached.second.first << ", "
                  //  << "Jty: " << cached.second.second
                   // << " }" << std::endl;
                    //apply_impulse(poly1, poly2, contact.contact_point, {cached.first.first * factor, cached.first.second * factor}, {cached.second.first * factor, cached.second.second * factor});
                }
                
            }
           
            std::map<ContactKey, std::pair<std::pair<double, double>, std::pair<double, double>>> frame_impulses;
            bool fracture = false;
            
            for (int _ = 0; _ < iterations; _++){
                    if (fracture){
                        fracture = false;
                        break;
                    } 
                    for (auto contact: combined_manifold){
                        auto[Jx, Jy, jtx, jty] = impulse_resolution(poly1, poly2, contact, finalMTV, iterations, _);
                        double mag = std::sqrt(Jx * Jx + Jy * Jy);
                        
       
                        std::optional<std::tuple<std::pair<Rectangle, Rectangle>, int>> objects = fractured(poly1, poly2, mag, contact.contact_point);
    
                        if (objects && !poly1->static_body && !poly2->static_body){
                            
                            std::tuple<std::pair<Rectangle, Rectangle>, int> broken = *objects;
                            int which_polygon = std::get<1>(broken);
                            std::pair<Rectangle, Rectangle> new_rects = std::get<0>(broken);
    
                            new_rects.first.allow_motion = true;
                            new_rects.first.collided = true;
    
                            new_rects.second.allow_motion = true;
                            new_rects.second.collided = true;
                            auto frac_rect1 = std::make_shared<Rectangle>(new_rects.first);
                            
                            auto frac_rect2 = std::make_shared<Rectangle>(new_rects.second);
                            
                            to_add.push_back(frac_rect1);
                            to_add.push_back(frac_rect2);
    
                            if (which_polygon == 0){
                                auto result_tuple = std::make_tuple(std::make_pair(frac_rect1, frac_rect2), poly1);
                                new_fractures.push_back(result_tuple);
    
                                to_remove.push_back(poly1);
                                
                            }
                            else if (which_polygon == 1){
                                auto result_tuple_2 = std::make_tuple(std::make_pair(frac_rect1, frac_rect2), poly2);
                                new_fractures.push_back(result_tuple_2);
                                to_remove.push_back(poly2);
                                
                            }
                            fracture = true;
                            break;
    
                        }
                        
                        double velocity_magnitude = std::sqrt(poly1->vx * poly1->vx + poly1->vy * poly1->vy + ((poly1->aabb.max_y - poly1->aabb.min_y)/2 * poly1->omega) * ((poly1->aabb.max_y - poly1->aabb.min_y)/2 * poly1->omega));
                        
                        
                        if (velocity_magnitude > 0.2 && !poly1->static_body && !poly2->static_body) {
                            poly1->sleep = false;
                            poly2->sleep = false;
                            poly1->num_times_sleepy = 0;
                            poly2->num_times_sleepy = 0;
                            poly1->gravity = 9.8;
                            poly2->gravity = 9.8;
                        }
    

                       // std::cout << "Jx: " << Jx << " Jy: " << Jy << "\n";
    
                        
                    }
    

                    
                   
                }
                
                auto p1 = poly1.get();
                auto p2 = poly2.get();
                
                if (dynamic_cast<Polygon*>(p1) && dynamic_cast<Polygon*>(p2)){
                   // positional_correction(poly1, poly2, bestMTV, 1.0, 1.0/(combined_manifold.size() *748/35));
                }
                else if (dynamic_cast<Polygon*>(p1) || dynamic_cast<Polygon*>(p2)){
                    //positional_correction(poly1, poly2, bestMTV, 1.0, 1.0/(combined_manifold.size() *350/35));
                }
                else{
                    //positional_correction(poly1, poly2, bestMTV, 1.0);
                }

                    
                   



                   
            
        }
    }
    for (auto poly:to_remove){
        poly->remove = true;
    }
    for (auto poly:to_add){
        polygons.push_back(poly);
    }
    if (!new_fractures.empty()){
        return std::make_optional(new_fractures);;
    }
    
    return std::nullopt;

}




