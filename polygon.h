#include <algorithm>
#include <numbers>
#include <math.h>
#include <vector>
#include <utility>
#include <map>
#include <memory> 
#include <iostream>
#include <functional>
#include <optional>
#include <ranges>
#include <unordered_set>
const double PI = 3.14159265358979323846;
inline double dt = 1.0/460;
#define CHECK_FINITE(v, msg) \
    if (!(std::isfinite(v))) throw std::runtime_error(msg)

#define CHECK_NONZERO(v, eps, msg) \
    if (std::abs(v) < (eps) || !std::isfinite(v)) throw std::runtime_error(msg)
inline double radians(double degrees) {
    return degrees * (PI / 180.0);
}

struct Vec2 {
    double x, y;
    Vec2(double x_=0, double y_=0) : x(x_), y(y_) {}
};
struct AABB {
    double min_x, max_x;
    double min_y, max_y;
};
struct CollisionResult {
    bool hit;
    Vec2 mtv_axis;
    Vec2 mtv;
    std::vector<Vec2> contacts;
};
struct ContactKey {
    int polyA_id;
    int polyB_id;
    int edgeA_index;
    int edgeB_index;
    Vec2 contact_point;
    
       // For std::map:
       bool operator<(const ContactKey& other) const {
        return std::tie(polyA_id, polyB_id, edgeA_index, edgeB_index)
             < std::tie(other.polyA_id, other.polyB_id, other.edgeA_index, other.edgeB_index);
    }

    // For unordered_map (hash required):
    
};
enum class BoundaryType { Left, Right };
struct Shape {
    virtual ~Shape() = default;
    inline static int next_id = 0;
    bool static_body;
    double vx;
    double vy;
    double center_x;
    double center_y;
    double length;
    double omega;
    double angle;
    double gravity;
    double mass;
    double elasticity;
    double collision_elasticity;
    bool allow_motion;
    bool collision_frame_handled;
    bool draw_animation;
    int num_times_sleepy;
    bool collided = false;
    bool remove = false;
    bool sleep = false;
    AABB aabb;
    double inertia;
    std::vector<Vec2> world_vertices;
    double local_center_x;
    double local_center_y;
    std::vector<Vec2> local_vertices;
    std::unordered_set<int> ignore_collisions;

    std::string name = "";
    std::vector<std::pair<Vec2, Vec2>> edge_coordinates;
    int id;
    bool operator==(const Shape& other) const {
        return id == other.id;}


    // Constructor
    virtual bool aabb_overlap(const AABB& a, const AABB& b) = 0;
    virtual void apply_rotation(Vec2 lever_arm_r, double Jx, double Jy) = 0;
    virtual const std::vector<std::shared_ptr<Shape>>& get_polygons() const = 0;
    Shape(double vx_=25, double center_x_=400, double center_y_=0,
          double length_=110, double omega_=3, double angle_=0, double mass_=25, bool static_=false)
        : vx(vx_), vy(0), center_x(center_x_), local_center_x(0), local_center_y(0), center_y(center_y_),
          length(length_), omega(omega_), angle(angle_),
          gravity(9.8), mass(mass_),
          elasticity(0.5), collision_elasticity(0.5),
          allow_motion(true), collision_frame_handled(false),
          draw_animation(false), num_times_sleepy(0),
          edge_coordinates(), static_body(static_)
    {
        if (static_body){
            
            gravity = 0;
            std::cout << "gravity: " << gravity << "\n";
            mass = 1e10;
        }
        id = next_id++;
        
    }
};


class Polygon : public Shape, public std::enable_shared_from_this<Polygon> {
    public:
    friend class CompoundShape;
    
    public:
        const std::vector<std::shared_ptr<Shape>>& get_polygons() const override {
            return cached_self_vector;
        }
        
        bool aabb_overlap(const AABB& a, const AABB& b) override;
        std::tuple<bool, Vec2, std::vector<Vec2>> sat_algorithm_boundary(Vec2 segment_edge, 
        std::pair<Vec2, Vec2> segment_vertices);
        void boundary_wall(Vec2 segv1, Vec2 segv2, std::function<bool(double, double)> comparator, BoundaryType side);   
        void wall(Vec2 segv1, Vec2 segv2);
        void clamp_motion();
        bool is_flat(double angle, std::vector<double> flat_angles, double tolerance=0.001);
        void put_to_sleep();
        
        void impulse_resolution(Vec2 MTV, Vec2 segment_edge,
            std::pair<Vec2, Vec2> segment_vertices, std::vector<Vec2> manifold);
        
        void update_world_vertices();
        void apply_motion(double dt);

        void apply_force(double dt);
        void apply_rotation(Vec2 lever_arm_r, double Jx, double Jy) override;
        int vertices;
        
        
        
        void update_aabb();
       
        void initialize_self_vector() {
            cached_self_vector.clear();
            cached_self_vector.push_back(shared_from_this());
        }
    


        Polygon(int vertices_ = 4, double vx_ = 0, double center_x_ = 400, 
            double center_y_ = 0, double length_ = 110,
            double omega_ = -3, double angle_ = 0, double mass_ = 25, bool static_ = false)
        : vertices(vertices_), Shape(vx_, center_x_, center_y_,
                                     length_, omega_, angle_, mass_, static_)
    {
        local_vertices = calculate_vertices_and_inertia();
        update_world_vertices();
    
        if (vertices < 3 || world_vertices.size() < 3 || local_vertices.size() < 3 ||
            inertia <= 0 || std::isnan(inertia) || mass <= 0 ||
            std::isnan(angle))
        {
            throw std::runtime_error("Polygon creation failed.");
        }
    }
    protected:
        virtual std::vector<Vec2>  calculate_vertices_and_inertia() ;

        
    private:
    // Avoid allocating repeatedly
    std::vector<std::shared_ptr<Shape>> cached_self_vector; 
    std::tuple<bool, Vec2, std::vector<Vec2>> x_boundaries(Vec2 segment_edge, 
        std::pair<Vec2, Vec2> segment_vertices,    double boundary,
        std::function<bool(double, double)> comparator, BoundaryType side);

        std::tuple<bool, Vec2, std::vector<Vec2>> y_boundaries(Vec2 segment_edge, 
            std::pair<Vec2, Vec2> segment_vertices,    double boundary,
            std::function<bool(double, double)> comparator, BoundaryType side);

        
        
        std::pair<double, double> compute_line_segment_projection(std::pair<Vec2, Vec2> segment_vertices, Vec2 normal);
        Vec2 closest_point_on_segment(Vec2 seg_vert_1,
        Vec2 seg_vert_2, Vec2 P);
        std::vector<Vec2> support_vertices(Vec2 seg_vert_1,
            Vec2 seg_vert_2, Vec2 seg_normal, std::vector<Vec2> edge_coordinates);
        std::vector<Vec2> build_contact_manifold(Vec2 seg_a, Vec2 seg_b,
            Vec2 seg_normal, std::vector<std::pair<Vec2, Vec2>> polygon_edges, int max_contacts, double tolerance = 1e-6);
        

};
struct RectState {
    Vec2 pos;
    double vx;
    double vy;
    double omega;
    double angle;
};
class Rectangle : public Polygon {
    public:
        int fracture_depth = 0;
        double width;
        bool aabb_overlap(const AABB& a, const AABB& b) override;
        void apply_rotation(Vec2 lever_arm_r, double Jx, double Jy) override;
        std::vector<Vec2> calculate_vertices_and_inertia() override;
        std::optional<std::pair<Rectangle, Rectangle>> fracture(double impulse, Vec2 contact_point, double fracture_threshold=250000);
        Rectangle(int vertices_ = 4, double vx_ = 0, double center_x_ = 400, 
            double center_y_ = 0, double length_ = 110, double width_ = 70,
            double omega_ = 3, double angle_ = (0), double mass_ = 25, bool static_ = false)
            : Polygon(4, vx_, center_x_, center_y_,
                length_, omega_, angle_,
                mass_, static_), width(width_){
                    
                    center_x = center_x_;
                    center_y = center_y_;
                    angle = angle_;
                    local_vertices = calculate_vertices_and_inertia();
                    update_world_vertices();
                    update_aabb();


                    if (vertices < 3 || world_vertices.size() < 3 || local_vertices.size() < 3 || inertia <=0 || std::isnan(inertia) || mass <= 0
                    || std::isnan(angle)){
                        throw std::runtime_error("Something went very wrong with polygon creation.");
                    }
                };

};
class RightTriangle : public Polygon {
    public:
        double width;
        std::vector<Vec2> calculate_vertices_and_inertia() override;
        bool aabb_overlap(const AABB& a, const AABB& b) override;
        void apply_rotation(Vec2 lever_arm_r, double Jx, double Jy) override;
        RightTriangle(double vx_ = 0, double center_x_ = 40, 
            double center_y_ = 0, double equal_side = 11, double unique_side = 7,
            double omega_ = 3, double angle_ = (0), double mass_ = 25)
            : Polygon(3, vx_, center_x_, center_y_,
                equal_side, omega_, angle_,
                mass_, false), width(unique_side){
                    local_center_x = center_x_;
                    local_center_y = center_y_;
                    angle = angle_;
                    local_vertices = calculate_vertices_and_inertia();
                    update_world_vertices();
                    update_aabb();

                    if (vertices < 3 || world_vertices.size() < 3 || local_vertices.size() < 3 || inertia <=0 || std::isnan(inertia) || mass <= 0
                    || std::isnan(angle)){
                        throw std::runtime_error("Something went very wrong with polygon creation.");
                    }
                };

};


class Joint {
    public:
        virtual std::vector<std::shared_ptr<Shape>> get_bodies() = 0;
        virtual void impulse_resolution() = 0;
        virtual ~Joint() = default;
    };
struct PendulumJoint : public Joint {
    Vec2 body_anchor;
    
    Vec2 world_anchor;
    double rope_length;
    Vec2 initial_components;
    std::shared_ptr<Shape> rigid_body;
    std::vector<std::shared_ptr<Shape>> get_bodies() override;
    void impulse_resolution();
    Vec2 update_local_anchor() const;
    double getRopeLength(Vec2 body_to_world_anchor) const {
        
        return std::sqrt(
            (world_anchor.x - body_to_world_anchor.x) * (world_anchor.x - body_to_world_anchor.x) +
            (world_anchor.y - body_to_world_anchor.y) * (world_anchor.y - body_to_world_anchor.y)
        );
    }
    
    PendulumJoint(Vec2 local, Vec2 world, std::shared_ptr<Shape> body)
        : body_anchor(local), world_anchor(world), rigid_body(body){
            Vec2 body_to_world = update_local_anchor();
            rope_length = getRopeLength(body_to_world);
            
        }
    
    
    
};
inline std::vector<std::shared_ptr<Shape>> PendulumJoint::get_bodies()  {
    return { rigid_body };
}

struct DynamicJoint : public Joint {
    Vec2 body_anchor1;
    
    Vec2 body_anchor2;
    double rope_length;
    Vec2 initial_components;
    std::shared_ptr<Shape> rigid_body1;
    std::shared_ptr<Shape> rigid_body2;
    void impulse_resolution();
    std::vector<std::shared_ptr<Shape>> get_bodies() override;
    std::tuple<double, double, Vec2> relative_velocity(Shape& polygon, Vec2 contact);
    Vec2 update_local_anchor(Vec2 anchor, std::shared_ptr<Shape> shape) const;
    
    double getRopeLength(Vec2 body_to_world_anchor1, Vec2 body_to_world_anchor2) const {
        
        return std::sqrt(
            (body_to_world_anchor2.x - body_to_world_anchor1.x) * (body_to_world_anchor2.x - body_to_world_anchor1.x) +
            (body_to_world_anchor2.y - body_to_world_anchor1.y) * (body_to_world_anchor2.y - body_to_world_anchor1.y)
        );
    }
    
    DynamicJoint(Vec2 world1, Vec2 world2, std::shared_ptr<Shape> body, std::shared_ptr<Shape> body_other)
        : body_anchor1(world1), body_anchor2(world2), rigid_body1(body), rigid_body2(body_other){
            Vec2 body_to_world1 = update_local_anchor(body_anchor1, rigid_body1);
            Vec2 body_to_world2 = update_local_anchor(body_anchor2, rigid_body2);
            rope_length = getRopeLength(body_to_world1, body_to_world2);
            
        }
    
    
    
};

struct PivotJoint : public Joint {
    Vec2 body_anchor1;
    
    Vec2 body_anchor2;
    double rope_length;
    Vec2 initial_components;
    std::shared_ptr<Shape> rigid_body1;
    std::shared_ptr<Shape> rigid_body2;
    void impulse_resolution();
    std::vector<std::shared_ptr<Shape>> get_bodies() override;
    std::tuple<double, double, Vec2> relative_velocity(Shape& polygon, Vec2 contact);
    Vec2 update_local_anchor(Vec2 anchor, std::shared_ptr<Shape> shape) const;
    

    PivotJoint(Vec2 world1, Vec2 world2, std::shared_ptr<Shape> body, std::shared_ptr<Shape> body_other)
        : body_anchor1(world1), body_anchor2(world2), rigid_body1(body), rigid_body2(body_other){
            Vec2 body_to_world1 = update_local_anchor(body_anchor1, rigid_body1);
            Vec2 body_to_world2 = update_local_anchor(body_anchor2, rigid_body2);
            
            
        }
    
    
    
};
inline std::vector<std::shared_ptr<Shape>> DynamicJoint::get_bodies()  {
    return { rigid_body1, rigid_body2};
}
inline std::vector<std::shared_ptr<Shape>> PivotJoint::get_bodies()  {
    return { rigid_body1, rigid_body2};
}
struct SoftJoint : public Joint {
    Vec2 body_anchor1;
    double k;
    Vec2 body_anchor2;
    double rope_length;
    Vec2 initial_components;
    std::shared_ptr<Shape> rigid_body1;
    std::shared_ptr<Shape> rigid_body2;
    std::vector<std::shared_ptr<Shape>> get_bodies() override;
    void impulse_resolution();
    std::tuple<double, double, Vec2> relative_velocity(Shape& polygon, Vec2 contact);
    Vec2 update_local_anchor(Vec2 anchor, std::shared_ptr<Shape> shape) const;
    
    double getRopeLength(Vec2 body_to_world_anchor1, Vec2 body_to_world_anchor2) const {
        
        return std::sqrt(
            (body_to_world_anchor2.x - body_to_world_anchor1.x) * (body_to_world_anchor2.x - body_to_world_anchor1.x) +
            (body_to_world_anchor2.y - body_to_world_anchor1.y) * (body_to_world_anchor2.y - body_to_world_anchor1.y)
        );
    }
    
    SoftJoint(Vec2 world1, Vec2 world2, std::shared_ptr<Shape> body, std::shared_ptr<Shape> body_other, double k_)
        : body_anchor1(world1), body_anchor2(world2), rigid_body1(body), rigid_body2(body_other), k(k_){
            Vec2 body_to_world1 = update_local_anchor(body_anchor1, rigid_body1);
            Vec2 body_to_world2 = update_local_anchor(body_anchor2, rigid_body2);
            rope_length = getRopeLength(body_to_world1, body_to_world2);
            
        }
    
    
    
};


inline std::vector<std::shared_ptr<Shape>> SoftJoint::get_bodies()  {
    return { rigid_body1, rigid_body2};
}

class PendulumSolver {
    public:
        std::vector<std::shared_ptr<Joint>> joints;
        void add_joint(std::shared_ptr<Joint> joint);
        
        void remove_joint(std::shared_ptr<Joint> joint);
        inline void remove_rigid_body(std::shared_ptr<Shape> rigid_body);
        
        void iterative_solver();
};
inline void PendulumSolver::add_joint(std::shared_ptr<Joint> joint) {
    joints.push_back(joint);
}
inline void PendulumSolver::remove_joint(std::shared_ptr<Joint> joint) {
    
    joints.erase(std::remove(joints.begin(), joints.end(), joint), joints.end());
}

inline void PendulumSolver::remove_rigid_body(std::shared_ptr<Shape> rigid_body) {
    std::vector<std::shared_ptr<Joint>> to_remove;
    for (auto& joint:joints){
        std::vector<std::shared_ptr<Shape>> bodies = joint->get_bodies();
        for (auto& body:bodies)
        if (rigid_body == body){
            to_remove.emplace_back(joint);
        }
    }
    for (auto& joint:to_remove){
        joints.erase(std::remove(joints.begin(), joints.end(), joint), joints.end());
    }
    
}

class CompoundShape : public Shape {
    public:
        
        std::vector<std::shared_ptr<Shape>> child_polygons;
        const std::vector<std::shared_ptr<Shape>>& get_polygons() const override {
            return child_polygons;
        }
        bool aabb_overlap(const AABB& a, const AABB& b) override;
        void boundary_wall(Vec2 segv1, Vec2 segv2, std::function<bool(double, double)> comparator, BoundaryType side);   
        void wall(Vec2 segv1, Vec2 segv2);
        void clamp_motion();
        bool is_flat(double angle, std::vector<double> flat_angles, double tolerance=0.001);
        void put_to_sleep();
        
        void impulse_resolution(Vec2 MTV, Vec2 segment_edge,
            std::pair<Vec2, Vec2> segment_vertices, std::vector<Vec2> manifold);
        void update_world_vertices();
        void apply_motion(double dt);

        void apply_force(double dt);
        void apply_rotation(Vec2 lever_arm_r, double Jx, double Jy) override;
        const int vertices;

        void update_aabb();
       
        
        bool sleep = false;

        bool operator==(const CompoundShape& other) const {
            return id == other.id;
        }
        CompoundShape(std::vector<std::shared_ptr<Shape>> children, int total_vertices, double vx_ = 0, double omega_ = -3, double angle_ = 0, bool static_ = false)
        : Shape(vx_, 0, 0, 0, 0, angle_, 0, static_),  // temporary mass 0, will update
            child_polygons(children),
            vertices(total_vertices)
            
        {
            inertia = 0;
            std::cout << "Static body: " << static_body << "\n";
            for (auto& child : child_polygons){
                //std::cout << "Local inertia child: " << child->inertia << ",  " << child->inertia << "\n";
                for (auto vert:child->local_vertices){
                    //std::cout << vert.x << "  " << vert.y << ", ";
                }
                
                
                mass += child->mass;
                local_center_x += child->mass * child->local_center_x;
                local_center_y += child->mass * child->local_center_y;
            }
            //std::cout << "Mass: " << mass << "\n";
            local_center_x /= mass;
            local_center_y /= mass;
           // std::cout << "Local centers: " << local_center_x << ",  " << local_center_y << "\n";
            
            for (auto& child : child_polygons){
                if (std::isnan(local_center_x)) local_center_x = 0;
                if (std::isnan(local_center_y)) local_center_y = 0;
                double dx = child->local_center_x - local_center_x;
                double dy = child->local_center_y - local_center_y;
                if (std::abs(dx) > 1e6 || std::abs(dy) > 1e6) {
                    std::cerr << "Warning: suspicious dx/dy, child id: " << child->id << "\n";
                }
        
                inertia += (child->inertia) + child->mass * (dx * dx + dy * dy);
            }
            
          //  std::cout << " inertia total: " << inertia << "\n";
            this->vx = vx_;
            this->omega = omega_;
            this->angle = angle_;
            this->center_x = local_center_x;
            this->center_y = local_center_y;

            update_world_vertices();

        }
    protected:
        
            

           
        };
struct SafeFloat {
    double value;
    std::string name;

    // Constructors
    SafeFloat(double v = 0.0, const std::string& n = "") : value(v), name(n) {
        check(__func__); // OK: inside constructor body
    }

    // Function to check for NaN/Inf
    void check(const char* func_name) const {
        if (std::isnan(value))
            std::cerr << "[NaN Detected] Variable '" << name << "' in function '" << func_name << "'\n";
        if (std::isinf(value))
            std::cerr << "[Inf Detected] Variable '" << name << "' in function '" << func_name << "'\n";
    }

    // Set or update variable name
    SafeFloat& setName(const std::string& n, const char* func_name) {
        name = n;
        check(func_name);
        return *this;
    }
    
    // Optional helper that uses current function automatically
    SafeFloat& setName(const std::string& n) {
        return setName(n, __func__); // __func__ is valid here inside a function body
    }
    // Operator overloads
    SafeFloat operator+(const SafeFloat& other) const { 
        SafeFloat res(value + other.value, name.empty() ? other.name : name);
        res.check(__func__);
        return res; 
    }

    SafeFloat operator-(const SafeFloat& other) const { 
        SafeFloat res(value - other.value, name.empty() ? other.name : name);
        res.check(__func__);
        return res; 
    }

    SafeFloat operator*(const SafeFloat& other) const { 
        SafeFloat res(value * other.value, name.empty() ? other.name : name);
        res.check(__func__);
        return res; 
    }

    SafeFloat operator/(const SafeFloat& other) const { 
        SafeFloat res(value / other.value, name.empty() ? other.name : name);
        res.check(__func__);
        return res; 
    }

    // Implicit conversion to double for math functions
    operator double() const { return value; }
};
using RectanglePair = std::pair<std::shared_ptr<Rectangle>, std::shared_ptr<Rectangle>>;
using FractureRecord = std::tuple<RectanglePair, std::shared_ptr<Shape>>;
using FractureVector = std::vector<FractureRecord>;
using OptionalFractureVector = std::optional<FractureVector>;
class PolygonCollisions {
    public:
        double restitution = 0.4;
        // first tuple ID 1, ID2, rounded contact, second pair: J impulses
        std::map<ContactKey, std::pair<std::pair<double, double>, std::pair<double, double>>> impulse_cache;
        std::map<std::tuple<int, int, double, double>, std::pair<double, double>> jt_accumulation;
        std::map<std::tuple<int, int, double, double>, std::pair<double, double>> jn_accumulation;
        std::vector<Vec2> normal_computation(Shape& polygon);
        bool point_in_polygon(const Vec2& point, const Shape& polygon);
        std::pair<double, double> poly_min_max(Shape& polygon, const Vec2 normal);
        std::tuple<bool, Vec2, Vec2, std::vector<ContactKey>> sat_algorithm(Shape& polygon, Shape& polygon_other);
        std::tuple<double, double, double, double> impulse_resolution(std::shared_ptr<Shape>& polygon, std::shared_ptr<Shape>& polygon_other, ContactKey contact_key,Vec2 MTV, int iterations, int count, double penn_corr=1.0/0.1);
        void apply_impulse(std::shared_ptr<Shape>& polygon, std::shared_ptr<Shape>& polygon_other, Vec2 contact, std::pair<double, double> impulse, std::pair<double, double> friction);
        OptionalFractureVector polygon_collisions(std::vector<std::shared_ptr<Shape>>&polygons);
        std::tuple<bool, Vec2, Vec2, std::vector<Vec2>> substep(double dt, std::shared_ptr<Rectangle>& rect, std::shared_ptr<Shape>& poly);
        PolygonCollisions() = default;
        void positional_correction(std::shared_ptr<Shape>& polygon, std::shared_ptr<Shape>& polygon_other, Vec2 MTV, double fraction, double penn_corr = 1.0/11.5);

    private:
        bool segments_intersect(Vec2 p1, Vec2 p2, Vec2 q1, Vec2 q2);
        std::vector<ContactKey> polygon_contact_points_other(Shape& polygon, Shape& polygon_other);
        std::pair<int, int> line_intersects_segment(Vec2 begin_point, Vec2 end_point, Shape& polygon);
        std::pair<int, int> raycaster(Vec2 point, Shape& polygon, int ray_length = 200000);
        std::vector<Vec2> polygon_contact_points(Shape& polygon, Shape& polygon_other, Vec2 mtv_axis);
        std::tuple<double, double, Vec2> relative_velocity(Shape& polygon, Vec2 contact);
        std::optional<std::tuple<std::pair<Rectangle, Rectangle>, int>> fractured(std::shared_ptr<Shape> polygon, std::shared_ptr<Shape> polygon_other, double impulse_magnitide, Vec2 contact);
};




class PolygonManager {
    public:
        int incrementer = 0;
        std::vector<std::shared_ptr<Shape>> polygons;
        std::shared_ptr<PolygonCollisions> collision_class;
        std::shared_ptr<PendulumSolver> pendulum_class;
        void add_polygon(std::shared_ptr<Shape> poly);
        void remove_polygon(std::shared_ptr<Shape> poly);
        OptionalFractureVector step();

        PolygonManager(std::shared_ptr<PolygonCollisions> collision_class_, std::shared_ptr<PendulumSolver> pendulum_class_)
        : collision_class(collision_class_), pendulum_class(pendulum_class_) {}
        

};

inline void PolygonManager::add_polygon(std::shared_ptr<Shape> poly) {
    polygons.push_back(poly);
}

inline void PolygonManager::remove_polygon(std::shared_ptr<Shape> poly) {
    
    polygons.erase(std::remove(polygons.begin(), polygons.end(), poly), polygons.end());
}

