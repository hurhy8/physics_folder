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
const double PI = 3.14159265358979323846;
inline double dt = 1.0/220;
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

enum class BoundaryType { Left, Right };
struct Shape {
    virtual ~Shape() = default;
    inline static int next_id = 0;
    
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
          double length_=110, double omega_=3, double angle_=0, double mass_=25)
        : vx(vx_), vy(0), center_x(center_x_), local_center_x(0), local_center_y(0), center_y(center_y_),
          length(length_), omega(omega_), angle(angle_),
          gravity(9.8), mass(mass_),
          elasticity(0.5), collision_elasticity(0.5),
          allow_motion(true), collision_frame_handled(false),
          draw_animation(false), num_times_sleepy(0), id(next_id++),
          edge_coordinates()
    {}
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
            double center_y_ = 0, int length_ = 110,
            double omega_ = -3, double angle_ = 0, double mass_ = 25)
        : vertices(vertices_), Shape(vx_, center_x_, center_y_,
                                     length_, omega_, angle_, mass_)
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

        int width;
        bool aabb_overlap(const AABB& a, const AABB& b) override;
        void apply_rotation(Vec2 lever_arm_r, double Jx, double Jy) override;
        std::vector<Vec2> calculate_vertices_and_inertia() override;
        std::optional<std::pair<Rectangle, Rectangle>> fracture(double impulse, Vec2 contact_point, double fracture_threshold=16.5);
        Rectangle(int vertices_ = 4, double vx_ = 0, double center_x_ = 400, 
            double center_y_ = 0, int length_ = 110, int width_ = 70,
            double omega_ = 3, double angle_ = (0), double mass_ = 25)
            : Polygon(4, vx_, center_x_, center_y_,
                length_, omega_, angle_,
                mass_), width(width_){
                    
                    
                    local_vertices = calculate_vertices_and_inertia();
                    update_world_vertices();
                    update_aabb();


                    if (vertices < 3 || world_vertices.size() < 3 || local_vertices.size() < 3 || inertia <=0 || std::isnan(inertia) || mass <= 0
                    || std::isnan(angle)){
                        throw std::runtime_error("Something went very wrong with polygon creation.");
                    }
                };

};


class IsoscelesTriangle : public Polygon {
    public:
        int width;
        std::vector<Vec2> calculate_vertices_and_inertia() override;
        bool aabb_overlap(const AABB& a, const AABB& b) override;
        void apply_rotation(Vec2 lever_arm_r, double Jx, double Jy) override;
        IsoscelesTriangle(int vertices_ = 3, double vx_ = 0, double center_x_ = 400, 
            double center_y_ = 0, int equal_side = 110, int unique_side = 70,
            double omega_ = 3, double angle_ = (0), double mass_ = 25)
            : Polygon(vertices_, vx_, center_x_, center_y_,
                equal_side, omega_, angle_,
                mass_), width(unique_side){
                    angle = angle_;
                    center_x = center_x_;
                    center_y = center_y_;
                    local_vertices = calculate_vertices_and_inertia();
                    update_world_vertices();
                    update_aabb();

                    if (vertices < 3 || world_vertices.size() < 3 || local_vertices.size() < 3 || inertia <=0 || std::isnan(inertia) || mass <= 0
                    || std::isnan(angle)){
                        throw std::runtime_error("Something went very wrong with polygon creation.");
                    }
                };

};

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
        CompoundShape(std::vector<std::shared_ptr<Shape>> children, int total_vertices, double vx_ = 0, double omega_ = -3, double angle_ = 0)
        : Shape(vx_, 0, 0, 0, 0, angle_, 0),  // temporary mass 0, will update
            child_polygons(children),
            vertices(total_vertices)
            
        {
            for (auto& child : child_polygons){
                std::cout << "Local inertia child: " << child->inertia << ",  " << child->inertia << "\n";
                for (auto vert:child->local_vertices){
                    std::cout << vert.x << "  " << vert.y << ", ";
                }
                std::cout << "\n";
                
                mass += child->mass;
                local_center_x += child->mass * child->local_center_x;
                local_center_y += child->mass * child->local_center_y;
            }
            std::cout << "Mass: " << mass << "\n";
            local_center_x /= mass;
            local_center_y /= mass;
            std::cout << "Local centers: " << local_center_x << ",  " << local_center_y << "\n";
            
            for (auto& child : child_polygons){
                double dx = child->local_center_x - local_center_x;
                double dy = child->local_center_y - local_center_y;
        
                inertia += child->inertia + child->mass * (dx * dx + dy * dy);
            }
            
            
            this->vx = vx_;
            this->omega = omega_;
            this->angle = angle_;
            this->center_x = local_center_x;
            this->center_y = local_center_y;

            update_world_vertices();

        }
    protected:
        
            

           
        };
using RectanglePair = std::pair<std::shared_ptr<Rectangle>, std::shared_ptr<Rectangle>>;
using FractureRecord = std::tuple<RectanglePair, std::shared_ptr<Shape>>;
using FractureVector = std::vector<FractureRecord>;
using OptionalFractureVector = std::optional<FractureVector>;
class PolygonCollisions {
    public:
        double restitution = 0.4;
        // first tuple ID 1, ID2, rounded contact, second pair: J impulses
        std::map<std::tuple<int, int, double, double>, std::pair<double, double>> impulse_cache;
        std::vector<Vec2> normal_computation(Shape& polygon);
        bool point_in_polygon(const Vec2& point, const Shape& polygon);
        std::pair<double, double> poly_min_max(Shape& polygon, const Vec2 normal);
        std::tuple<bool, Vec2, Vec2, std::vector<Vec2>> sat_algorithm(Shape& polygon, Shape& polygon_other);
        std::tuple<double, double> impulse_resolution(std::shared_ptr<Shape>& polygon, std::shared_ptr<Shape>& polygon_other, Vec2 contact,Vec2 MTV, int iterations, int count, double penn_corr=1.0/0.1);
        void apply_impulse(std::shared_ptr<Shape>& polygon, std::shared_ptr<Shape>& polygon_other, Vec2 contact, std::pair<double, double> impulses);
        OptionalFractureVector polygon_collisions(std::vector<std::shared_ptr<Shape>>&polygons);
        std::tuple<bool, Vec2, Vec2, std::vector<Vec2>> substep(double dt, std::shared_ptr<Rectangle>& rect, std::shared_ptr<Shape>& poly);
        PolygonCollisions() = default;
        void positional_correction(std::shared_ptr<Shape>& polygon, std::shared_ptr<Shape>& polygon_other, Vec2 MTV, double fraction, double penn_corr = 1.0/11.5);

    private:
        bool segments_intersect(Vec2 p1, Vec2 p2, Vec2 q1, Vec2 q2);
        
        int line_intersects_segment(Vec2 begin_point, Vec2 end_point, Shape& polygon);
        int raycaster(Vec2 point, Shape& polygon, int ray_length = 200000);
        std::vector<Vec2> polygon_contact_points(Shape& polygon, Shape& polygon_other, Vec2 mtv_axis);
        std::tuple<double, double, Vec2> relative_velocity(Shape& polygon, Vec2 contact);
        std::optional<std::tuple<std::pair<Rectangle, Rectangle>, int>> fractured(std::shared_ptr<Shape> polygon, std::shared_ptr<Shape> polygon_other, double impulse_magnitide, Vec2 contact);

};




class PolygonManager {
    public:
        int incrementer = 0;
        std::vector<std::shared_ptr<Shape>> polygons;
        std::shared_ptr<PolygonCollisions> collision_class;
        void add_polygon(std::shared_ptr<Shape> poly);
        void remove_polygon(std::shared_ptr<Shape> poly);
        OptionalFractureVector step();

        PolygonManager(std::shared_ptr<PolygonCollisions> collision_class_)
        : collision_class(collision_class_) {}
        

};

inline void PolygonManager::add_polygon(std::shared_ptr<Shape> poly) {
    polygons.push_back(poly);
}

inline void PolygonManager::remove_polygon(std::shared_ptr<Shape> poly) {
    
    polygons.erase(std::remove(polygons.begin(), polygons.end(), poly), polygons.end());
}

